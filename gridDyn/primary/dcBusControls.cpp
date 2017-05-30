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

#include "busControls.h"
#include "dcBus.h"
#include "linkModels/dcLink.h"
#include "utilities/vectorOps.hpp"


dcBusControls::dcBusControls(dcBus *busToControl):controlledBus(busToControl)
{

}

bool dcBusControls::hasAdjustments() const
{
	return ((!controlObjects.empty()) || (!proxyControlObject.empty()));
}

bool dcBusControls::hasAdjustments(index_t sid) const
{
	for (auto &adj : controlObjects)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	for (auto &adj : proxyControlObject)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	return false;
}



double dcBusControls::getAdjustableCapacityUp(coreTime time) const
{
	double cap = 0.0;

	for (auto &adj : controlObjects)
	{
		cap += adj->getAdjustableCapacityUp(time);
	}
	//TODO:: links do not have this function yet
	/*
	for (auto &adj : pControlLinks)
	{

	//cap += adj->getAdjustableCapacityUp(time);
	}
	*/
	return cap;
}

double dcBusControls::getAdjustableCapacityDown(coreTime time) const
{
	double cap = 0.0;

	for (auto &adj : controlObjects)
	{
		cap += adj->getAdjustableCapacityDown(time);
	}
	//TODO:: links do not have this function yet
	/*
	for (auto &adj : pControlLinks)
	{
	cap += adj->getAdjustableCapacityDown(time);
	}
	*/
	return cap;
}



void dcBusControls::addControlObject(gridObject *obj, bool update)
{
	if (dynamic_cast<gridSecondary *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : controlObjects)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		controlObjects.push_back(static_cast<gridSecondary *>(obj));
		cfrac.push_back(obj->get("participation"));
	}
	else if (dynamic_cast<dcLink *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : controlLinks)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		controlLinks.push_back(static_cast<gridLink *>(obj));
		clinkFrac.push_back(obj->get("participation"));
	}
	else
	{
		return;
	}
	if (update)
	{
		updateControls();
	}
}

void dcBusControls::removeControlObject(index_t oid, bool update)
{
	for (size_t kk = 0; kk < controlObjects.size(); ++kk)
	{
		if (oid == controlObjects[kk]->getID())
		{
			controlObjects.erase(controlObjects.begin() + kk);
			cfrac.erase(cfrac.begin() + kk);
			if (update)
			{
				updateControls();
			}
			return;
		}
	}
	for (size_t kk = 0; kk < controlLinks.size(); ++kk)
	{
		if (oid == controlLinks[kk]->getID())
		{
			controlLinks.erase(controlLinks.begin() + kk);
			clinkFrac.erase(clinkFrac.begin() + kk);
			if (update)
			{
				updateControls();
			}
		}
	}
}


void dcBusControls::updateControls()
{
	double pfsum = sum(cfrac) + sum(clinkFrac);
	proxyControlObject.clear();
	gridObject *pco;
	auto pcount = cfrac.size() + clinkFrac.size();
	Pmax = 0;
	Pmin = 0;
	for (size_t kk = 0; kk < cfrac.size(); ++kk)
	{
		pco = controlObjects[kk];
		if ((pfsum > 1.0) && (pcount > 1))
		{
			cfrac[kk] /= pfsum;
			pco->set("participation", cfrac[kk]);
		}
		Pmax += pco->get("pmax");
		Pmin += pco->get("pmin");
		if (pco->checkFlag(remote_power_control))
		{
			if (static_cast<dcBus *> (pco->find("bus"))->directPath(controlledBus, pco))
			{
				auto opath = static_cast<dcBus *> (pco->find("bus"))->getDirectPath(controlledBus, pco);
				opath.pop_back();
				if (dynamic_cast<gridLink *> (opath.back()))
				{
					proxyControlObject.push_back(static_cast<gridLink *> (opath.back()));
				}
			}
			else
			{
				controlledBus->log(controlledBus, print_level::warning, "Generator " + pco->getName() + " on indirect path for power control to bus " + controlledBus->getName());
			}
		}
	}
	for (size_t kk = 0; kk < clinkFrac.size(); ++kk)
	{
		pco = controlLinks[kk];
		if (pfsum != 1.0)
		{
			clinkFrac[kk] /= pfsum;
			pco->set("participation", clinkFrac[kk]);
		}
		Pmax += pco->get("pmax");
		Pmin += pco->get("pmin");
		if (pco->checkFlag(remote_power_control))
		{
			//TODO:: PT what to do in this case?  
		}
	}
	//override bus participation with generator participation
	if ((pcount == 1) && (pfsum != 1.0) && (controlledBus->get("participation") == 1.0))
	{
		controlledBus->set("participation", pfsum);
	}

}



