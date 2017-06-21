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

#ifndef TIMESERIESMULTI_H_
#define TIMESERIESMULTI_H_
#pragma once

// don't need the time series object but if you include this I want timeSeries to be included as well
// along with all the other objects
#include "timeSeries.hpp"
/** a multidimensional time series object */
template <typename dataType = double, typename timeType = double>
class timeSeriesMulti
{
  public:
    std::string description;  //!< a description of the time series
  private:
    std::vector<timeType> m_time;  //!< a vector of times associated with the m_data
    std::vector<std::vector<dataType>> m_data;  //!< a 2d vector of m_data to store the time series information
    std::vector<std::string> fields;  //!< container for all the strings associated with the different columns
    fsize_t cols = 1;  //!< the number of columns of m_data
    fsize_t count = 0;  //!< the current m_data location
    fsize_t capacity = 0;  //!< the total capacity of the time series

  public:
    timeSeriesMulti ()
    {
        m_data.resize (1);
        fields.resize (1);
    }
    explicit timeSeriesMulti (fsize_t numCols)
    {
        cols = 1;
        m_data.resize (1);
        setCols (numCols);
    }
    timeSeriesMulti (fsize_t numCols, fsize_t numRows)
    {
        cols = 1;
        count = numRows;
        setCols (numCols);
        resize (numRows);
    }
    explicit timeSeriesMulti (const std::string &fileName) { loadFile (fileName); }
    const std::vector<std::string> &getFields () const { return fields; }
    const std::string getField (fsize_t index) const { return fields[index]; }
    void setField (fsize_t index, std::string newField)
    {
        ensureSizeAtLeast (fields, index + 1);
        fields[index] = std::move (newField);
    }

    void setFields (const std::vector<std::string> &newFields)
    {
        for (size_t ii = 0; (ii < cols) && (ii < newFields.size ()); ++ii)
        {
            fields[ii] = newFields[ii];
        }
    }

    void setField (std::vector<std::string> &&newFields)
    {
        if (newFields.size () == cols)
        {
            fields = std::move (newFields);
        }
    }
    /** add a m_data point to the time series
    @param[in] t the time
    @param[in] point the value
    @param[in] column the column to add the m_data to column 0 for default
    @throw out_of_range if the column is not valid
    */
    void addData (timeType t, dataType point, unsigned int column = 0)
    {
        if (column >= cols)
        {
            throw (std::out_of_range ("specified column > dataset size"));
        }
        if ((count == 0) || (t - m_time[count - 1] > timeType (0.0)))
        {
            m_time.push_back (t);
            ++count;
        }

        m_data[column].push_back (point);
    }
    /** add a vector of m_data points to the time series
    @param[in] t the time
    @param[in] ndata the vector of values
    @param[in] column the column to start adding the m_data to column 0 for default
    @throw invalidDataSize if the data is not sized correctly
    */
    void addData (timeType t, const std::vector<dataType> &ndata, unsigned int column = 0)
    {
        if (ndata.size () + column > cols)
        {
            throw (invalidDataSize ());
        }
        if (count > 0)
        {
            if (t - m_time[count - 1] > timeType (0.0))
            {
                m_time.push_back (t);
                ++count;
            }
        }
        else
        {
            m_time.push_back (t);
            ++count;
        }
        auto dv = m_data.begin () + column;
        for (auto newDataPoint : ndata)
        {
            dv->push_back (newDataPoint);
            ++dv;
        }
    }
    /** add a time series of m_data points
    @param[in] ndata the vector of values must be the same size as the rest of the data
    @param[in] column the column the m_data represents
    @throw invalidDataSize if the data is not sized correctly
    */
    void addData (const std::vector<dataType> &ndata, unsigned int column)
    {
        if (ndata.size () != count)
        {
            throw (invalidDataSize ());
        }
        if (column >= cols)
        {
            setCols (column);
        }
        m_data[column] = ndata;
    }

