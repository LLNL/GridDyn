#pragma once
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef _MATRIX_DATA_SPARSE_H_
#define _MATRIX_DATA_SPARSE_H_

#include "matrixDataOrdering.h"
#include "utilities/matrixData.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>


/**
 * class for storing data from the Jacobian computation
 */

/** @brief class implementing an expandable sparse matrix geared for Jacobian entries*/
template <class Y = double>
class matrixDataSparse : public matrixData<Y>
{
  private:
    std::vector<matrixElement<Y>> data;  //!< the vector of tuples containing the data
    count_t sortCount = 0;  //!< count of the last sort operation
    decltype (data.cbegin ()) cptr;  //!< ptr to the beginning of the sequence;
  public:
    /** @brief constructor
    @param[in] startCount  the number of elements to allocate space for initially
    */
    explicit matrixDataSparse (index_t startCount = 50) { data.reserve (startCount); }
    /**
     * function to clear the data
     */
    void clear () override { data.clear (); }
    void assign (index_t row, index_t col, Y num) override
    {
        assert (row != ((index_t) (-1)));
        assert (col != ((index_t) (-1)));
        assert (row < matrixData<Y>::rowLim);
        assert (col < matrixData<Y>::colLim);
        assert (std::isfinite (num));
        assert (static_cast<int> (row) >= 0);
        assert (static_cast<int> (col) >= 0);

        // data.push_back (cLoc (X, Y, num));
        data.emplace_back (row, col, num);
    }

    /**
     * @brief reserve space for the count of the Jacobian elements
     * @param[in] reserveSize the amount of space to reserve
     */
    void reserve (count_t reserveSize) override { data.reserve (reserveSize); }
    count_t size () const override { return static_cast<count_t> (data.size ()); }

    count_t capacity () const override { return static_cast<count_t> (data.capacity ()); }

    /**
     * sort the index based first on column number then column number
     */
    void sortIndex (sparse_ordering ordering = sparse_ordering::row_ordered)
    {
        switch (ordering)
        {
        case sparse_ordering::column_ordered:
            std::sort (data.begin (), data.end (), compareCol<Y>);
            break;
        case sparse_ordering::row_ordered:
            std::sort (data.begin (), data.end (), compareRow<Y>);
            break;
        }
        sortCount = static_cast<count_t> (data.size ());
    }

    /**
     * @brief compact the index merging values with the same row and column number together
     */
    void compact () override
    {
        if (data.empty ())
        {
            return;
        }
        if (!isSorted ())
        {
            sortIndex (sparse_ordering::column_ordered);
        }
        auto currentDataLocation = data.begin ();
        auto testDataLocation = currentDataLocation + 1;
        auto dataEnd = data.end ();
        while (testDataLocation != dataEnd)
        {
            // Check if the next is equal to the previous in location
            // if they are add them if not shift the new one to the right location and move on
            if ((testDataLocation->col == currentDataLocation->col) &&
                (testDataLocation->row == currentDataLocation->row))
            {
                currentDataLocation->data += testDataLocation->data;
            }
            else
            {
                ++currentDataLocation;
                *currentDataLocation = *testDataLocation;
            }
            ++testDataLocation;
        }
        // reduce the size and indicate that we are still sorted.

        data.resize (++currentDataLocation - data.begin ());
        sortCount = static_cast<count_t> (data.size ());
    }
    matrixElement<Y> element (index_t N) const override { return data[N]; }


    auto begin () const { return data.cbegin (); }

    auto end () const { return data.end (); }

    void start () override { cptr = data.cbegin (); }

    matrixElement<Y> next () override
    {
        matrixElement<Y> tp = *cptr;
        ++cptr;
        return tp;
    }

    bool moreData () override { return (cptr != data.cend ()); }

    /**
     * @brief get the number nonzero of elements in each row
     * @return a vector of the column counts
     */
    std::vector<count_t> columnCount ()
    {
        if (!isSorted ())
        {
            sortIndex (sparse_ordering::column_ordered);
        }
        auto dataEnd = data.end ();
        std::vector<count_t> colCount ((*(dataEnd - 1)).row, 0);
        count_t cnt = 1;

        index_t testRow = data.front ().row;
        for (auto testData = data.begin (); testData != dataEnd; ++testData)
        {
            if (testRow != testData->row)
            {
                colCount[testRow] = cnt;
                cnt = 0;
                testRow = testData->row;
            }
            ++cnt;
        }
        colCount[testRow] = cnt;
        return colCount;
    }
    /** @brief check if the sparse array is sorted
    @return bool indicating sorted status
    */
    bool isSorted () const { return (sortCount == static_cast<count_t> (data.size ())); }

