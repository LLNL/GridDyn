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
#include "vectorOps.hpp"
#include "stringOps.h"
#include <iostream>
#include <fstream>
#include <iomanip>

timeSeries::timeSeries ()
{
}

timeSeries::timeSeries (const std::string &fname)
{
  loadFile (fname);
}

bool timeSeries::addData (double t, double point)
{
  time.push_back (t);
  data.push_back (point);
  count = count + 1;
  return true;

}

bool timeSeries::addData (const std::vector<double> &tm, const std::vector<double> &val)
{
  if (tm.size () == val.size ())
    {
      time.resize (count + tm.size ());
      data.resize (count + tm.size ());
      std::copy (tm.begin (), tm.end (), time.begin () + count);
      std::copy (val.begin (), val.end (), data.begin () + count);
      count += static_cast<fsize_t> (tm.size ());
    }
  else if (val.size () == 1)
    {
      time.resize (count + tm.size ());
      std::copy (tm.begin (), tm.end (), time.begin () + count);
      data.resize (count + tm.size (),val[0]);
      count += static_cast<fsize_t> (tm.size ());
    }
  else
    {
      return false;
    }
  return true;
}

void timeSeries::resize (fsize_t newSize)
{
  time.resize (newSize,0);
  data.resize (newSize,0);
  count = newSize;
}

void timeSeries::reserve (fsize_t newSize)
{
  time.reserve (newSize);
  data.reserve (newSize);
}

void timeSeries::clear ()
{
  time.clear ();
  data.clear ();
}

void timeSeries::loadFile(const std::string &filename, unsigned int column)
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


void timeSeries::loadBinaryFile (const std::string &filename,unsigned int column)
{
  std::ifstream fio (filename.c_str (), std::ios::in | std::ios::binary);
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  fsize_t nc;
  //double *buf;
  char *dbuff;
  fsize_t dcount;
  fsize_t cc;
  fsize_t rcount;
  int align;
  fio.read ((char *)(&align), sizeof(fsize_t));
  if (align != 1)
    {
      //I don't know what to do here yet
    }
  fio.read ((char *)(&dcount), sizeof(fsize_t));
  if (dcount > 0)
    {
      dbuff = new char[dcount];
      fio.read (dbuff, dcount);
      description = std::string (dbuff, dcount);
      delete[] dbuff;
    }
  fio.read ((char *)(&nc), sizeof(fsize_t));
  fio.read ((char *)(&rcount), sizeof(fsize_t));

  resize (nc);            //update the size
  fsize_t cols = rcount - 1;



  //now read the field names
  unsigned char len;
  dbuff = new char[256];
  for (cc = 0; cc < cols; cc++)
    {
      fio.read ((char *)(&len), 1);
      if (len > 0)
        {
          fio.read (dbuff, len);
          if (cc == column)
            {
              field = std::string (dbuff, len);
            }
        }
    }
  delete[] dbuff;
  //allocate a buffer to store the read data
  //buf = new double[nc];
  fio.read ((char *)(time.data ()), nc * sizeof(double));

  if (rcount < 2)
    {
      fio.close ();
	  throw(fileIncomplete());
    }
  for (cc = 0; cc < cols; cc++)
    {
      if (cc == column)
        {
          fio.read ((char *)(data.data ()), nc * sizeof(double));
        }
      else
        {
          fio.seekg (nc * sizeof(double), std::ifstream::ios_base::cur);
        }
      //data[cc] = std::vector<double>(buf, buf + nc);
    }
  fio.read ((char *)(&nc), sizeof(fsize_t));
  fsize_t ocount = count;
  while (!fio.eof ())
    {
      fio.read ((char *)(&rcount), sizeof(fsize_t));
      if (rcount != cols + 1)
        {
          break;
        }
      resize (nc + ocount);
      fio.read ((char *)(time.data () + ocount), nc * sizeof(double));
      for (cc = 0; cc < cols; cc++)
        {
          if (cc == column)
            {
              fio.read ((char *)(data.data () + ocount), nc * sizeof(double));
            }
          else
            {
              fio.seekg (nc * sizeof(double), std::ifstream::ios_base::cur);
            }

        }
      //read the next character
      fio.read ((char *)(&nc), sizeof(fsize_t));
    }



  fio.close ();
  //delete [] buf;


}