    /** add a time series of m_data points
    @param[in] ndata the vector of values must be the same size as the rest of the data
    @param[in] column the column the m_data represents
    @throw invalidDataSize if the data is not sized correctly
    */
    void addData (std::vector<dataType> &&ndata, unsigned int column)
    {
        if (ndata.size () != count)
        {
            throw (invalidDataSize ());
        }
        if (column >= cols)
        {
            setCols (column);
        }
        m_data[column] = std::move (ndata);
    }
    /** add a vector of m_data points and the time vector
    @param[in] ntime the vector of times
    @param[in] ndata the vector of values
    @param[in] column the column to start adding the m_data to column 0 for default
    @throw invalidDataSize if the sizes of the time and data are not equal
    */
    void addData (const std::vector<timeType> &ntime, const std::vector<dataType> &ndata, unsigned int column = 0)
    {
        if (ntime.size () != ndata.size ())
        {
            throw (invalidDataSize ());
        }
        if (column >= cols)
        {
            setCols (column);
        }
        m_time = ntime;

        m_data[column] = ndata;
        count = static_cast<fsize_t> (ntime.size ());
    }
    /** add a vector of m_data points and the time vector
    @param[in] ntime the vector of times
    @param[in] ndata the vector of values
    @param[in] column the column to start adding the m_data to column 0 for default
    @throw invalidDataSize if the sizes of the time and data are not equal
    */
    void addData (std::vector<timeType> &&ntime, std::vector<dataType> &&ndata, unsigned int column = 0)
    {
        if (ntime.size () != ndata.size ())
        {
            throw (invalidDataSize ());
        }
        if (column >= cols)
        {
            setCols (column);
        }
        m_time = std::move (ntime);

        m_data[column] = std::move (ndata);
        count = static_cast<fsize_t> (ntime.size ());
    }

    void updateData (fsize_t column, fsize_t row, dataType newValue)
    {
        if ((column < cols) && (row < count))
        {
            m_data[column][row] = newValue;
        }
        else
        {
            throw (std::out_of_range ("invalid element specification"));
        }
    }
    fsize_t size () const noexcept { return count; }
    fsize_t columns () const noexcept { return cols; }

    /** @brief return true if there is no data*/
    bool empty () const { return (count == 0); }
    /** @brief get a vector for the time*/
    const std::vector<timeType> &time () const { return m_time; }
    /** @brief get an element of the time*/
    timeType time (fsize_t index) const { return m_time[index]; }
    timeType lastTime () const { return m_time[count - 1]; }
    /** @brief get a vector for the m_data*/
    const std::vector<dataType> &data (fsize_t index) const { return m_data[index]; }
    /** @brief get an element of the time*/
    dataType data (fsize_t col_index, fsize_t row_index) const { return m_data[col_index][row_index]; }
    dataType lastData (fsize_t index) const { return m_data[index][count - 1]; }
    std::vector<dataType> lastData () const
    {
        std::vector<dataType> b (cols);
        for (fsize_t ii = 0; ii < cols; ++ii)
        {
            b[ii] = m_data[ii][count - 1];
        }
        return b;
    }

    void resize (fsize_t newSize)
    {
        m_time.resize (newSize, timeType (0.0));
        for (auto &dk : m_data)
        {
            dk.resize (newSize, dataType (0.0));
        }
        count = newSize;
    }

    void reserve (fsize_t newCapacity)
    {
        m_time.reserve (newCapacity);
        capacity = newCapacity;
        for (auto &dk : m_data)
        {
            dk.reserve (newCapacity);
        }
    }

    void setCols (fsize_t newCols)
    {
        fields.resize (newCols);
        m_data.resize (newCols);
        for (size_t kk = cols; kk < newCols; ++kk)
        {
            m_data[kk].reserve (std::max (capacity, count));
            m_data[kk].resize (count);
        }
        cols = newCols;
    }
    void clear ()
    {
        m_time.clear ();
        for (auto &dk : m_data)
        {
            dk.clear ();
        }
        count = 0;
    }

    const std::vector<dataType> &operator[] (size_t kk) const { return m_data[kk]; }
    void scaleData (fsize_t col, dataType factor)
    {
        if (col < cols)
        {
            std::transform (m_data[col].begin (), m_data[col].end (), m_data[col].begin (),
                            [factor](dataType val) { return val * factor; });
        }
        else
        {
            throw (std::out_of_range ("invalid column specification"));
        }
    }

    void scaleData (dataType factor)
    {
        for (auto &dc : m_data)
        {
            std::transform (dc.begin (), dc.end (), dc.begin (), [factor](dataType val) { return val * factor; });
        }
    }
    /** @brief load a file into the time series
    automatically detect the file type based on extension
    @param[in] fileName  the file to load
    @return the number of points that were loaded
    */
    void loadFile (const std::string &fileName)
    {
        if (fileName.empty ())
        {
            throw (fileNotFoundError ());
        }
        if (fileName.size () > 5)
        {
            std::string ext = convertToLowerCase (fileName.substr (fileName.length () - 3));
            if ((ext == "csv") || (ext == "txt"))
            {
                loadTextFile (fileName);
                return;
            }
        }
        loadBinaryFile (fileName);
    }

