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

#ifndef GRIDDYN_COLLECTOR_H_
#define GRIDDYN_COLLECTOR_H_

#include "utilities/timeSeriesMulti.h"
#include "events/eventInterface.h"
#include "core/helperObject.h"
#include "utilities/units.h"
#include "core/objectOperatorInterface.h"
#include <memory>

class gridGrabberInfo
{
public:
  std::string  m_target;        //!< name of the object to target
  std::string field;        //!<the field to record
  std::string rString;     //!<a string defining the recorder
  int column = -1;      //!< (suggestion) which column to stick the data in
  index_t offset = kNullLocation;      //!<the offset to use to numerically pick off the state
  double gain = 1.0;      //!<a multiplier factor for the results
  double bias = 0.0;       //!< a shift factor of the results
  gridUnits::units_t outputUnits = gridUnits::defUnit;       //!<which units to output the data
public:
  gridGrabberInfo ()
  {
  }
};

class gridGrabber;
class stateGrabber;

/** class for capturing and storing data from a grid simulation */
class collector : public helperObject,public eventInterface, public objectOperatorInterface
{
protected:
	int warningCount = 0;  //!< counter for the number of warnings
	std::vector<std::string> warnList;  //!< listing for the number of warnings
	coreTime timePeriod; //!< the actual period of the collector
	coreTime reqPeriod; //!< the requested period of the collector
	coreTime startTime = negTime; //!< the time to start collecting
	coreTime stopTime = maxTime;  //!< the time to stop collecting
	coreTime triggerTime = maxTime; //!< the next trigger time for the collector
	coreTime lastTriggerTime = negTime; //!< the last time the collector was triggered
	std::vector<double> data;
	count_t columns = 0; //!< the length of the data vector
	/** data structure to capture the grabbers and location for a specific grabber*/
	class collectorPoint
	{
	public:
		std::shared_ptr<gridGrabber> dataGrabber; //!< the grabber for the data from the object directly
		std::shared_ptr<stateGrabber> dataGrabberSt;  //!< the grabber for the data from the object state
		int column=-1; //!< the starting column for the data
		int columnCount=1;  //!< the number of columns associated with the point
		std::string colname;   //!< the name for the data collected
		collectorPoint(std::shared_ptr<gridGrabber> dg, std::shared_ptr<stateGrabber> sg, int ncol = -1, int ccnt = 1, const std::string &cname = "")
			:dataGrabber(std::move(dg)), dataGrabberSt(std::move(sg)), column(ncol), columnCount(ccnt), colname(cname)
		{};
	};

	std::vector<collectorPoint> points;        //!<the data grabbers

	bool recheck = false;	//!< flag indicating that the recorder should recheck all the fields
	bool armed = true;	//!< flag indicating if the recorder is armed and ready to go
	bool delayProcess = true;          //!< wait to process recorders until other events have executed

public:
	collector(coreTime time0 = timeZero, coreTime period = timeOneSecond);
	explicit collector(const std::string &name);

	virtual std::shared_ptr<collector> clone(std::shared_ptr<collector> gr = nullptr) const;

	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual change_code trigger(coreTime time) override;
	void recheckColumns();
	coreTime nextTriggerTime() const override
	{
		return triggerTime;
	}
	event_execution_mode executionMode() const override
	{
		return (delayProcess) ? event_execution_mode::delayed : event_execution_mode::normal;
	}

	virtual void add(std::shared_ptr<gridGrabber> ggb, int column = -1);
	virtual void add(std::shared_ptr<stateGrabber> sst, int column = -1);
	virtual void add(gridGrabberInfo *gdRI, coreObject *obj);
	virtual void add(const std::string &field, coreObject *obj);
	virtual void add(std::shared_ptr<gridGrabber> ggb, std::shared_ptr<stateGrabber> sst, int column);

	bool isArmed() const override
	{
		return armed;
	}

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	
	virtual void setTime(coreTime time);


	virtual coreObject * getObject() const override;

	virtual void getObjects(std::vector<coreObject *> &objects) const override;

	virtual void flush();
	virtual const std::string &getSinkName() const;

	/** the the most recent value associated with a particular column
	@param[in] column the column of the data to get
	@return the most recent value
	*/
	double getValue(index_t column) const
	{
		return (column < data.size()) ? data[column] : kNullVal;
	}
	virtual std::vector<std::string> getColumnDescriptions() const;
	/** get the current warning count*/
	int getWarningCount() const
	{
		return warningCount;
	}
	/** get a list of the warnings that were generated on construction
	@return a vector of the warnings 
	*/
	const std::vector<std::string> &getWarnings() const
	{
		return warnList;
	}
	/** erase the warning list*/
	void clearWarnings()
	{
		warnList.clear();
		warningCount = 0;
	}
protected:
	/** callback intended more for child classes to indicate that a dataPoint has been added*/
	virtual void dataPointAdded(const collectorPoint& cp);
	/** get a column number, the requested column is a request only
	*@param[in] requestedColumn the column that is being requested
	@return the actual column granted*/
	int getColumn(int requestedColumn);

	void updateColumns(int requestedColumn);
	void addWarning(const std::string &warnMessage)
	{
		warnList.push_back(warnMessage);
		++warningCount;
	}
	void addWarning(std::string &&warnMessage)
	{
		warnList.push_back(warnMessage);
		++warningCount;
	}
};


/** @brief make a collector from a string
@param[in] type the type of collector to create
@return a shared_ptr to a collector object
*/
std::unique_ptr<collector> makeCollector(const std::string &type,const std::string &name="");

/** class to store and save data from the grid, based on a collector */
class gridRecorder : public collector
{
protected:
  
  coreTime lastSaveTime = negTime; //!< the last time the recorder saved to file
  timeSeriesMulti<double,coreTime> dataset;  //!< the actual time series data
  std::string filename; //!< the filename to store the data
  std::string directory; //!< the directory to generate the specified file 
  
  bool binaryFile = true;	//!< flag indicating if the file is binary
  bool firstTrigger = true; //!< flag indicating that the recorder has not been triggered yet
  int precision = -1;                //!< precision for writing text files.
  count_t autosave = 0;			//!< flag indicating the recorder should autosave after the given number of points
public:
  gridRecorder (coreTime time0 = timeZero,coreTime period = 1.0);
  explicit gridRecorder(const std::string &name);
  ~gridRecorder ();

  virtual std::shared_ptr<collector> clone (std::shared_ptr<collector> gr=nullptr) const override;

  virtual change_code trigger (coreTime time) override;

  
  void saveFile (const std::string &fileName = "");

  void setSpace (double span);
  void addSpace (double span);
 
  void set (const std::string &param, double val) override;
  void set (const std::string &param, const std::string &val) override;

  virtual void flush() override;
  virtual const std::string &getSinkName() const override;

  const std::string &getFileName () const
  {
    return filename;
  }
  const std::string &getDirectory () const
  {
    return directory;
  }
  
  void reset ();

  //const timeSeriesMulti<double,coreTime> * getData () const
   const auto &getTimeSeries() const
  {
    return dataset;
  }
  const std::vector<coreTime> &getTime () const
  {
    return dataset.time();
  }
  const std::vector<double> &getData (count_t col) const
  {
       return dataset.data((col < columns)?col:0);
  }
private:
	void fillDatasetFields();
};

#endif