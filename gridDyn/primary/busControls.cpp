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
#include "acBus.h"
#include "linkModels/gridLink.h"
#include "utilities/vectorOps.hpp"


busControls::busControls(acBus *busToControl):controlledBus(busToControl)
{

}

bool busControls::hasVoltageAdjustments(index_t sid) const
{
	for (auto &adj : vControlObjects)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	for (auto &adj : proxyVControlObject)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	return false;
}

bool busControls::hasPowerAdjustments(index_t sid) const
{
	for (auto &adj : pControlObjects)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	for (auto &adj : proxyPControlObject)
	{
		if (sid == adj->getID())
		{
			return true;
		}
	}
	return false;
}

double busControls::getAdjustableCapacityUp(coreTime time) const
{
	double cap = 0.0;

	for (auto &adj : pControlObjects)
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

double busControls::getAdjustableCapacityDown(coreTime time) const
{
	double cap = 0.0;

	for (auto &adj : pControlObjects)
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

void busControls::addPowerControlObject(gridObject *obj,bool update)
{
	if (dynamic_cast<gridSecondary *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : pControlObjects)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		pControlObjects.push_back(static_cast<gridSecondary *>(obj));
		pcfrac.push_back(obj->get("participation"));
	}
	else if (dynamic_cast<gridLink *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : pControlLinks)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		pControlLinks.push_back(static_cast<gridLink *>(obj));
		pclinkFrac.push_back(obj->get("participation"));
	}
	else
	{
		return;
	}
	if (update)
	{
		updatePowerControls();
	}
}

void busControls::addVoltageControlObject(gridObject *obj, bool update)
{
	if (dynamic_cast<gridSecondary *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : vControlObjects)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		vControlObjects.push_back(static_cast<gridSecondary *>(obj));
		vcfrac.push_back(obj->get("vcontrolfrac"));
	}
	else if (dynamic_cast<gridLink *>(obj))
	{
		auto  objid = obj->getID();
		for (auto &rvc : vControlLinks)
		{
			if (objid == rvc->getID())
			{
				return;
			}
		}
		vControlLinks.push_back(static_cast<gridLink *>(obj));
		vclinkFrac.push_back(obj->get("vcontrolfrac"));
	}
	else
	{
		return;
	}
	if (update)
	{
		updatePowerControls();
	}
}

void busControls::removePowerControlObject(index_t oid, bool update)
{
	for (size_t kk = 0; kk < pControlObjects.size(); ++kk)
	{
		if (oid == pControlObjects[kk]->getID())
		{
			pControlObjects.erase(pControlObjects.begin() + kk);
			pcfrac.erase(pcfrac.begin() + kk);
			if (update)
			{
				updatePowerControls();
			}
			return;
		}
	}
	for (size_t kk = 0; kk < pControlLinks.size(); ++kk)
	{
		if (oid == pControlLinks[kk]->getID())
		{
			pControlLinks.erase(pControlLinks.begin() + kk);
			pclinkFrac.erase(pclinkFrac.begin() + kk);
			if (update)
			{
				updatePowerControls();
			}
		}
	}
}

void busControls::removeVoltageControlObject(index_t oid, bool update)
{
	for (size_t kk = 0; kk < vControlObjects.size(); ++kk)
	{
		if (oid == vControlObjects[kk]->getID())
		{
			vControlObjects.erase(vControlObjects.begin() + kk);
			vcfrac.erase(vcfrac.begin() + kk);
			if (update)
			{
				updateVoltageControls();
			}
			return;
		}
	}
	for (size_t kk = 0; kk < vControlLinks.size(); ++kk)
	{
		if (oid == vControlLinks[kk]->getID())
		{
			vControlLinks.erase(vControlLinks.begin() + kk);
			vclinkFrac.erase(vclinkFrac.begin() + kk);
			if (update)
			{
				updateVoltageControls();
			}
		}
	}
}


void busControls::updateVoltageControls()
{
	double vfsum = sum(vcfrac) + sum(vclinkFrac);
	proxyVControlObject.clear();
	bool non_direct_remote = false;
	gridObject *vco;
	Qmax = 0;
	Qmin = 0;
	for (size_t kk = 0; kk < vcfrac.size(); ++kk)
	{
		vco = vControlObjects[kk];
		if (vfsum != 1.0)
		{
			vcfrac[kk] /= vfsum;
			vco->set("vcontrolfrac", vcfrac[kk]);
		}
		Qmax += vco->get("qmax");
		Qmin += vco->get("qmin");
		if (vco->checkFlag(remote_voltage_control))
		{
			if (static_cast<acBus *> (vco->find("bus"))->directPath(controlledBus, vco))
			{
				auto opath = static_cast<acBus *> (vco->find("bus"))->getDirectPath(controlledBus, vco);
				opath.pop_back();
				if (dynamic_cast<gridLink *> (opath.back()))
				{
					proxyVControlObject.push_back(static_cast<gridLink *> (opath.back()));
				}
			}
			else
			{
				non_direct_remote = true;
			}
		}
	}

	for (size_t kk = 0; kk < vclinkFrac.size(); ++kk)
	{
		vco = vControlLinks[kk];
		if (vfsum != 1.0)
		{
			vclinkFrac[kk] /= vfsum;
			vco->set("vcontrolfrac", vclinkFrac[kk]);
		}
		Qmax += vco->get("qmax");
		Qmin += vco->get("qmin");
		if (vco->checkFlag(remote_voltage_control))
		{
			non_direct_remote = true;
		}
	}

	if (non_direct_remote)
	{
		for (auto &vcobj : vControlObjects)
		{
			vcobj->setFlag("indirect_voltage_control", true);
		}
		for (auto &vcobj : vControlLinks)
		{
			vcobj->setFlag("indirect_voltage_control", true);
		}
		controlledBus->set("type", "pq");
		controlledBus->opFlags.set(indirect_voltage_control);
	}
	//check if the v and p controls are identical
	controlledBus->opFlags.set(acBus::bus_flags::identical_PQ_control_objects, checkIdenticalControls());
	
}