    void loadBinaryFile (const std::string &fileName)
    {
        std::ifstream fio (fileName.c_str (), std::ios::in | std::ios::binary);
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        fsize_t nc;
        fsize_t dcount;

        fsize_t rcount;
        int align;
        fio.read (reinterpret_cast<char *> (&align), sizeof (fsize_t));
        if (align != 1)
        {
            // I don't know what to do here yet
        }
        fio.read (reinterpret_cast<char *> (&dcount), sizeof (fsize_t));
        std::vector<char> dbuff (256);
        if (dcount > 0)
        {
            fio.read (dbuff.data (), dcount);
            description = std::string (dbuff.data (), dcount);
        }
        fio.read (reinterpret_cast<char *> (&nc), sizeof (fsize_t));
        fio.read (reinterpret_cast<char *> (&rcount), sizeof (fsize_t));

        setCols (rcount - 1);  // update the number of columns the file contains the time, then the m_data columns
        resize (nc);  // update the size

        // now read the field names
        unsigned char len;
        for (fsize_t cc = 0; cc < cols; cc++)
        {
            fio.read (reinterpret_cast<char *> (&len), 1);
            if (len > 0)
            {
                fio.read (dbuff.data (), len);
                fields[cc] = std::string (dbuff.data (), len);
            }
        }

        // allocate a buffer to store the read m_data
        std::vector<double> buf (nc);
        fio.read (reinterpret_cast<char *> (buf.data ()), nc * sizeof (double));

        if (rcount < 2)
        {
            fio.close ();
            throw (fileIncomplete ());
        }
        for (fsize_t cc = 0; cc < cols; cc++)
        {
            fio.read (reinterpret_cast<char *> (m_data[cc].data ()), nc * sizeof (dataType));
            // m_data[cc] = std::vector<double>(buf, buf + nc);
        }
        fio.read (reinterpret_cast<char *> (&nc), sizeof (fsize_t));
        fsize_t ocount = count;
        while (!fio.eof ())
        {
            fio.read (reinterpret_cast<char *> (&rcount), sizeof (fsize_t));
            if (rcount != cols + 1)
            {
                break;
            }
            resize (nc + ocount);
            buf.resize (nc + ocount);
            fio.read (reinterpret_cast<char *> (buf.data () + ocount), nc * sizeof (double));
            for (fsize_t cc = 0; cc < cols; cc++)
            {
                fio.read (reinterpret_cast<char *> (m_data[cc].data () + ocount), nc * sizeof (dataType));
                // m_data[cc] = std::vector<double>(buf, buf + nc);
            }
            ocount += nc;
            // read the next character
            fio.read (reinterpret_cast<char *> (&nc), sizeof (fsize_t));
        }

        m_time = vectorConvert<timeType> (std::move (buf));

        fio.close ();
    }
    void loadTextFile (const std::string &fileName)
    {
        std::ifstream fio (fileName.c_str (), std::ios::in);
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        std::string line;
        std::getline (fio, line);
        while (line[0] == '#')
        {
            if (line.size () > 2)
            {
                if (description.empty ())
                {
                    description = line.substr (1);
                }
                else
                {
                    description += '\n' + line.substr (1);
                }
            }
            std::getline (fio, line);
        }

        auto colnames = stringOps::splitline (line, ',');
        setCols (static_cast<fsize_t> (colnames.size ()) - 1);
        for (fsize_t kk = 1; kk < static_cast<fsize_t> (colnames.size ()); ++kk)
        {
            fields[kk - 1] = stringOps::removeQuotes (colnames[kk]);
        }
        clear ();
        timeType timeV;
        while (std::getline (fio, line))
        {
            auto svc = str2vector (line, -1e48, ",");
            timeV = svc[0];
            for (fsize_t kk = 1; kk < static_cast<fsize_t> (svc.size ()); ++kk)
            {
                addData (timeV, svc[kk], kk - 1);
            }
        }
        fio.close ();
    }
    void writeBinaryFile (const std::string &fileName, bool append = false)
    {
        std::ofstream fio (fileName.c_str (),
                           std::ios::out | std::ios::binary | ((append) ? (std::ios::app) : (std::ios::trunc)));
        if (!fio)
        {
            throw (openFileError ());
        }
        if (!append)
        {
            fsize_t temp = 1;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            temp = static_cast<fsize_t> (description.length ());
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            if (temp > 0)
            {
                fio.write (description.c_str (), temp);
            }

            // now write the size of the m_data
            temp = count;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (fsize_t));
            temp = cols + 1;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (fsize_t));
            // now write the column names
            unsigned char ccnt = 0;
            for (fsize_t cc = 0; cc < cols; cc++)
            {
                if (fields[cc].size () > 255)
                {
                    ccnt = 255;
                }
                else
                {
                    ccnt = static_cast<unsigned char> (fields[cc].size ());
                }
                fio.write (reinterpret_cast<const char *> (&ccnt), 1);
                if (ccnt > 0)
                {
                    fio.write (fields[cc].c_str (), ccnt);
                }
            }
        }
        else
        {
            fsize_t temp = count;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (fsize_t));
            temp = cols + 1;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (fsize_t));
        }
        // now write the data
        if (count > 0)
        {
            for (auto &t : m_time)
            {
                auto tr = static_cast<double> (t);
                fio.write (reinterpret_cast<const char *> (&tr), sizeof (double));
            }
            for (fsize_t cc = 0; cc < cols; cc++)
            {
                fio.write (reinterpret_cast<const char *> (m_data[cc].data ()), count * sizeof (dataType));
            }
        }

        fio.close ();
    }
    void writeTextFile (const std::string &fileName, int precision = 8, bool append = false)
    {
        std::ofstream fio (fileName.c_str (), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
        if (!fio)
        {
            throw (openFileError ());
        }
        std::string ndes = stringOps::characterReplace (description, '\n', "\n#");

        if (!append)
        {
            fio << "# " << ndes << "\n\"time\"";
            for (auto fieldName : fields)
            {
                fio << R"(, ")" << fieldName << '"';
            }
            fio << '\n';
        }
        if (precision < 1)
        {
            precision = 8;
        }
        for (size_t rr = 0; rr < count; rr++)
        {
            fio << std::setprecision (5) << m_time[rr];
            for (size_t kk = 0; kk < cols; ++kk)
            {
                fio << ',' << std::setprecision (precision) << m_data[kk][rr];
            }
            fio << '\n';
        }
        fio.close ();
    }

  private:
};

