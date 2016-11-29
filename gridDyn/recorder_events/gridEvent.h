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

#ifndef GRIDDYN_EVENT_H_
#define GRIDDYN_EVENT_H_

// headers
//#include "gridDyn.h"

#include "gridCore.h"
#include "timeSeriesMulti.h"

#include "eventInterface.h"
#include "core/objectOperatorInterface.h"
#include "units.h"

/** helper data class for holding information about an event during construction
*/
class gridEventInfo
{
public:
  std::string name;
  std::string description;
  std::string type;
  std::string file;
  gridDyn_time period = timeZero;
  std::vector<gridDyn_time> time;
  std::vector<double> value;
  stringVec fieldList;
  std::vector<gridCoreObject *>targetObjs;
  std::vector<index_t> columns;
  std::vector<gridUnits::units_t> units;
public:
  gridEventInfo ()
  {
  }
  gridEventInfo(const std::string &eventString, gridCoreObject *rootObj);

  void loadString(const std::string &eventString, gridCoreObject *rootObj);
};

/** basic event class enabling a property change in an object */
class gridEvent : public eventInterface, objectOperatorInterface
{
public:
        std::string description;  //!< event description
protected:
		std::string  name;		//!< event name
		std::string field;		//!< event trigger field
		index_t eventId;		//!< a unique Identifier code for the event
        double value=0.0;			//!< new value
		gridDyn_time triggerTime;		//!< the time the event is scheduled to be triggered
        gridCoreObject *m_obj = nullptr;		//!< the target object of the event
		gridUnits::units_t unitType = gridUnits::defUnit; //!< units of the event
        bool armed = false;		//!< flag indicating if the event is armed or not
		bool resettable = false;  //!< flag indicating if the event can be reset;
		bool reversable = false; //!< flag indicating if the event can be reversed;
private:
	static std::atomic<count_t> eventCount;
public:
	explicit gridEvent(const std::string &newName);
	explicit gridEvent(gridDyn_time time0 = negTime);
	gridEvent(gridEventInfo *gdEI, gridCoreObject *rootObject);

	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const;
        virtual ~gridEvent();

		virtual void updateEvent(gridEventInfo *gdEI, gridCoreObject *rootObject);
        virtual change_code trigger();
        virtual change_code trigger(gridDyn_time time) override;

        virtual gridDyn_time nextTriggerTime() const override
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
		virtual void set(const std::string &param, double val);
		virtual void set(const std::string &param, const std::string &val);
        virtual void setTime(gridDyn_time time);
        virtual void setValue(double val, gridUnits::units_t unitType=gridUnits::defUnit);
        virtual std::string toString();

        virtual bool setTarget(gridCoreObject *gdo, const std::string &var = "");

		virtual void updateObject(gridCoreObject *gco, object_update_mode mode = object_update_mode::direct) override;

		virtual gridCoreObject * getObject() const override;
		virtual void getObjects(std::vector<gridCoreObject *> &objects) const override;
		const std::string &getName() const
		{
			return name;
		}
		void setName(std::string newName)
		{
			name = newName;
		}
protected:
	void loadField(gridCoreObject *gdo, const std::string field);
	virtual bool checkArmed();
};

