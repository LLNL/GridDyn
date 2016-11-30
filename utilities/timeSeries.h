/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
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

#include <string>
#include <vector>
#include <cstdint>
#include <exception>

#include "stringConversion.h"
#include "vectorOps.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

class fileNotFoundError :public std::exception
{
public:
	virtual const char *what() const noexcept override
	{
		return "file not found";
	}
};

class fileLoadFailure :public std::exception
{
public:
	virtual const char *what() const noexcept override
	{
		return "file load failure";
	}
};

class fileIncomplete :public std::exception
{
public:
	virtual const char *what() const noexcept override
	{
		return "file incomplete";
	}
};

typedef std::uint32_t fsize_t;


//TODO::PT add iterators
//TODO:: PT make some of the data private
/** @brief class to hold a single time series*/
template <typename dataType=double, typename timeType=double>
class timeSeries
{
public:
  std::string description;  //!< time series description
  std::vector<timeType> time;  //!< storage for time data
  std::vector<dataType> data;  //!< storage for value data
  std::string field;    //!< the name of the field the data comes from
  fsize_t count = 0;  //!< the current index location
  /** default constructor*/
  timeSeries()
  {
  }
  /** constructor to build the time series from a file*/
  explicit timeSeries(const std::string &fname)
  {
	  loadFile(fname);
  }
  /** add a data point to the time series
  @param[in] t the time
  @param[in] point the value
  @return true if the data was successfully added
  */
  bool addData (timeType t, dataType point)
  {
	  time.push_back(t);
	  data.push_back(point);
	  count = count + 1;
	  return true;

  }
  /** add a vector of data points to the time series
  @param[in] tm the time
  @param[in] val the value
  @return true if the data was successfully added
  */
  bool addData (const std::vector<timeType> &tm, const std::vector<dataType> &val)
  {
	  if (tm.size() == val.size())
	  {
		  time.resize(count + tm.size());
		  data.resize(count + tm.size());
		  std::copy(tm.begin(), tm.end(), time.begin() + count);
		  std::copy(val.begin(), val.end(), data.begin() + count);
		  count += static_cast<fsize_t> (tm.size());
	  }
	  else if (val.size() == 1)
	  {
		  time.resize(count + tm.size());
		  std::copy(tm.begin(), tm.end(), time.begin() + count);
		  data.resize(count + tm.size(), val[0]);
		  count += static_cast<fsize_t> (tm.size());
	  }
	  else
	  {
		  return false;
	  }
	  return true;
  }
  /** resize the time series
  @param[in] newSize  the new size of the time series*/
  void resize (fsize_t newSize)
  {
	  time.resize(newSize, timeType(0.0));
	  data.resize(newSize, dataType(0.0));
	  count = newSize;
  }
  /** reserve space in the time series
  @param[in] newCapacity  the required capacity of the time series*/
  void reserve (fsize_t newCapacity)
  {
	  time.reserve(newCapacity);
	  data.reserve(newCapacity);
  }

  /** @brief clear the data from the time series*/
  void clear ()
  {
	  time.clear();
	  data.clear();
  }