    Y at (index_t rowN, index_t colN) const override
    {
        if (isSorted ())
        {
            auto res =
              std::lower_bound (data.begin (), data.end (), matrixElement<Y>{rowN, colN, Y (0)}, compareCol<Y>);
            if (res == data.end ())
            {
                return Y (0);
            }
            if ((res->row == rowN) && (res->col == colN))
            {
                return res->data;
            }
            else
            {
                return Y (0);
            }
        }
        else
        {
            for (const auto &rv : data)
            {
                if ((rv.row == rowN) && (rv.col == colN))
                {
                    return rv.data;
                }
            }
            return Y (0);
        }
    }

    /** @brief scale a subset of the elements
    @param[in] factor the scaling factor
    @param[in] start the starting index
    @param[in] count the number of elements to scale
    */
    void scale (Y factor, index_t startIndex = 0, count_t count = 0x0FFFFFFFF)
    {
        if (startIndex >= data.size ())
        {
            return;
        }
        auto res = data.begin () + startIndex;
        auto term = data.end ();
        if (count < data.size ())
        {
            term = data.begin () + std::min (startIndex + count, static_cast<count_t> (data.size ()));
        }
        while (res != term)
        {
            res->data *= factor;
            ++res;
        }
    }
    /** @brief scale all the elements of a particular row
    @param[in] row the row to scale
    @param[in] factor the scaling factor
    */
    void scaleRow (index_t row, Y factor)
    {
        for (auto &res : data)
        {
            if (res.row == row)
            {
                res.data *= factor;
            }
        }
    }
    /** @brief scale all the elements of a particular column
    @param[in] col the row to scale
    @param[in] factor the scaling factor
    */
    void scaleCol (index_t col, Y factor)
    {
        for (auto &res : data)
        {
            if (res.col == col)
            {
                res.data *= factor;
            }
        }
    }
    /** @brief translate all the elements in a particular row to some other row
    @param[in] origRow the row to change
    @param[in] newRow the row to change origRow into
    */
    void translateRow (index_t origRow, index_t newRow)
    {
        for (auto &res : data)
        {
            if (res.row == origRow)
            {
                res.row = newRow;
            }
        }
    }
    /** @brief translate all the elements in a particular column to some other column
    @param[in] origCol the column to change
    @param[in] newCol the column to change origCol into
    */
    void translateCol (index_t origCol, index_t newCol)
    {
        for (auto &res : data)
        {
            if (res.col == origCol)
            {
                res.col = newCol;
            }
        }
    }
    using matrixData<Y>::copyTranslateRow;

    /** @brief translate all the elements in a particular row in a2 and translate row origRow to newRow
    @param[in] a2  the matrixDataSparse object to copy from
    @param[in] origRow the column to change
    @param[in] newRow the column to change origRow into
    */
    void copyTranslateRow (const matrixDataSparse<Y> &a2, index_t origRow, index_t newRow)
    {
        for (const auto &res : a2.data)
        {
            if (res.row == origRow)
            {
                data.emplace_back (newRow, res.col, res.data);
            }
        }
    }
    /** @brief translate all the elements in a particular column in a2 and translate column origCol to newCol
    @param[in] a2  the matrixDataSparse object to copy from
    @param[in] origCol the column to change
    @param[in] newCol the column to change origCol into
    */
    void copyTranslateCol (const matrixDataSparse<Y> &a2, index_t origCol, index_t newCol)
    {
        for (const auto &res : a2.data)
        {
            if (res.col == origCol)
            {
                data.emplace_back (res.row, newCol, res.data);
            }
        }
    }

    /** @brief copy all data from a2 to the array and translate origCol into a set of new indices and scale factors
      this function is useful when the input to some object is a summation of other states,  this function allows
    the translation into a large sparse data object
    @param[in] a2  the matrixDataSparse object to copy from
    @param[in] origCol the column to change
    @param[in] newIndices a vector of indices to change
    @param[in] mult the scaler multiplier for each fo the new indices
    */
    void copyReplicate (const matrixDataSparse<Y> &a2,
                        index_t origCol,
                        std::vector<index_t> newIndices,
                        std::vector<Y> mult)
    {
        for (const auto &res : a2.data)
        {
            if (res.col == origCol)
            {
                for (index_t nn = 0; nn < newIndices.size (); ++nn)
                {
                    data.emplace_back (res.row, newIndices[nn], res.data * mult[nn]);
                }
            }
            else
            {
                data.push_back (res);
            }
        }
    }
    /** @brief remove invalid rows or those given by the testrow
    @param[in] rowTest,  the row index to remove*/
    void filter (index_t rowTest = (index_t) (-1))
    {
        auto rem = std::remove_if (data.begin (), data.end (),
                                   [rowTest, clim = matrixData<Y>::colLim,
                                    rlim = matrixData<Y>::rowLim](const matrixElement<Y> &el) {
                                       return ((el.row == rowTest) || (el.row >= rlim) || (el.col >= clim));
                                   });
        data.erase (rem, data.end ());
    }