void busControls::updatePowerControls()
{
	double pfsum = sum(pcfrac) + sum(pclinkFrac);
	proxyPControlObject.clear();
	gridObject *pco;
	auto pcount = pcfrac.size()+pclinkFrac.size();
	Pmax = 0;
	Pmin = 0;
	for (size_t kk = 0; kk < pcfrac.size(); ++kk)
	{
		pco = pControlObjects[kk];
		if ((pfsum > 1.0) && (pcount > 1))
		{
			pcfrac[kk] /= pfsum;
			pco->set("participation", pcfrac[kk]);
		}
		Pmax += pco->get("pmax");
		Pmin += pco->get("pmin");
		if (pco->checkFlag(remote_power_control))
		{
			if (static_cast<acBus *> (pco->find("bus"))->directPath(controlledBus, pco))
			{
				auto opath = static_cast<acBus *> (pco->find("bus"))->getDirectPath(controlledBus, pco);
				opath.pop_back();
				if (dynamic_cast<gridLink *> (opath.back()))
				{
					proxyPControlObject.push_back(static_cast<gridLink *> (opath.back()));
				}
			}
			else
			{
				controlledBus->log(controlledBus,print_level::warning,"Generator " + pco->getName() + " on indirect path for power control to bus " + controlledBus->getName());
			}
		}
	}
	for (size_t kk = 0; kk < pclinkFrac.size(); ++kk)
	{
		pco = pControlLinks[kk];
		if (pfsum != 1.0)
		{
			pclinkFrac[kk] /= pfsum;
			pco->set("participation", pclinkFrac[kk]);
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
	//check if the v and p controls are identical
	controlledBus->opFlags.set(acBus::bus_flags::identical_PQ_control_objects, checkIdenticalControls());

}


bool busControls::checkIdenticalControls()
{
	if ((vControlObjects.size() == pControlObjects.size()) && (vControlLinks.size() == pControlLinks.size()))
	{
		for (size_t kk = 0; kk < vControlObjects.size(); ++kk)
		{
			if (vControlObjects[kk]->getID() != pControlObjects[kk]->getID())
			{
				return false;
			}
		}
		for (size_t kk = 0; kk < vControlLinks.size(); ++kk)
		{
			if (vControlLinks[kk]->getID() != pControlLinks[kk]->getID())
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}


void busControls::mergeBus(acBus *mbus)
{
	//bus with the lowest ID is the master
	if (controlledBus->getID() < mbus->getID())
	{
		if (controlledBus->checkFlag(acBus::bus_flags::slave_bus))  //if we are already a slave forward the merge to the master
		{
			masterBus->mergeBus(mbus);
		}
		else
		{
			if (mbus->checkFlag(acBus::bus_flags::slave_bus))
			{
				if (controlledBus->getID() != mbus->busController.masterBus->getID())
				{
					mergeBus(static_cast<acBus *>(mbus->busController.masterBus));
				}
			}
			else
			{
				//This bus becomes the master of mbus
				mbus->busController.masterBus = controlledBus;
				mbus->opFlags.set(acBus::bus_flags::slave_bus);
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
		if (controlledBus->checkFlag(acBus::bus_flags::slave_bus))  //if we are already a slave forward the merge to the master
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
				if (mbus->checkFlag(acBus::bus_flags::slave_bus))
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


void busControls::unmergeBus(acBus *mbus)
{
	if (controlledBus->checkFlag(acBus::bus_flags::slave_bus))
	{
		if (mbus->checkFlag(acBus::bus_flags::slave_bus))
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
		if ((mbus->checkFlag(acBus::bus_flags::slave_bus)) && (isSameObject(controlledBus,mbus->busController.masterBus)))
		{
			for (auto &eb : slaveBusses)
			{
				eb->opFlags.reset(acBus::bus_flags::slave_bus);
			}
			checkMerge();
			mbus->checkMerge();
		}
	}
}

void busControls::checkMerge()
{
	if (!controlledBus->isEnabled())
	{
		return;
	}
	if (controlledBus->checkFlag(acBus::bus_flags::directconnect))
	{
		directBus->mergeBus(controlledBus);
	}
	for (auto &lnk : controlledBus->attachedLinks)
	{
		lnk->checkMerge();
	}
}