  /** @brief load a file into the time series
  automatically detect the file type based on extension
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  */
  void loadFile(const std::string &filename, unsigned int column = 0)
  {
	  std::string ext = filename.substr(filename.length() - 3);
	  if ((ext == "csv") || (ext == "txt"))
	  {
		  loadTextFile(filename, column);
	  }
	  else
	  {
		  loadBinaryFile(filename, column);
	  }
  }
  /** @brief load a binary file into the time series
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  */
  void loadBinaryFile (const std::string &filename,unsigned int column = 0)
  {
	  std::ifstream fio(filename.c_str(), std::ios::in | std::ios::binary);
	  if (!fio)
	  {
		  throw(fileNotFoundError());
	  }
	  fsize_t nc;
	  //double *buf;
	
	  fsize_t dcount;
	  fsize_t rcount;
	  int align;
	  fio.read((char *)(&align), sizeof(fsize_t));
	  if (align != 1)
	  {
		  //I don't know what to do here yet
	  }
	  fio.read((char *)(&dcount), sizeof(fsize_t));
	  std::vector<char> dbuff(256);
	  if (dcount > 0)
	  {
		  if (dcount > 256)
		  {
			  dbuff.resize(dcount);
		  }
		  fio.read(dbuff.data(), dcount);
		  description = std::string(dbuff.data(), dcount);
	  }
	  fio.read((char *)(&nc), sizeof(fsize_t));
	  fio.read((char *)(&rcount), sizeof(fsize_t));

	  resize(nc);            //update the size
	  fsize_t cols = rcount - 1;



	  //now read the field names
	  unsigned char len;
	  for (fsize_t cc = 0; cc < cols; cc++)
	  {
		  fio.read((char *)(&len), 1);
		  if (cc == column)
		  {
			  if ((len > 0) && (len <= 256))
			  {

				  if (cc == column)
				  {
					  fio.read(dbuff.data(), len);
					  field = std::string(dbuff.data(), len);
				  }
			  }
			  else if (len > 256)
			  {
				  fio.read(dbuff.data(), 256);
				  field = std::string(dbuff.data(), 256);
				  fio.seekg(len - 256, std::ifstream::ios_base::cur);
			  }
		  }
		  else
		  {
			  fio.seekg(len - 256, std::ifstream::ios_base::cur);
		  }
	  }
	  
	  //allocate a buffer to store the read data
	  std::vector<double> buf(nc);
	  fio.read((char *)(buf.data()), nc * sizeof(double));
	  
	  if (rcount < 2)
	  {
		  fio.close();
		  throw(fileIncomplete());
	  }
	  for (fsize_t cc = 0; cc < cols; cc++)
	  {
		  if (cc == column)
		  {
			  fio.read((char *)(data.data()), nc * sizeof(dataType));
		  }
		  else
		  {
			  fio.seekg(nc * sizeof(dataType), std::ifstream::ios_base::cur);
		  }
	  }
	  fio.read((char *)(&nc), sizeof(fsize_t));
	  fsize_t ocount = count;
	  while (!fio.eof())
	  {
		  fio.read((char *)(&rcount), sizeof(fsize_t));
		  if (rcount != cols + 1)
		  {
			  break;
		  }
		  resize(nc + ocount);
		  buf.resize(nc + ocount);
		  fio.read((char *)(buf.data() + ocount), nc * sizeof(double));
		  for (fsize_t cc = 0; cc < cols; cc++)
		  {
			  if (cc == column)
			  {
				  fio.read((char *)(data.data() + ocount), nc * sizeof(dataType));
			  }
			  else
			  {
				  fio.seekg(nc * sizeof(dataType), std::ifstream::ios_base::cur);
			  }

		  }
		  ocount += nc;
		  //read the next character
		  fio.read((char *)(&nc), sizeof(fsize_t));
	  }


	  time = vectorConvert<timeType>(std::move(buf));
	  fio.close();


  }
  /** @brief load a text file into the time series
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  */
  void loadTextFile (const std::string &filename, unsigned int column = 0)
  {
	  std::ifstream fio(filename.c_str(), std::ios::in);
	  if (!fio)
	  {
		  throw(fileNotFoundError());
	  }
	  std::string line, line2;
	  std::getline(fio, line);
	  if (line[0] == '#')
	  {
		  std::getline(fio, line2);
	  }
	  else
	  {
		  line2 = line;
	  }
	  auto cols = splitlineBracket(line2, ",");
	  if (cols.size() <= column + 1)
	  {
		  fio.close();
		  throw(fileLoadFailure());
	  }
	  if (line.size() > 2)
	  {
		  description = line.substr(1);
	  }
	  clear();
	  field = removeChars(cols[column + 1], "\"");

	  while (std::getline(fio, line))
	  {
		  auto svc = str2vector(line, -1e48, ",");
		  if (svc.size() > column + 1)
		  {
			  addData(svc[0], svc[column + 1]);
		  }
	  }
	  fio.close();

  }
  /** @brief write a binary file from the data in the time series
  @param[in] filename  the file to write
  @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
  */
  void writeBinaryFile (const std::string &filename,bool append = false)
  {
	  int temp;
	  std::ofstream fio(filename.c_str(), std::ios::out | std::ios::binary | ((append) ? (std::ios::app) : (std::ios::trunc)));
	  if (!fio)
	  {
		  throw(fileNotFoundError());
	  }
	  //write a bit ordering integer
	  if (!append)
	  {
		  temp = 1;
		  fio.write((const char *)&temp, sizeof(int));
		  temp = static_cast<fsize_t> (description.length());
		  fio.write((const char *)&temp, sizeof(int));
		  if (temp > 0)
		  {
			  fio.write(description.c_str(), temp);
		  }
		  temp = count;
		  fio.write((const char *)&temp, sizeof(int));
		  temp = 2;
		  fio.write((const char *)&temp, sizeof(int));

		  //write the field  name
		  unsigned char ccnt;
		  if (field.size() > 255)
		  {
			  ccnt = 255;
		  }
		  else
		  {
			  ccnt = static_cast<unsigned char> (field.size());
		  }
		  fio.write((const char *)&ccnt, 1);
		  if (ccnt > 0)
		  {
			  fio.write(field.c_str(), ccnt);
		  }
	  }
	  else
	  {
		  temp = count;
		  fio.write((const char *)&temp, sizeof(int));
		  temp = 2;
		  fio.write((const char *)&temp, sizeof(int));
	  }
	  if (count > 0)
	  {
		  for (auto &t : time)
		  {
			  double tr = static_cast<double>(t);
			  fio.write((const char *)(&tr), sizeof(double));
		  }
		  fio.write((const char *)(data.data()), count * sizeof(dataType));
	  }

	  fio.close();


  }
  /** @brief write a csv file from the data in the time series
  @param[in] filename  the file to write
  @param[in] precision  the number of digits that should be included for non-integer data in the file
  @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
  */
  void writeTextFile (const std::string &filename,int precision = 8, bool append = false)
  {
	  std::ofstream fio(filename.c_str(), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
	  if (!fio)
	  {
		  throw(fileNotFoundError());
	  }
	  if (!append)
	  {
		  std::string ndes = characterReplace(description, '\n', "\n#");
		  fio << "#" << ndes << "\n\"time\", \"" << field << "\"\n";
	  }
	  if (precision < 1)
	  {
		  precision = 8;
	  }
	  for (size_t rr = 0; rr < count; rr++)
	  {
		  fio << std::setprecision(5) << time[rr] << ',' << std::setprecision(precision) << data[rr] << '\n';
	  }
	  fio.close();

  }
};




#endif