    void cascade (matrixDataSparse<Y> &a2, index_t cascadeIndex)
    {
        auto term = data.size ();
        size_t nn = 0;
        Y keyval = Y (0);
        while (nn != term)
        {
            if (data[nn].col == cascadeIndex)
            {
                size_t mm = 0;
                keyval = data[nn].data;
                for (size_t kk = 0; kk < a2.data.size (); kk++)
                {
                    if (a2.data[kk].row == cascadeIndex)
                    {
                        if (mm == 0)
                        {
                            data[nn].col = a2.data[kk].col;
                            data[nn].data = a2.data[kk].data * keyval;
                            ++mm;
                        }
                        else
                        {
                            // data.push_back (cLoc (std::get<adRow> (data[nn]), std::get<adCol> (a2->data[kk]),
                            // keyval * std::get<adVal> (a2->data[kk])));
                            data.emplace_back (data[nn].row, a2.data[kk].col, keyval * a2.data[kk].data);
                            ++mm;
                        }
                    }
                }
            }
            ++nn;
        }
    }
    using matrixData<Y>::merge;
    void merge (matrixDataSparse<Y> &a2) { data.insert (data.end (), a2.data.begin (), a2.data.end ()); }

    void transpose ()
    {
        for (auto &celement : data)
        {
            std::swap (celement.col, celement.row);
            //	int t1 = std::get<adCol>(data[kk]);
            //	std::get<adCol>(data[kk]) = std::get<adRow>(data[kk]);
            //	std::get<adRow>(data[kk]) = t1;
        }
    }
    void diagMultiply (std::vector<Y> diag)
    {
        for (size_t kk = 0; kk < data.size (); kk++)
        {
            data[kk].data *= diag[data[kk].col];
        }
    }

    std::vector<Y> vectorMult (std::vector<Y> V)
    {
        sortIndex (sparse_ordering::row_ordered);
        auto maxRow = data.back ().row;
        std::vector<Y> out (maxRow, 0);
        auto res = data.begin ();
        auto term = data.end ();
        for (index_t nn = 0; nn <= maxRow; ++nn)
        {
            while ((res != term) && (*res.row == nn))
            {
                out[nn] += *res.data * V[*res.col];
                ++res;
            }
        }
        return out;
    }

  protected:
};

template <class Y>
std::vector<index_t> findMissing (matrixDataSparse<Y> &ad)
{
    std::vector<index_t> missing;
    ad.compact ();
    ad.sortIndex (sparse_ordering::row_ordered);
    index_t pp = 0;
    for (index_t kk = 0; kk < ad.rowLimit (); ++kk)
    {
        bool good = false;
        if (pp > ad.size ())
        {
            break;
        }
        auto element = ad.element (pp);
        while (element.row <= kk)
        {
            if ((element.row == kk) && (std::isnormal (element.data)))
            {
                good = true;
                ++pp;
                break;
            }

            ++pp;
            if (pp >= ad.size ())
            {
                break;
            }
            element = ad.element (pp);
        }
        if (!good)
        {
            missing.push_back (kk);
        }
    }
    return missing;
}

template <class Y>
std::vector<std::vector<index_t>> findRank (matrixDataSparse<Y> &ad)
{
    std::vector<index_t> vr, vt;
    std::vector<Y> vq, vtq;
    std::vector<std::vector<index_t>> mrows;
    ad.sortIndex (sparse_ordering::column_ordered);
    ad.compact ();
    ad.sortIndex (sparse_ordering::row_ordered);
    Y factor = 0;
    index_t pp = 0;
    index_t qq = 0;
    auto mp = ad.size ();
    bool good = false;
    for (index_t kk = 0; kk < ad.rowLimit () - 1; ++kk)
    {
        vr.clear ();
        vq.clear ();
        auto nextElement = ad.element (pp);
        while (nextElement.row == kk)
        {
            vr.push_back (nextElement.col);
            vq.push_back (nextElement.data);
            ++pp;
            nextElement = ad.element (pp);
        }
        qq = pp;
        for (index_t nn = kk + 1; nn < ad.rowLimit (); ++nn)
        {
            vt.clear ();
            vtq.clear ();
            good = false;
            if (nextElement.col != vr[0])
            {
                good = true;
            }
            while (nextElement.row == nn)
            {

                if (!good)
                {
                    vt.push_back (nextElement.col);
                    vtq.push_back (nextElement.data);
                }
                ++qq;

                if (qq >= mp)
                {
                    break;
                }
                nextElement = ad.element (qq);
            }
            if (!good)
            {
                continue;
            }
            if (vr.size () != vtq.size ())
            {
                continue;
            }

            for (size_t jj = 0; jj < vr.size (); ++jj)
            {
                if (vt[jj] != vr[jj])
                {
                    good = true;
                    break;
                }
                else if (jj == 0)
                {
                    factor = vtq[jj] / vq[jj];
                }
                else if (std::abs (vtq[jj] / vq[jj] - factor) > 0.000001)
                {
                    good = true;
                    break;
                }
            }
            if (good)
            {
                continue;
            }
            else
            {
                mrows.push_back ({kk, nn});
            }
        }
    }
    return mrows;
}

#endif
