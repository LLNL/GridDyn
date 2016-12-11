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

#ifndef GRIDDYN_COLLECTOR_H_
#define GRIDDYN_COLLECTOR_H_

#include "timeSeriesMulti.h"
#include "eventInterface.h"

#include "units.h"
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
class collector : public eventInterface, objectOperatorInterface
{
public:
	std::string description; //!< description
protected:
	int warningCount = 0;  //!< counter for the number of warnings
	std::vector<std::string> warnList;  //!< listing for the number of warnings
	std::string  name;  //!< name of the collector
	gridDyn_time timePeriod; //!< the actual period of the collector
	gridDyn_time reqPeriod; //!< the requested period of the collector
	gridDyn_time startTime = negTime; //!< the time to start collecting
	gridDyn_time stopTime = maxTime;  //!< the time to stop collecting
	gridDyn_time triggerTime = negTime; //!< the next trigger time for the collector
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
		collectorPoint(std::shared_ptr<gridGrabber> dg, std::shared_ptr<stateGrabber> sg, int ncol = -1, int ccnt = 1, std::string cname = "")
			:dataGrabber(dg), dataGrabberSt(sg), column(ncol), columnCount(ccnt), colname(cname)
		{};
	};

	std::vector<collectorPoint> points;        //!<the data grabbers

	bool recheck = false;	//!< flag indicating that the recorder should recheck all the fields
	bool armed = true;	//!< flag indicating if the recorder is armed and ready to go
	bool delayProcess = true;          //!< wait to process recorders until other events have executed

public:
	collector(gridDyn_time time0 = timeZero, gridDyn_time period = 1.0);
	explicit collector(const std::string &name);
	virtual ~collector();

	virtual std::shared_ptr<collector> clone(std::shared_ptr<collector> gr = nullptr) const;

	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual change_code trigger(gridDyn_time time) override;
	void recheckColumns();
	gridDyn_time nextTriggerTime() const override
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

	virtual void set(const std::string &param, double val);
	virtual void set(const std::string &param, const std::string &val);

	
	virtual void setTime(gridDyn_time time);


	virtual coreObject * getObject() const override;

	virtual void getObjects(std::vector<coreObject *> &objects) const override;

	const std::string &getName() const
	{
		return name;
	}
	void setName(const std::string &newName)
	{
		name = newName;
	}
	virtual void flush();
	virtual const std::string &getSinkName() const;

	virtual std::vector<std::string> getColumnDescriptions() const;
	/** get the current warning count*/
	int getWarningCount() const
	{
		return warningCount;
	}
	const std::vector<std::string> &getWarnings() const
	{
		return warnList;
	}

	void clearWarnings()
	{
		warnList.clear();
		warningCount = 0;
	}
protected:
	virtual void dataPointAdded(const collectorPoint& cp);
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
std::shared_ptr<collector> makeCollector(const std::string &type,const std::string &name="");

/** class to store and save data from the grid, based on a collector */
class gridRecorder : public collector
{
protected:
  
  gridDyn_time lastSaveTime = negTime; //!< the last time the recorder saved to file
  timeSeriesMulti<double,gridDyn_time> dataset;  //!< the actual time series data
  std::string filename; //!< the filename to store the data
  std::string directory; //!< the directory to generate the specified file 
  
  bool binaryFile = true;	//!< flag indicating if the file is binary
  bool firstTrigger = true; //!< flag indicating that the recorder has not been triggered yet
  int precision = -1;                //!< precision for writing text files.
  count_t autosave = 0;			//!< flag indicating the recorder should autosave after the given number of points
public:
  gridRecorder (gridDyn_time time0 = timeZero,gridDyn_time period = 1.0);
  explicit gridRecorder(const std::string &name);
  ~gridRecorder ();

  virtual std::shared_ptr<collector> clone (std::shared_ptr<collector> gr=nullptr) const override;

  virtual change_code trigger (gridDyn_time time) override;

  
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

  const timeSeriesMulti<double,gridDyn_time> * getData () const
  {
    return &(dataset);
  }
  const std::vector<gridDyn_time> &getTime () const
  {
    return dataset.time;
  }
  const std::vector<double> &getData (size_t col) const
  {
  
       return dataset.data[(col < columns)?col:0];
  }
private:
	void fillDatasetFields();
};

#endif