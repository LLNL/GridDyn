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

#ifndef TIMESERIES_H_
#define TIMESERIES_H_
#pragma once

#include <cstdint>
#include <exception>

#include "stringConversion.h"
#include "vectorOps.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

class fileNotFoundError : public std::exception
{
  public:
    virtual const char *what () const noexcept override { return "file not found"; }
};

class openFileError : public std::exception
{
  public:
    virtual const char *what () const noexcept override { return "unable to open file"; }
};

class fileLoadFailure : public std::exception
{
  public:
    virtual const char *what () const noexcept override { return "file load failure"; }
};

class fileIncomplete : public std::exception
{
  public:
    virtual const char *what () const noexcept override { return "file incomplete"; }
};

class invalidDataSize : public std::exception
{
  public:
    virtual const char *what () const noexcept override { return "input data sizes are not valid"; }
};

using fsize_t = std::uint32_t;

// TODO::PT add iterators
/** @brief class to hold a single time series*/
template <typename dataType = double, typename timeType = double>
class timeSeries
{
  public:
    std::string description;  //!< time series description
    std::string field;  //!< the name of the field the data comes from
  private:
    std::vector<timeType> m_time;  //!< storage for time data
    std::vector<dataType> m_data;  //!< storage for value data

    fsize_t count = 0;  //!< the current index location
  public:
    /** default constructor*/
    timeSeries () = default;
    /** constructor to build the time series from a file*/
    explicit timeSeries (const std::string &fileName) { loadFile (fileName); }
    /** add a data point to the time series
    @param[in] t the time
    @param[in] point the value
    @return true if the data was successfully added
    */
    void addData (timeType t, dataType point)
    {
        m_time.push_back (t);
        m_data.push_back (point);
        count = count + 1;
    }
    /** add a vector of data points to the time series
    @param[in] tm the time
    @param[in] val the value
    @return true if the data was successfully added
    */
    void addData (const std::vector<timeType> &tm, const std::vector<dataType> &val)
    {
        if (tm.size () == val.size ())
        {
            m_time.resize (count + tm.size ());
            m_data.resize (count + tm.size ());
            std::copy (tm.begin (), tm.end (), m_time.begin () + count);
            std::copy (val.begin (), val.end (), m_data.begin () + count);
            count += static_cast<fsize_t> (tm.size ());
        }
        else if (val.size () == 1)
        {
            m_time.resize (count + tm.size ());
            std::copy (tm.begin (), tm.end (), m_time.begin () + count);
            m_data.resize (count + tm.size (), val[0]);
            count += static_cast<fsize_t> (tm.size ());
        }
        else
        {
            throw (invalidDataSize ());
        }
    }
    /** resize the time series
    @param[in] newSize  the new size of the time series*/
    void resize (fsize_t newSize)
    {
        m_time.resize (newSize, timeType (0.0));
        m_data.resize (newSize, dataType (0.0));
        count = newSize;
    }
    /** reserve space in the time series
    @param[in] newCapacity  the required capacity of the time series*/
    void reserve (fsize_t newCapacity)
    {
        m_time.reserve (newCapacity);
        m_data.reserve (newCapacity);
    }
    /** @brief get the size()*/
    fsize_t size () const { return count; }
    /** @brief return true if there is no data*/
    bool empty () const { return (count == 0); }
    /** @brief clear the data from the time series*/
    void clear ()
    {
        m_time.clear ();
        m_data.clear ();
        count = 0;
    }

