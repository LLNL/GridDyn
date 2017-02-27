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

#ifndef GRIDDYN_EVENT_H_
#define GRIDDYN_EVENT_H_

// headers
//#include "gridDyn.h"

#include "core/coreObject.h"
#include "core/helperObject.h"
#include "timeSeriesMulti.h"

#include "eventInterface.h"
#include "core/objectOperatorInterface.h"
#include "units.h"

#include <future>
/** helper data class for holding information about an event during construction
*/
class gridEventInfo
{
public:
  std::string name;
  std::string description;
  std::string type;
  std::string file;
  coreTime period = timeZero;
  std::vector<coreTime> time;
  std::vector<double> value;
  stringVec fieldList;
  std::vector<coreObject *>targetObjs;
  std::vector<index_t> columns;
  std::vector<gridUnits::units_t> units;
public:
  gridEventInfo ()
  {
  }
  gridEventInfo(const std::string &eventString, coreObject *rootObj);

  void loadString(const std::string &eventString, coreObject *rootObj);
};

/** basic event class enabling a property change in an object
eventInterface, objectOperatorInterface are pure virtual interfaces
*/
class gridEvent : public helperObject,public eventInterface, public objectOperatorInterface
{

protected:
		std::string field;		//!< event trigger field
        double value=0.0;			//!< new value
		coreTime triggerTime;		//!< the time the event is scheduled to be triggered
        coreObject *m_obj = nullptr;		//!< the target object of the event
		gridUnits::units_t unitType = gridUnits::defUnit; //!< units of the event
		index_t eventId;		//!< a unique Identifier code for the event
        bool armed = false;		//!< flag indicating if the event is armed or not
		bool resettable = false;  //!< flag indicating if the event can be reset;
		bool reversable = false; //!< flag indicating if the event can be reversed;
		bool initRequired = false;  //!< flag indicating the event requires initialization

public:
	explicit gridEvent(const std::string &newName);
	/** default constructor will set the event time to the maximum so it will never trigger on its own
	@param[in] time0 the time to trigger the event
	*/
	explicit gridEvent(coreTime time0 = maxTime);
	gridEvent(gridEventInfo &gdEI, coreObject *rootObject);

	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const;

		virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject);
        virtual change_code trigger();
        virtual change_code trigger(coreTime time) override;

        virtual coreTime nextTriggerTime() const override
        {
                return triggerTime;
        }
        virtual bool isArmed() const override
        {
                return armed;
        }
		event_execution_mode executionMode() const override
		{
			return event_execution_mode::normal;
		}
		virtual void set(const std::string &param, double val) override;
		virtual void set(const std::string &param, const std::string &val) override;
		virtual void setFlag(const std::string &param, bool val) override;
        virtual void setTime(coreTime time);
        virtual void setValue(double val, gridUnits::units_t unitType=gridUnits::defUnit);
        virtual std::string toString();

        virtual bool setTarget(coreObject *gdo, const std::string &var = "");

		virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;

		virtual coreObject * getObject() const override;
		virtual void getObjects(std::vector<coreObject *> &objects) const override;
		
protected:
	void loadField(coreObject *gdo, const std::string &field);
	virtual bool checkArmed();
};

/** single event allowing multiple changes in multiple events at a single time point */
class compoundEvent :public gridEvent
{
protected:
        stringVec fields;
        std::vector<double> values;
        std::vector<gridUnits::units_t> units;
        std::vector<coreObject *> targetObjects;
public:
	explicit compoundEvent(const std::string &newName);
	explicit compoundEvent(coreTime time0 = 0.0);
	compoundEvent(gridEventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone( std::shared_ptr<gridEvent> ggb = nullptr) const override;
	
	//virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void setValue( double val, gridUnits::units_t newUnits=gridUnits::defUnit) override;
	virtual void setValue(const std::vector<double> &val);
	virtual std::string toString() override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;
	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual coreObject * getObject() const override;
	virtual void getObjects(std::vector<coreObject *> &objects) const override;
};

/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class gridPlayer: public gridEvent
{
protected:
        coreTime period = maxTime;  //!< period of the player
        timeSeries<double,coreTime> ts;	//!< the time series containing the data for the player 
        index_t currIndex = kNullLocation;	//!< the current index of the player
		index_t column = 0;
		coreTime timeOffset = timeZero; //!< an offset to the time series time
		std::string eFile;		//!< the file name
		std::future<void> fileLoaded;
		bool loadFileProcess = false;
public:
	explicit gridPlayer(const std::string &newName);
        gridPlayer(coreTime time0 = 0.0, double loopPeriod = 0.0);
		gridPlayer(gridEventInfo &gdEI, coreObject *rootObject);
		virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

		virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
		virtual change_code trigger() override;
		virtual change_code trigger(coreTime time) override;

		virtual void set(const std::string &param, double val) override;
		virtual void set(const std::string &param, const std::string &val) override;
        void setTime(coreTime time) override;
        void setTimeValue(coreTime time, double val);
        void setTimeValue(const std::vector<coreTime> &time, const std::vector<double> &val);
        void loadEventFile(const std::string &fname);
        virtual std::string toString() override;

        virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;

		virtual void initialize() override;
        //friendly helper functions for sorting
protected:
        virtual void updateTrigger(coreTime time);
		virtual void setNextValue();
};

/** event type allowing multiple changes on multiple object at a set of given time points*/
class compoundEventPlayer : public compoundEvent
{
protected:
	coreTime period = maxTime;  //!< period of the player
	timeSeriesMulti<double,coreTime> ts;	//!< the time series containing the data for the player 
	index_t currIndex = kNullLocation;	//!< the current index of the player
	std::string eFile;		//!< the file name
	std::vector<index_t> columns;
public:
	explicit compoundEventPlayer(const std::string &newName);
	compoundEventPlayer();
	compoundEventPlayer(gridEventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	//virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;

	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	void setTime(coreTime time) override;
	void setTimeValue(coreTime time, double val);
	void setTimeValue(const std::vector<coreTime> &time, const std::vector<double> &val);
	void loadEventFile(const std::string &fname);
	virtual std::string toString() override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;
	virtual void initialize() override;
protected:
	virtual void updateTrigger(coreTime time);

};

/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class interpolatingPlayer : public gridPlayer
{
protected:
	std::string slopeField; //!< the object field to trigger for a slope input
	double samplePeriod = kBigNum; //!< the sampling period to update the interpolated value
	double slope=0.0;	//!< the actual slope to use
	bool useSlopeField = false;	//!< flag indicating that the event is actually using the slopefield
public:
	explicit interpolatingPlayer(const std::string &newName);
	interpolatingPlayer(coreTime time0 = 0.0, double loopPeriod = 0.0);
	interpolatingPlayer(gridEventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	//virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void setFlag(const std::string &flag, bool val) override;
	virtual std::string toString() override;

	//friendly helper functions for sorting
protected:
	virtual void setNextValue() override;
};

std::unique_ptr<gridEvent> make_event (const std::string &field, double val, coreTime eventTime, coreObject *rootObject);
std::unique_ptr<gridEvent> make_event (gridEventInfo &gdEI, coreObject *rootObject);
std::unique_ptr<gridEvent> make_event(const std::string &field, coreObject *rootObject);
#endif