// comparison functions
template <typename dataType, typename timeType>
dataType compare (const timeSeries<dataType, timeType> &ts1, const timeSeries<dataType, timeType> &ts2)
{
    return compareVec (ts1.data (), ts2.data ());
}

template <typename dataType, typename timeType>
dataType compare (const timeSeries<dataType, timeType> &ts1, const timeSeries<dataType, timeType> &ts2, int cnt)
{
    return compareVec (ts1.data (), ts2.data (), cnt);
}

template <typename dataType, typename timeType>
dataType
compare (const timeSeriesMulti<dataType, timeType> &ts1, const timeSeries<dataType, timeType> &ts2, int stream)
{
    return compareVec (ts1[stream], ts2.data ());
}

template <typename dataType, typename timeType>
dataType compare (const timeSeriesMulti<dataType, timeType> &ts1,
                  const timeSeries<dataType, timeType> &ts2,
                  int stream,
                  int cnt)
{
    return compareVec (ts1[stream], ts2.data (), cnt);
}

template <typename dataType, typename timeType>
dataType compare (const timeSeriesMulti<dataType, timeType> &ts1, const timeSeriesMulti<dataType, timeType> &ts2)
{
    dataType diff (0);
    auto cnt = std::min (ts1.columns (), ts2.columns ());

    for (decltype (cnt) kk = 0; kk < cnt; ++kk)
    {
        diff += compareVec (ts1[kk], ts2[kk]);
    }

    return diff;
}

template <typename dataType, typename timeType>
dataType compare (const timeSeriesMulti<dataType, timeType> &ts1,
                  const timeSeriesMulti<dataType, timeType> &ts2,
                  int stream)
{
    return compareVec (ts1[stream], ts2[stream]);
}

template <typename dataType, typename timeType>
dataType compare (const timeSeriesMulti<dataType, timeType> &ts1,
                  const timeSeriesMulti<dataType, timeType> &ts2,
                  int stream1,
                  int stream2)
{
    return compareVec (ts1[stream1], ts2[stream2]);
}

template <typename dataType, typename timeType>
dataType compare (const timeSeriesMulti<dataType, timeType> &ts1,
                  const timeSeriesMulti<dataType, timeType> &ts2,
                  int stream1,
                  int stream2,
                  int cnt)
{
    return compareVec (ts1[stream1], ts2[stream2], cnt);
}

#endif