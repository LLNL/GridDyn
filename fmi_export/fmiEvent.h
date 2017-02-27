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
#pragma once
#ifndef _FMI_EVENT_H_
#define _FMI_EVENT_H_

#include "events/gridEvent.h"

class fmiCoordinator;
/** class to manage the inputs for an FMI configuration in GridDyn*/
class fmiEvent : public gridEvent
{
public:
	enum class fmiEventType
	{
		parameter,
		input,
	};
private:
	fmiCoordinator *coord = nullptr;
	fmiEventType eventType = fmiEventType::input;
public:
	
	fmiEvent(const std::string &newName, fmiEventType type=fmiEventType::input);
	fmiEvent(fmiEventType type = fmiEventType::input);
	fmiEvent(gridEventInfo &gdEI, coreObject *rootObject);
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	virtual void set(const std::string &param, double val) override;
	virtual void set(const std::string &param, const std::string &val) override;

	virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
	
	virtual bool setTarget(coreObject *gdo, const std::string &var = "") override;

	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	friend class fmiCoordinator;
private:
	/** function to find the fmi coordinator so we can connect to that*/
	void findCoordinator();
};
#endif
