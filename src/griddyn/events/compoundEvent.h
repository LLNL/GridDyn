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

#ifndef GRIDDYN_COMPOUND_EVENT_H_
#define GRIDDYN_COMPOUND_EVENT_H_
#pragma once

#include "Event.h"


namespace griddyn
{
namespace events
{
/** single event allowing multiple changes in multiple events at a single time point */
class compoundEvent :public Event
{
protected:
        stringVec fields;  //!< the vector of fields to modify
        std::vector<double> values;	//!< the vector of values to change to 
        std::vector<gridUnits::units_t> units;	//!< vector of units corresponding to the changes
        std::vector<coreObject *> targetObjects;	//!< the set of objects to target
public:
	explicit compoundEvent(const std::string &eventName);
	explicit compoundEvent(coreTime time0 = 0.0);
	compoundEvent(const EventInfo &gdEI, coreObject *rootObject);
	virtual std::unique_ptr<Event> clone() const override;

	virtual void cloneTo(Event *evnt) const override;
	
	//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void setValue( double val, gridUnits::units_t newUnits=gridUnits::defUnit) override;
	virtual void setValue(const std::vector<double> &val);
	virtual std::string to_string() override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;
	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual coreObject * getObject() const override;
	virtual void getObjects(std::vector<coreObject *> &objects) const override;
};
}//namespace events
}//namespace griddyn
#endif
