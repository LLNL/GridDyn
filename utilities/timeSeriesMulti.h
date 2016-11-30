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

#ifndef TIMESERIESMULTI_H_
#define TIMESERIESMULTI_H_

#include "timeSeries.h"

template <typename dataType=double, typename timeType=double>
class timeSeriesMulti
{
public:
	std::string description;  //!< a description of the time series
	std::vector<timeType> time;	//!< a vector of times associated with the data
	std::vector<std::vector<dataType> > data;  //!< a 2d vector of data to store the time series information
	fsize_t cols = 1;		//!< the number of columns of data
	fsize_t count = 0;	//!< the current data location
	fsize_t capacity = 0;	//!< the total capacity of the time series
	std::vector<std::string> fields;	//!< container for all the strings associated with the different columns
public:
	timeSeriesMulti()
	{
		data.resize(1);
		fields.resize(1);
	}
	explicit timeSeriesMulti(fsize_t numCols)
	{
		cols = 1;
		data.resize(1);
		setCols(numCols);
	}
	timeSeriesMulti(fsize_t numCols, fsize_t numRows)
	{
		cols = 1;
		count = numRows;
		setCols(numCols);
		resize(numRows);
	}
	explicit timeSeriesMulti(const std::string &fname)
	{
		loadFile(fname);
	}
	/** add a data point to the time series
	@param[in] t the time
	@param[in] point the value
	@param[in] column the column to add the data to column 0 for default
	@return true if the data was successfully added
	*/
	bool addData(timeType t, dataType point, unsigned int column = 0)
	{
		if (column >= cols)
		{
			return false;
		}
		if (count > 0)
		{
			if (t - time[count - 1] > timeType(0.0))
			{
				time.push_back(t);
				++count;
			}
		}
		else
		{
			time.push_back(t);
			++count;
		}
		data[column].push_back(point);

		return true;

	}
	/** add a vector of data points to the time series
	@param[in] t the time
	@param[in] ndata the vector of values
	@param[in] column the column to start adding the data to column 0 for default
	@return true if the data was successfully added
	*/
	bool addData(timeType t, std::vector<dataType> &ndata, unsigned int column = 0)
	{
		if (ndata.size() + column > cols)
		{
			return false;
		}
		if (count > 0)
		{
			if (t - time[count - 1] > timeType(0.0))
			{
				time.push_back(t);
				++count;
			}
		}
		else
		{
			time.push_back(t);
			++count;
		}
		auto dv = data.begin() + column;
		for (auto newDataPoint : ndata)
		{
			dv->push_back(newDataPoint);
			++dv;
		}

		return true;
	}
	/** add a time series of data points
	@param[in] ndata the vector of values
	@param[in] column the column the data represents
	@return true if the data was successfully added
	*/
	bool addData(std::vector<dataType> &ndata, unsigned int column)
	{
		if (ndata.size() != count)
		{
			return false;
		}
		if (column >= cols)
		{
			setCols(column);
		}
		data[column] = ndata;
		return true;
	}
	/** add a vector of data points and the time vector
	@param[in] ntime the vector of times
	@param[in] ndata the vector of values
	@param[in] column the column to start adding the data to column 0 for default
	@return true if the data was successfully added
	*/
	bool addData(std::vector<timeType> &ntime, std::vector<dataType> &ndata, unsigned int column = 0)
	{
		if (ntime.size() != ndata.size())
		{
			return false;
		}
		if (column >= cols)
		{
			setCols(column);
		}
		time = ntime;

		data[column] = ndata;
		count = static_cast<fsize_t> (ntime.size());
		return true;

	}

	void resize(fsize_t newSize)
	{
		time.resize(newSize, timeType(0.0));
		for (auto &dk : data)
		{
			dk.resize(newSize, dataType(0.0));
		}
		count = newSize;
	}

	void reserve(fsize_t newCapacity)
	{
		time.reserve(newCapacity);
		capacity = newCapacity;
		for (auto &dk : data)
		{
			dk.reserve(newCapacity);
		}
	}

	void setCols(fsize_t newCols)
	{

		fields.resize(newCols);
		data.resize(newCols);
		for (size_t kk = cols; kk < newCols; ++kk)
		{
			data[kk].reserve(std::max(capacity, count));
			data[kk].resize(count);
		}
		cols = newCols;
	}
	void clear()
	{
		time.clear();
		for (auto &dk : data)
		{
			dk.clear();
		}
		count = 0;
	}

	/** @brief load a file into the time series
	automatically detect the file type based on extension
	@param[in] filename  the file to load
	@return the number of points that were loaded
	*/
	void loadFile(const std::string &filename)
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

	void loadBinaryFile(const std::string &filename)
	{
		std::ifstream fio(filename.c_str(), std::ios::in | std::ios::binary);
		if (!fio)
		{
			throw(fileNotFoundError());
		}
		fsize_t nc;
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

			fio.read(dbuff.data(), dcount);
			description = std::string(dbuff.data(), dcount);
		}
		fio.read((char *)(&nc), sizeof(fsize_t));
		fio.read((char *)(&rcount), sizeof(fsize_t));

		setCols(rcount - 1);       //update the number of columns the file contains the time, then the data columns
		resize(nc);       // update the size



						  //now read the field names
		unsigned char len;
		for (fsize_t cc = 0; cc < cols; cc++)
		{
			fio.read((char *)(&len), 1);
			if ((len > 0)&&(len<=256))
			{
				fio.read(dbuff.data(), len);
				fields[cc] = std::string(dbuff.data(), len);
			}
			else if (len > 256)
			{
				fio.read(dbuff.data(), 256);
				fields[cc] = std::string(dbuff.data(), 256);
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
			fio.read((char *)(data[cc].data()), nc * sizeof(dataType));
			//data[cc] = std::vector<double>(buf, buf + nc);
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
				fio.read((char *)(data[cc].data() + ocount), nc * sizeof(dataType));
				//data[cc] = std::vector<double>(buf, buf + nc);
			}
			ocount += nc;
			//read the next character
			fio.read((char *)(&nc), sizeof(fsize_t));
			
		}

