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



#include "timeSeries.h"
#include "stringConversion.h"
#include "vectorOps.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

timeSeries::timeSeries ()
{
}

timeSeries::timeSeries (const std::string &fname)
{
  
}

bool timeSeries::addData (double t, double point)


bool timeSeries::addData (const std::vector<double> &tm, const std::vector<double> &val)


void timeSeries::resize (fsize_t newSize)


void timeSeries::reserve (fsize_t newSize)


void timeSeries::clear ()


void timeSeries::loadFile(const std::string &filename, unsigned int column)



void timeSeries::loadBinaryFile (const std::string &filename,unsigned int column)



void timeSeries::loadTextFile (const std::string &filename,unsigned int column)


void timeSeries::writeBinaryFile (const std::string &filename,bool append)


void timeSeries::writeTextFile (const std::string &filename,int precision, bool append)




/*
std::vector<std::vector<double>> data2;

                int cols;
                stringVec fields;
                void addData(double t, double point, int column);
                void addData(double t, std::vector<double> ndata);
                void setSize(int newSize);
                void setCapacity(int newCapacity);
                void setCols(int numCols);
                int loadBinaryFile(const std::string filename);
                int loadTextFile(const std::string filename);
                int writeBinaryFile(const std::string filename);
                int writeTextFile(const std::string filename);
                double compare(timeSeries *ts2, int stream=1, int cnt=-1);
                double compare(timeSeriesMulti *ts2, int stream=1,int cnt=-1){return compare(ts2,stream,stream,cnt);};
                double compare(timeSeriesMulti *ts2, int stream1=1,int stream2=1,int cnt=-1);
                */

timeSeriesMulti::timeSeriesMulti ()

timeSeriesMulti::timeSeriesMulti (fsize_t numCols)

timeSeriesMulti::timeSeriesMulti (fsize_t numCols, fsize_t numRows)


timeSeriesMulti::timeSeriesMulti (const std::string &fname)


bool timeSeriesMulti::addData (double t, double point,unsigned int column)


bool timeSeriesMulti::addData (double t, std::vector<double> &ndata,unsigned int column)


bool timeSeriesMulti::addData (std::vector<double> &ndata, unsigned int column)

bool timeSeriesMulti::addData (std::vector<double> &ntime, std::vector<double> &ndata, unsigned int column)


void timeSeriesMulti::setCols (fsize_t newCols)


void timeSeriesMulti::resize (fsize_t newSize)


void timeSeriesMulti::reserve (fsize_t newSize)



void timeSeriesMulti::clear ()



void timeSeriesMulti::loadFile(const std::string &filename)


void timeSeriesMulti::loadBinaryFile (const std::string &filename)


void timeSeriesMulti::loadTextFile (const std::string &filename)


void timeSeriesMulti::writeBinaryFile (const std::string &filename,bool append)


void timeSeriesMulti::writeTextFile (const std::string &filename,int precision, bool append)


//large series of comparison functions
double compare (timeSeries *ts1, timeSeries *ts2)
{

  return compareVec (ts1->data, ts2->data);

}

double compare (timeSeries *ts1, timeSeries *ts2, int cnt)




double compare (timeSeriesMulti *ts1, timeSeries *ts2, int stream)


double compare (timeSeriesMulti *ts1, timeSeries *ts2, int stream, int cnt)


double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2)


double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream)


double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream1, int stream2)


double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream1, int stream2, int cnt)

