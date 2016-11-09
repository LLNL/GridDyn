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

#define FILE_LOAD_SUCCESS 0
#define FILE_NOT_FOUND (-1)
#define FILE_INCOMPLETE (-2);
#define FILE_LOAD_FAILURE (-3);

typedef std::uint32_t fsize_t;

//TODO::PT add iterators
//TODO:: PT make some of the data private
/** @brief class to hold a single time series*/
class timeSeries
{
public:
  std::string description;  //!< time series description
  std::vector<double> time;  //!< storage for time data
  std::vector<double> data;  //!< storage for value data
  std::string field;    //!< the name of the field the data comes from
  fsize_t count = 0;  //!< the current index location
  /** default constructor*/
  timeSeries ();
  /** constructor to build the time series from a file*/
  explicit timeSeries (const std::string &fname);
  /** add a data point to the time series
  @param[in] t the time
  @param[in] point the value
  @return true if the data was successfully added
  */
  bool addData (double t, double point);
  /** add a vector of data points to the time series
  @param[in] tm the time
  @param[in] val the value
  @return true if the data was successfully added
  */
  bool addData (const std::vector<double> &tm, const std::vector<double> &val);
  /** resize the time series
  @param[in] newSize  the new size of the time series*/
  void resize (fsize_t newSize);
  /** reserve space in the time series
  @param[in] newCapacity  the required capacity of the time series*/
  void reserve (fsize_t newCapacity);
  /** @brief clear the data from the time series*/
  void clear ();

  /** @brief load a file into the time series
  automatically detect the file type based on extension
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  @return the number of points that were loaded
  */
  int loadFile(const std::string &filename, unsigned int column = 0);
  /** @brief load a binary file into the time series
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  @return the number of points that were loaded
  */
  int loadBinaryFile (const std::string &filename,unsigned int column = 0);
  /** @brief load a text file into the time series
  @param[in] filename  the file to load
  @param[in] column  the column of data in the file to load into the time series
  @return the number of points that were loaded
  */
  int loadTextFile (const std::string &filename, unsigned int column = 0);
  /** @brief write a binary file from the data in the time series
  @param[in] filename  the file to write
  @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
  @return the number of points that were written
  */
  int writeBinaryFile (const std::string &filename,bool append = false);
  /** @brief write a csv file from the data in the time series
  @param[in] filename  the file to write
  @param[in] precision  the number of digits that should be included for non-integer data in the file
  @param[in] append  flag indicating that if the file exists it should be appended rather than overwritten
  @return the number of points that were written
  */
  int writeTextFile (const std::string &filename,int precision = 8, bool append = false);
};

/** @brief class holding multiple time series associated with a single time*/
class timeSeries2
{
public:
  std::string description;  //!< a description of the time series
  std::vector<double> time;	//!< a vector of times associated with the data
  std::vector<std::vector<double> > data;  //!< a 2d vector of data to store the time series information
  fsize_t cols = 1;		//!< the number of columns of data
  fsize_t count = 0;	//!< the current data location
  fsize_t capacity = 0;	//!< the total capacity of the time series
  std::vector<std::string> fields;	//!< container for all the strings associated with the different columns
public:
  timeSeries2 ();
  explicit timeSeries2 (fsize_t numCols);
  timeSeries2 (fsize_t numCols, fsize_t numRows);
  explicit timeSeries2 (const std::string &fname);
  /** add a data point to the time series
  @param[in] t the time
  @param[in] point the value
	@param[in] column the column to add the data to column 0 for default
  @return true if the data was successfully added
  */
  bool addData (double t, double point, unsigned int column = 0);
  /** add a vector of data points to the time series
  @param[in] t the time
  @param[in] ndata the vector of values
  @param[in] column the column to start adding the data to column 0 for default
  @return true if the data was successfully added
  */
  bool addData (double t, std::vector<double> &ndata, unsigned int column = 0);
  /** add a time series of data points
  @param[in] ndata the vector of values
  @param[in] column the column the data represents
  @return true if the data was successfully added
  */
  bool addData (std::vector<double> &ndata, unsigned int column);

  /** add a vector of data points and the time vector
  @param[in] ntime the vector of times
  @param[in] ndata the vector of values
  @param[in] column the column to start adding the data to column 0 for default
  @return true if the data was successfully added
  */
  bool addData (std::vector<double> &ntime,std::vector<double> &ndata, unsigned int column = 0);
  void resize (fsize_t newSize);
  void reserve (fsize_t newCapacity);
  void setCols (fsize_t numCols);
  void clear ();

  /** @brief load a file into the time series
  automatically detect the file type based on extension
  @param[in] filename  the file to load
  @return the number of points that were loaded
  */
  int loadFile(const std::string &filename);

  int loadBinaryFile (const std::string &filename);
  int loadTextFile (const std::string &filename);
  int writeBinaryFile (const std::string &filename, bool append = false);
  int writeTextFile (const std::string &filename, int precision = 8, bool append = false);
private:
};

//comparison functions
double compare (timeSeries *ts1, timeSeries *ts2);
double compare (timeSeries *ts1, timeSeries *ts2, int cnt);
double compare (timeSeries2 *ts1, timeSeries *ts2, int stream);
double compare (timeSeries2 *ts1, timeSeries *ts2, int stream, int cnt);
double compare (timeSeries2 *ts1, timeSeries2 *ts2);
double compare (timeSeries2 *ts1, timeSeries2 *ts2, int stream);
double compare (timeSeries2 *ts1, timeSeries2 *ts2, int stream1, int stream2);
double compare (timeSeries2 *ts1, timeSeries2 *ts2, int stream1, int stream2, int cnt);


#endif