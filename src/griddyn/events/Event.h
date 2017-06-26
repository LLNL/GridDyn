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
#pragma once
// headers
//#include "griddyn.h"

#include "core/coreObject.h"
#include "core/helperObject.h"

#include "eventInterface.hpp"
#include "core/objectOperatorInterface.hpp"
#include "utilities/units.h"

namespace griddyn
{
/** helper data class for holding information about an event during construction
*/
class EventInfo
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
	EventInfo() = default;
  EventInfo(const std::string &eventString, coreObject *rootObj);

  void loadString(const std::string &eventString, coreObject *rootObj);
};

/** basic event class enabling a property change in an object
eventInterface, objectOperatorInterface are pure virtual interfaces
*/
class Event : public helperObject,public eventInterface, public objectOperatorInterface
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
	explicit Event(const std::string &eventName);
	/** default constructor will set the event time to the maximum so it will never trigger on its own
	@param[in] time0 the time to trigger the event
	*/
	explicit Event(coreTime time0 = maxTime);
	Event(EventInfo &gdEI, coreObject *rootObject);

	virtual std::shared_ptr<Event> clone(std::shared_ptr<Event> gE = nullptr) const;

		virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject);
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
		virtual void setFlag(const std::string &flag, bool val) override;
        virtual void setTime(coreTime time);
        virtual void setValue(double val, gridUnits::units_t unitType=gridUnits::defUnit);
        virtual std::string to_string();

        virtual bool setTarget(coreObject *gdo, const std::string &var = "");

		virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;

		virtual coreObject * getObject() const override;
		virtual void getObjects(std::vector<coreObject *> &objects) const override;
		
protected:
	void loadField(coreObject *gdo, const std::string &field);
	virtual bool checkArmed();
};



std::unique_ptr<Event> make_event (const std::string &field, double val, coreTime eventTime, coreObject *rootObject);
std::unique_ptr<Event> make_event (EventInfo &gdEI, coreObject *rootObject);
std::unique_ptr<Event> make_event(const std::string &field, coreObject *rootObject);

}//namespace griddyn
#endif