    /** @brief get a vector for the time*/
    const std::vector<timeType> &time () const { return m_time; }
    /** @brief get an element of the time*/
    timeType time (fsize_t index) const { return m_time[index]; }
    timeType lastTime () const { return m_time[count - 1]; }
    /** @brief get a vector for the data*/
    const std::vector<dataType> &data () const { return m_data; }
    /** @brief get an element of the time*/
    dataType data (fsize_t index) const { return m_data[index]; }
    dataType lastData () const { return m_data[count - 1]; }
    /** @brief load a file into the time series
    automatically detect the file type based on extension
    @param[in] fileName  the file to load
    @param[in] column  the column of data in the file to load into the time series
    */
    void loadFile (const std::string &fileName, unsigned int column = 0)
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
                loadTextFile (fileName, column);
                return;
            }
        }
        loadBinaryFile (fileName, column);
    }
    /** @brief load a binary file into the time series
    @param[in] fileName  the file to load
    @param[in] column  the column of data in the file to load into the time series
    */
    void loadBinaryFile (const std::string &fileName, unsigned int column = 0)
    {
        std::ifstream fio (fileName.c_str (), std::ios::in | std::ios::binary);
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        fsize_t nc;
        // double *buf;

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
            if (dcount > 256)
            {
                dbuff.resize (dcount);
            }
            fio.read (dbuff.data (), dcount);
            description = std::string (dbuff.data (), dcount);
        }
        fio.read (reinterpret_cast<char *> (&nc), sizeof (fsize_t));
        fio.read (reinterpret_cast<char *> (&rcount), sizeof (fsize_t));

        resize (nc);  // update the size
        fsize_t cols = rcount - 1;

        // now read the field names
        unsigned char len;
        for (fsize_t cc = 0; cc < cols; cc++)
        {
            fio.read (reinterpret_cast<char *> (&len), 1);
            if (cc == column)
            {
                if (len > 0)
                {
                    if (cc == column)
                    {
                        fio.read (dbuff.data (), len);
                        field = std::string (dbuff.data (), len);
                    }
                }
            }
            else
            {
                fio.seekg (len - 256, std::ifstream::ios_base::cur);
            }
        }

        // allocate a buffer to store the read data
        std::vector<double> buf (nc);
        fio.read (reinterpret_cast<char *> (buf.data ()), nc * sizeof (double));

        if (rcount < 2)
        {
            fio.close ();
            throw (fileIncomplete ());
        }
        for (fsize_t cc = 0; cc < cols; cc++)
        {
            if (cc == column)
            {
                fio.read (reinterpret_cast<char *> (m_data.data ()), nc * sizeof (dataType));
            }
            else
            {
                fio.seekg (nc * sizeof (dataType), std::ifstream::ios_base::cur);
            }
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
                if (cc == column)
                {
                    fio.read (reinterpret_cast<char *> (m_data.data () + ocount), nc * sizeof (dataType));
                }
                else
                {
                    fio.seekg (nc * sizeof (dataType), std::ifstream::ios_base::cur);
                }
            }
            ocount += nc;
            // read the next character
            fio.read (reinterpret_cast<char *> (&nc), sizeof (fsize_t));
        }

        m_time = vectorConvert<timeType> (std::move (buf));
        fio.close ();
    }
    /** @brief load a text file into the time series
    @param[in] fileName  the file to load
    @param[in] column  the column of data in the file to load into the time series
    */
    void loadTextFile (const std::string &fileName, unsigned int column = 0)
    {
        std::ifstream fio (fileName.c_str (), std::ios::in);
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        std::string line, line2;
        std::getline (fio, line);
        if (line[0] == '#')
        {
            std::getline (fio, line2);
        }
        else
        {
            line2 = line;
        }
        auto cols = stringOps::splitlineBracket (line2, ",");
        if (cols.size () <= column + 1)
        {
            fio.close ();
            throw (fileLoadFailure ());
        }
        if (line.size () > 2)
        {
            description = line.substr (1);
        }
        clear ();
        field = stringOps::removeQuotes (cols[column + 1]);

        while (std::getline (fio, line))
        {
            auto svc = str2vector (line, -1e48, ",");
            if (svc.size () > column + 1)
            {
                addData (svc[0], svc[column + 1]);
            }
        }
        fio.close ();
    }
    /** @brief write a binary file from the data in the time series
    @param[in] fileName  the file to write
    @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
    */
    void writeBinaryFile (const std::string &fileName, bool append = false)
    {
        int temp;
        std::ofstream fio (fileName.c_str (),
                           std::ios::out | std::ios::binary | ((append) ? (std::ios::app) : (std::ios::trunc)));
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        // write a bit ordering integer
        if (!append)
        {
            temp = 1;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            temp = static_cast<fsize_t> (description.length ());
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            if (temp > 0)
            {
                fio.write (description.c_str (), temp);
            }
            temp = count;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            temp = 2;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));

            // write the field  name
            unsigned char ccnt;
            if (field.size () > 255)
            {
                ccnt = 255;
            }
            else
            {
                ccnt = static_cast<unsigned char> (field.size ());
            }
            fio.write (reinterpret_cast<const char *> (&ccnt), 1);
            if (ccnt > 0)
            {
                fio.write (field.c_str (), ccnt);
            }
        }
        else
        {
            temp = count;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
            temp = 2;
            fio.write (reinterpret_cast<const char *> (&temp), sizeof (int));
        }
        if (count > 0)
        {
            for (auto &t : m_time)
            {
                auto tr = static_cast<double> (t);
                fio.write (reinterpret_cast<const char *> (&tr), sizeof (double));
            }
            fio.write (reinterpret_cast<const char *> (m_data.data ()), count * sizeof (dataType));
        }

        fio.close ();
    }
    /** @brief write a csv file from the data in the time series
    @param[in] fileName  the file to write
    @param[in] precision  the number of digits that should be included for non-integer data in the file
    @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
    */
    void writeTextFile (const std::string &fileName, int precision = 8, bool append = false)
    {
        std::ofstream fio (fileName.c_str (), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
        if (!fio)
        {
            throw (fileNotFoundError ());
        }
        if (!append)
        {
            auto ndes = stringOps::characterReplace (description, '\n', "\n#");
            fio << "#" << ndes << "\n\"time\", \"" << field << "\"\n";
        }
        if (precision < 1)
        {
            precision = 8;
        }
        for (size_t rr = 0; rr < count; rr++)
        {
            fio << std::setprecision (5) << m_time[rr] << ',' << std::setprecision (precision) << m_data[rr]
                << '\n';
        }
        fio.close ();
    }
};

#endif