		time = vectorConvert<timeType>(std::move(buf));

		fio.close();


	}
	void loadTextFile(const std::string &filename)
	{
		std::ifstream fio(filename.c_str(), std::ios::in);
		unsigned int kk;
		if (!fio)
		{
			throw(fileNotFoundError());
		}
		std::string line;
		std::getline(fio, line);
		while (line[0] == '#')
		{
			if (line.size() > 2)
			{
				if (description.empty())
				{
					description = line.substr(1);
				}
				else
				{
					description += '\n' + line.substr(1);
				}
			}
			std::getline(fio, line);
		}

		auto colnames = splitline(line, ',');
		setCols(static_cast<fsize_t> (colnames.size()) - 1);
		for (kk = 1; kk < colnames.size(); ++kk)
		{
			fields[kk - 1] = removeChar(colnames[kk], '"');
		}
		clear();
		timeType timeV;
		while (std::getline(fio, line))
		{
			auto svc = str2vector(line, -1e48, ",");
			timeV = svc[0];
			for (kk = 1; kk < svc.size(); ++kk)
			{
				addData(timeV, svc[kk], kk - 1);
			}
		}
		fio.close();

	}
	void writeBinaryFile(const std::string &filename, bool append = false)
	{
		std::ofstream fio(filename.c_str(), std::ios::out | std::ios::binary | ((append) ? (std::ios::app) : (std::ios::trunc)));
		if (!fio)
		{
			throw(fileNotFoundError());
		}
		if (!append)
		{
			fsize_t temp = 1;
			fio.write((const char *)&temp, sizeof(int));
			temp = static_cast<fsize_t> (description.length());
			fio.write((const char *)&temp, sizeof(int));
			if (temp > 0)
			{
				fio.write(description.c_str(), temp);
			}

			//now write the size of the data
			temp = count;
			fio.write((const char *)&temp, sizeof(fsize_t));
			temp = cols + 1;
			fio.write((const char *)&temp, sizeof(fsize_t));
			//now write the column names
			unsigned char ccnt = 0;
			for (fsize_t cc = 0; cc < cols; cc++)
			{
				if (fields[cc].size() > 255)
				{
					ccnt = 255;
				}
				else
				{
					ccnt = static_cast<unsigned char> (fields[cc].size());
				}
				fio.write((const char *)&ccnt, 1);
				if (ccnt > 0)
				{
					fio.write(fields[cc].c_str(), ccnt);
				}
			}
		}
		else
		{
			fsize_t temp = count;
			fio.write((const char *)&temp, sizeof(fsize_t));
			temp = cols + 1;
			fio.write((const char *)&temp, sizeof(fsize_t));
		}
		//now write the data
		if (count > 0)
		{
			
			for (auto &t : time)
			{
				double tr = static_cast<double>(t);
				fio.write((const char *)(&tr), sizeof(double));
			}
			for (fsize_t cc = 0; cc < cols; cc++)
			{
				fio.write((const char *)(data[cc].data()), count * sizeof(dataType));
			}
		}

		fio.close();

	}
	void writeTextFile(const std::string &filename, int precision = 8, bool append = false)
	{

		std::ofstream fio(filename.c_str(), std::ios::out | ((append) ? (std::ios::app) : (std::ios::trunc)));
		if (!fio)
		{
			throw(fileNotFoundError());
		}
		std::string ndes = characterReplace(description, '\n', "\n#");

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
			fio << std::setprecision(5) << time[rr];
			for (size_t kk = 0; kk < cols; ++kk)
			{
				fio << ',' << std::setprecision(precision) << data[kk][rr];
			}
			fio << '\n';
		}
		fio.close();

	}

private:
};

//comparison functions
template <typename dataType, typename timeType>
dataType compare(timeSeries<dataType,timeType> *ts1, timeSeries<dataType, timeType> *ts2)
{
	return compareVec(ts1->data, ts2->data);
}

template <typename dataType, typename timeType>
dataType compare(timeSeries<dataType, timeType> *ts1, timeSeries<dataType, timeType> *ts2, int cnt)
{
	return compareVec(ts1->data, ts2->data, cnt);
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeries<dataType, timeType> *ts2, int stream)
{

	return compareVec(ts1->data[stream], ts2->data);
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeries<dataType, timeType> *ts2, int stream, int cnt)
{
	return compareVec(ts1->data[stream], ts2->data, cnt);
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeriesMulti<dataType, timeType> *ts2)
{
	double diff = 0;
	int cnt = std::min(ts1->cols, ts2->cols);

	for (int kk = 0; kk < cnt; ++kk)
	{
		diff += compareVec(ts1->data[kk], ts2->data[kk]);
	}

	return diff;
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeriesMulti<dataType, timeType> *ts2, int stream)
{
	return compareVec(ts1->data[stream], ts2->data[stream]);
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeriesMulti<dataType, timeType> *ts2, int stream1, int stream2)
{
	return compareVec(ts1->data[stream1], ts2->data[stream2]);
}

template <typename dataType, typename timeType>
dataType compare(timeSeriesMulti<dataType, timeType> *ts1, timeSeriesMulti<dataType, timeType> *ts2, int stream1, int stream2, int cnt)
{
	return compareVec(ts1->data[stream1], ts2->data[stream2], cnt);
}

#endif