void timeSeries::loadTextFile (const std::string &filename,unsigned int column)
{
  std::ifstream fio (filename.c_str (), std::ios::in);
  if (!fio)
    {
	  throw(fileNotFoundError());
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
  auto cols = splitlineBracket (line2, ",");
  if (cols.size () <= column + 1)
    {
      fio.close ();
	  throw(fileLoadFailure());
    }
  if (line.size () > 2)
    {
      description = line.substr (1);
    }
  clear ();
  field = removeChars (cols[column + 1],"\"");

  while (std::getline (fio, line))
    {
      auto svc = str2vector (line, -1e48,",");
      if (svc.size () > column + 1)
        {
          addData (svc[0],svc[column + 1]);
        }
    }
  fio.close ();

}

void timeSeries::writeBinaryFile (const std::string &filename,bool append)
{
  int temp;
  std::ofstream fio (filename.c_str (), std::ios::out | std::ios::binary | ((append) ? (std::ios::app): (std::ios::trunc)));
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  //write a bit ordering integer
  if (!append)
    {
      temp = 1;
      fio.write ((const char *)&temp, sizeof(int));
      temp = static_cast<fsize_t> (description.length ());
      fio.write ((const char *)&temp, sizeof(int));
      if (temp > 0)
        {
          fio.write (description.c_str (), temp);
        }
      temp = count;
      fio.write ((const char *)&temp, sizeof(int));
      temp = 2;
      fio.write ((const char *)&temp, sizeof(int));

      //write the field  name
      unsigned char ccnt;
      if (field.size () > 255)
        {
          ccnt = 255;
        }
      else
        {
          ccnt = static_cast<unsigned char> (field.size ());
        }
      fio.write ((const char *)&ccnt, 1);
      if (ccnt > 0)
        {
          fio.write (field.c_str (), ccnt);
        }
    }
  else
    {
      temp = count;
      fio.write ((const char *)&temp, sizeof(int));
      temp = 2;
      fio.write ((const char *)&temp, sizeof(int));
    }
  if (count > 0)
    {
      fio.write ((const char *)&(time.front ()),count * sizeof(double));
      fio.write ((const char *)&(data.front ()),count * sizeof(double));
    }

  fio.close ();


}

void timeSeries::writeTextFile (const std::string &filename,int precision, bool append)
{
  std::ofstream fio (filename.c_str (), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  if (!append)
    {
      std::string ndes = characterReplace (description, '\n', "\n#");
      fio << "#" << ndes << "\n\"time\", \"" << field << "\"\n" ;
    }
  if (precision < 1)
    {
      precision = 8;
    }
  for (size_t rr = 0; rr < count; rr++)
    {
      fio << std::setprecision (5) << time[rr] << ',' << std::setprecision (precision) << data[rr] << '\n';
    }
  fio.close ();

}



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
{
  data.resize (1);
  fields.resize (1);
}
timeSeriesMulti::timeSeriesMulti (fsize_t numCols)
{
  cols = 1;
  count = 0;
  data.resize (1);
  setCols (numCols);
}
timeSeriesMulti::timeSeriesMulti (fsize_t numCols, fsize_t numRows)
{
  cols = 1;
  count = numRows;
  setCols (numCols);
  resize (numRows);
}

timeSeriesMulti::timeSeriesMulti (const std::string &fname)
{
  loadFile (fname);
}

bool timeSeriesMulti::addData (double t, double point,unsigned int column)
{
  if (column >= cols)
    {
      return false;
    }
  if (count > 0)
    {
      if (t - time[count - 1] > 0.0001)
        {
          time.push_back (t);
          ++count;
        }
    }
  else
    {
      time.push_back (t);
      ++count;
    }
  data[column].push_back (point);

  return true;

}

bool timeSeriesMulti::addData (double t, std::vector<double> &ndata,unsigned int column)
{
  if (ndata.size () + column > cols)
    {
      return false;
    }
  if (count > 0)
    {
      if (t - time[count - 1] > 0.0001)
        {
          time.push_back (t);
          ++count;
        }
    }
  else
    {
      time.push_back (t);
      ++count;
    }
  auto dv = data.begin () + column;
  for (auto newDataPoint : ndata)
    {
      dv->push_back (newDataPoint);
      ++dv;
    }

  return true;
}

bool timeSeriesMulti::addData (std::vector<double> &ndata, unsigned int column)
{
  if (ndata.size () != count)
    {
      return false;
    }
  if (column >= cols)
    {
      setCols (column);
    }
  data[column] = ndata;
  return true;
}
bool timeSeriesMulti::addData (std::vector<double> &ntime, std::vector<double> &ndata, unsigned int column)
{
  if (ntime.size () != ndata.size ())
    {
      return false;
    }
  if (column >= cols)
    {
      setCols (column);
    }
  time = ntime;

  data[column] = ndata;
  count = static_cast<fsize_t> (ntime.size ());
  return true;

}

void timeSeriesMulti::setCols (fsize_t newCols)
{

  fields.resize (newCols);
  data.resize (newCols);
  for (size_t kk = cols; kk < newCols; ++kk)
    {
      data[kk].reserve (std::max (capacity,count));
      data[kk].resize (count);
    }
  cols = newCols;
}

void timeSeriesMulti::resize (fsize_t newSize)
{
  time.resize (newSize,0);
  for (auto &dk : data)
    {
      dk.resize (newSize,0);
    }
  count = newSize;
}

void timeSeriesMulti::reserve (fsize_t newSize)
{
  time.reserve (newSize);
  capacity = newSize;
  for (auto &dk : data)
    {
      dk.reserve (newSize);
    }
}


void timeSeriesMulti::clear ()
{
  time.clear ();
  for (auto &dk : data)
    {
      dk.clear ();
    }
  count = 0;
}


void timeSeriesMulti::loadFile(const std::string &filename)
{
	std::string ext = filename.substr(filename.length() - 3);
	if ((ext == "csv") || (ext == "txt"))
	{
		loadTextFile(filename);
	}
	else
	{
		loadBinaryFile(filename);
	}
}

void timeSeriesMulti::loadBinaryFile (const std::string &filename)
{
  std::ifstream fio (filename.c_str (), std::ios::in | std::ios::binary);
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  fsize_t nc;
  //double *buf;
  char *dbuff;
  fsize_t dcount;
  fsize_t cc;
  fsize_t rcount;
  int align;
  fio.read ((char *)(&align), sizeof(fsize_t));
  if (align != 1)
    {
      //I don't know what to do here yet
    }
  fio.read ((char *)(&dcount),sizeof(fsize_t));
  if (dcount > 0)
    {
      dbuff = new char[dcount];
      fio.read (dbuff,dcount);
      description = std::string (dbuff,dcount);
      delete [] dbuff;
    }
  fio.read ((char *)(&nc),sizeof(fsize_t));
  fio.read ((char *)(&rcount),sizeof(fsize_t));

  setCols (rcount - 1);       //update the number of columns the file contains the time, then the data columns
  resize (nc);       // update the size



  //now read the field names
  unsigned char len;
  dbuff = new char[256];
  for (cc = 0; cc < cols; cc++)
    {
      fio.read ((char *)(&len), 1);
      if (len > 0)
        {
          fio.read (dbuff, len);
          fields[cc] = std::string (dbuff, len);
        }
    }
  delete [] dbuff;
  //allocate a buffer to store the read data
  //buf = new double[nc];
  fio.read ((char *)(time.data ()),nc * sizeof(double));

  //time = std::vector<double>(buf, buf + nc);

  if (rcount < 2)
    {
      fio.close ();
      //delete [] buf;
	  throw(fileIncomplete());
    }
  for (cc = 0; cc < cols; cc++)
    {
      fio.read ((char *)(data[cc].data ()),nc * sizeof(double));
      //data[cc] = std::vector<double>(buf, buf + nc);
    }
  fio.read ((char *)(&nc), sizeof(fsize_t));
  fsize_t ocount = count;
  while (!fio.eof ())
    {
      fio.read ((char *)(&rcount),sizeof(fsize_t));
      if (rcount != cols + 1)
        {
          break;
        }
      resize (nc + ocount);
      fio.read ((char *)(time.data () + ocount), nc * sizeof(double));
      for (cc = 0; cc < cols; cc++)
        {
          fio.read ((char *)(data[cc].data () + ocount), nc * sizeof(double));
          //data[cc] = std::vector<double>(buf, buf + nc);
        }
      //read the next character
      fio.read ((char *)(&nc), sizeof(fsize_t));
    }



  fio.close ();
  //delete [] buf;


}

void timeSeriesMulti::loadTextFile (const std::string &filename)
{
  std::ifstream fio (filename.c_str (), std::ios::in);
  unsigned int kk;
  if (!fio)
    {
	  throw(fileNotFoundError());
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

  auto colnames = splitline (line, ',');
  setCols (static_cast<fsize_t> (colnames.size ()) - 1);
  for (kk = 1; kk < colnames.size (); ++kk)
    {
      fields[kk - 1] = removeChar (colnames[kk], '"');
    }
  clear ();
 double timeV;
  while (std::getline (fio, line))
    {
      auto svc = str2vector (line, -1e48, ",");
      timeV = svc[0];
      for (kk = 1; kk < svc.size (); ++kk)
        {
          addData (timeV, svc[kk], kk - 1);
        }
    }
  fio.close ();

}

void timeSeriesMulti::writeBinaryFile (const std::string &filename,bool append)
{
  fsize_t temp;
  fsize_t cc;
  std::ofstream fio (filename.c_str (), std::ios::out | std::ios::binary | ((append) ? (std::ios::app) : (std::ios::trunc)));
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  if (!append)
    {
      temp = 1;
      fio.write ((const char *)&temp, sizeof(int));
      temp = static_cast<fsize_t> (description.length ());
      fio.write ((const char *)&temp, sizeof(int));
      if (temp > 0)
        {
          fio.write (description.c_str (), temp);
        }

      //now write the size of the data
      temp = count;
      fio.write ((const char *)&temp, sizeof(fsize_t));
      temp = cols + 1;
      fio.write ((const char *)&temp, sizeof(fsize_t));
      //now write the column names
      unsigned char ccnt = 0;
      for (cc = 0; cc < cols; cc++)
        {
          if (fields[cc].size () > 255)
            {
              ccnt = 255;
            }
          else
            {
              ccnt = static_cast<unsigned char> (fields[cc].size ());
            }
          fio.write ((const char *)&ccnt, 1);
          if (ccnt > 0)
            {
              fio.write (fields[cc].c_str (), ccnt);
            }
        }
    }
  else
    {
      temp = count;
      fio.write ((const char *)&temp, sizeof(fsize_t));
      temp = cols + 1;
      fio.write ((const char *)&temp, sizeof(fsize_t));
    }
  //now write the data
  if (count > 0)
    {
      fio.write ((const char *)(time.data ()),count * sizeof(double));
      for (cc = 0; cc < cols; cc++)
        {
          fio.write ((const char *)(data[cc].data ()),count * sizeof(double));
        }
    }

  fio.close ();

}

void timeSeriesMulti::writeTextFile (const std::string &filename,int precision, bool append)
{

  std::ofstream fio (filename.c_str (), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
  if (!fio)
    {
	  throw(fileNotFoundError());
    }
  std::string ndes = characterReplace (description,'\n',"\n#");

  if (!append)
    {
      fio << "# " << ndes << "\n\"time\"";
      for (auto fname : fields)
        {
          fio << ", \"" << fname << "\"";
        }
      fio << '\n';
    }
  if (precision < 1)
    {
      precision = 8;
    }
  for (size_t rr = 0; rr < count; rr++)
    {
      fio << std::setprecision (5) << time[rr];
      for (size_t kk = 0; kk < cols; ++kk)
        {
          fio << ',' << std::setprecision (precision) << data[kk][rr];
        }
      fio << '\n';
    }
  fio.close ();
 
}

//large series of comparison functions
double compare (timeSeries *ts1, timeSeries *ts2)
{

  return compareVec (ts1->data, ts2->data);

}

double compare (timeSeries *ts1, timeSeries *ts2, int cnt)
{
  return compareVec (ts1->data, ts2->data,cnt);
}



double compare (timeSeriesMulti *ts1, timeSeries *ts2, int stream)
{

  return compareVec (ts1->data[stream], ts2->data);
}

double compare (timeSeriesMulti *ts1, timeSeries *ts2, int stream, int cnt)
{

  return compareVec (ts1->data[stream], ts2->data,cnt);
}

double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2)
{
	double diff = 0;
  int cnt = std::min (ts1->cols, ts2->cols);

  for (int kk = 0; kk < cnt; ++kk)
    {
      diff += compareVec (ts1->data[kk], ts2->data[kk]);
    }

  return diff;
}

double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream)
{

  return compareVec (ts1->data[stream], ts2->data[stream]);
}

double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream1, int stream2)
{

  return compareVec (ts1->data[stream1], ts2->data[stream2]);
}

double compare (timeSeriesMulti *ts1, timeSeriesMulti *ts2, int stream1, int stream2, int cnt)
{

  return compareVec (ts1->data[stream1], ts2->data[stream2],cnt);
}