void dcBusControls::mergeBus(dcBus *mbus)
{
	//bus with the lowest ID is the master
	if (controlledBus->getID() < mbus->getID())
	{
		if (controlledBus->checkFlag(dcBus::bus_flags::slave_bus))  //if we are already a slave forward the merge to the master
		{
			masterBus->mergeBus(mbus);
		}
		else
		{
			if (mbus->checkFlag(dcBus::bus_flags::slave_bus))
			{
				if (controlledBus->getID() != mbus->busController.masterBus->getID())
				{
					mergeBus(static_cast<dcBus *>(mbus->busController.masterBus));
				}
			}
			else
			{
				//This bus becomes the master of mbus
				mbus->busController.masterBus = controlledBus;
				mbus->opFlags.set(dcBus::bus_flags::slave_bus);
				slaveBusses.push_back(mbus);
				for (auto sb : mbus->busController.slaveBusses)
				{
					slaveBusses.push_back(sb);
					sb->busController.masterBus = controlledBus;
				}
				mbus->busController.slaveBusses.clear();
			}
		}
	}
	else if (controlledBus->getID() > mbus->getID()) // mbus is now this buses master
	{
		if (controlledBus->checkFlag(dcBus::bus_flags::slave_bus))  //if we are already a slave forward the merge to the master
		{
			if (masterBus->getID() != mbus->getID())
			{
				masterBus->mergeBus(mbus);
			}
		}
		else //we were a master now mbus is the master
		{
			if (slaveBusses.empty()) //no slave buses
			{
				masterBus = mbus;
				mbus->busController.slaveBusses.push_back(controlledBus);
			}
			else
			{
				if (mbus->checkFlag(dcBus::bus_flags::slave_bus))
				{
					mbus->busController.masterBus->mergeBus(controlledBus);
				}
				else
				{
					masterBus = mbus;
					mbus->busController.slaveBusses.push_back(controlledBus);
					for (auto sb : slaveBusses)
					{
						mbus->busController.slaveBusses.push_back(sb);
						sb->busController.masterBus = mbus;
					}
					slaveBusses.clear();
				}
			}
		}


	}
}


void dcBusControls::unmergeBus(dcBus *mbus)
{
	if (controlledBus->checkFlag(dcBus::bus_flags::slave_bus))
	{
		if (mbus->checkFlag(dcBus::bus_flags::slave_bus))
		{
			if (isSameObject(mbus->busController.masterBus,masterBus))
			{
				masterBus->unmergeBus(mbus);
			}
		}
		else if (isSameObject(masterBus,mbus))
		{
			mbus->unmergeBus(controlledBus); //flip it around so this bus is unmerged from mbus
		}
	}
	else//in the masterbus
	{
		if ((mbus->checkFlag(dcBus::bus_flags::slave_bus)) && (isSameObject(controlledBus,mbus->busController.masterBus)))
		{
			for (auto &eb : slaveBusses)
			{
				eb->opFlags.reset(dcBus::bus_flags::slave_bus);
			}
			checkMerge();
			mbus->checkMerge();
		}
	}
}

void dcBusControls::checkMerge()
{
	if (!controlledBus->isEnabled())
	{
		return;
	}
	if (controlledBus->checkFlag(dcBus::bus_flags::directconnect))
	{
		directBus->mergeBus(controlledBus);
	}
	for (auto &lnk : controlledBus->attachedLinks)
	{
		lnk->checkMerge();
	}
}