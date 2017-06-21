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
#pragma once
#ifndef GRIDDYN_INTERPOLATING_PLAYER_H_
#define GRIDDYN_INTERPOLATING_PLAYER_H_

// headers
//#include "griddyn.h"

#include "Player.h"
namespace griddyn
{
namespace events
{
/** event player allowing a timeSeries of events to occur over numerous time points on a single object and field*/
class interpolatingPlayer : public Player
{
protected:
	std::string slopeField; //!< the object field to trigger for a slope input
	double samplePeriod = kBigNum; //!< the sampling period to update the interpolated value
	double slope=0.0;	//!< the actual slope to use
	bool useSlopeField = false;	//!< flag indicating that the event is actually using the slopefield
public:
	explicit interpolatingPlayer(const std::string &eventName);
	interpolatingPlayer(coreTime time0 = 0.0, double loopPeriod = 0.0);
	interpolatingPlayer(EventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<Event> clone(std::shared_ptr<Event> gE = nullptr) const override;

	//virtual void updateEvent(EventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;
	virtual void setFlag(const std::string &flag, bool val) override;
	virtual std::string to_string() override;

	//friendly helper functions for sorting
protected:
	virtual void setNextValue() override;
};
}//namespace events
}//namespace griddyn
#endif
