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
#ifndef REVERSIBLE_EVENT_H_
#define REVERSIBLE_EVENT_H_

#include "gridEvent.h"

class gridGrabber;
/** an event that allows undoing,  it is a grabber and event rolled into one */
class reversibleEvent :public gridEvent
{
protected:
	double undoValue=kNullVal;
	std::unique_ptr<gridGrabber> ggrab;
	bool canUndo = false;
	bool hasUndo = false;
public:
	explicit reversibleEvent(const std::string &newName);
	explicit reversibleEvent(coreTime time0 = 0.0);
	reversibleEvent(gridEventInfo &gdEI, coreObject *rootObject);
	virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject);
	virtual ~reversibleEvent();
	virtual std::shared_ptr<gridEvent> clone(std::shared_ptr<gridEvent> ggb = nullptr) const override;

	//virtual void updateEvent(gridEventInfo &gdEI, coreObject *rootObject) override;
	virtual change_code trigger() override;
	virtual change_code trigger(coreTime time) override;

	virtual bool setTarget(coreObject *gdo, const std::string &var = "");

	virtual void updateObject(coreObject *gco, object_update_mode mode = object_update_mode::direct) override;
	virtual change_code undo();
	virtual double query();

};


#endif