/** single event allowing multiple changes in multiple events at a single time point */
class compoundEvent :public gridEvent
{
protected:
        stringVec fields;
        std::vector<double> values;
        std::vector<gridUnits::units_t> units;
        std::vector<gridCoreObject *> targetObjects;
public:
	explicit compoundEvent(const std::string &newName);
	explicit compoundEvent(gridDyn_time time0 = 0.0);
	compoundEvent(gridEventInfo *gdEI, gridCoreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone( std::shared_ptr<gridEvent> ggb = nullptr) const override;
	
	//virtual void updateEvent(gridEventInfo *gdEI, gridCoreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(gridDyn_time time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void setValue( double val, gridUnits::units_t newUnits=gridUnits::defUnit) override;
	virtual void setValue(const std::vector<double> &val);
	virtual std::string toString() override;

	virtual bool setTarget(gridCoreObject *gdo, const std::string &var = "") override;
	virtual void updateObject(gridCoreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual gridCoreObject * getObject() const override;
	virtual void getObjects(std::vector<gridCoreObject *> &objects) const override;
};

/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class gridPlayer: public gridEvent
{
protected:
        gridDyn_time period = maxTime;  //!< period of the player
        timeSeries<double,gridDyn_time> ts;	//!< the time series containing the data for the player 
        index_t currIndex = kNullLocation;	//!< the current index of the player
        std::string eFile;		//!< the file name
		index_t column = 0;
public:
	explicit gridPlayer(const std::string &newName);
        gridPlayer(gridDyn_time time0 = 0.0, double loopPeriod = 0.0);
		gridPlayer(gridEventInfo *gdEI, gridCoreObject *rootObject);
		virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

		virtual void updateEvent(gridEventInfo *gdEI, gridCoreObject *rootObject) override;
		virtual change_code trigger() override;
		virtual change_code trigger(gridDyn_time time) override;

		virtual void set(const std::string &param, double val) override;
		virtual void set(const std::string &param, const std::string &val) override;
        void setTime(gridDyn_time time) override;
        void setTimeValue(gridDyn_time time, double val);
        void setTimeValue(const std::vector<gridDyn_time> &time, const std::vector<double> &val);
        void loadEventFile(const std::string &fname);
        virtual std::string toString() override;

        virtual bool setTarget(gridCoreObject *gdo, const std::string &var = "") override;

        //friendly helper functions for sorting
protected:
        virtual void updateTrigger(gridDyn_time time);
		virtual void setNextValue();
};

/** event type allowing multiple changes on multiple object at a set of given time points*/
class compoundEventPlayer : public compoundEvent
{
protected:
	gridDyn_time period = maxTime;  //!< period of the player
	timeSeriesMulti<double,gridDyn_time> ts;	//!< the time series containing the data for the player 
	index_t currIndex = kNullLocation;	//!< the current index of the player
	std::string eFile;		//!< the file name
	std::vector<index_t> columns;
public:
	explicit compoundEventPlayer(const std::string &newName);
	compoundEventPlayer();
	compoundEventPlayer(gridEventInfo *gdEI, gridCoreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	//virtual void updateEvent(gridEventInfo *gdEI, gridCoreObject *rootObject) override;

	virtual change_code trigger() override;
	virtual change_code trigger(gridDyn_time time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	void setTime(gridDyn_time time) override;
	void setTimeValue(gridDyn_time time, double val);
	void setTimeValue(const std::vector<gridDyn_time> &time, const std::vector<double> &val);
	void loadEventFile(const std::string &fname);
	virtual std::string toString() override;

	virtual bool setTarget(gridCoreObject *gdo, const std::string &var = "") override;

protected:
	virtual void updateTrigger(gridDyn_time time);

};

/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class interpolatingPlayer : public gridPlayer
{
protected:
	std::string slopeField;
	bool useSlopeField = false;
	double samplePeriod = kBigNum;
	double slope;
public:
	explicit interpolatingPlayer(const std::string &newName);
	interpolatingPlayer(gridDyn_time time0 = 0.0, double loopPeriod = 0.0);
	interpolatingPlayer(gridEventInfo *gdEI, gridCoreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	//virtual void updateEvent(gridEventInfo *gdEI, gridCoreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(gridDyn_time time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	
	virtual std::string toString() override;

	//friendly helper functions for sorting
protected:
	virtual void setNextValue() override;
};

std::shared_ptr<gridEvent> make_event (const std::string &field, double val, gridDyn_time eventTime, gridCoreObject *rootObject);
std::shared_ptr<gridEvent> make_event (gridEventInfo *gdEI, gridCoreObject *rootObject);
std::shared_ptr<gridEvent> make_event(const std::string &field, gridCoreObject *rootObject);
#endif
