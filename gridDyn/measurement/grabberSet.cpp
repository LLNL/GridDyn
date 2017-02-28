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

#include "grabberSet.h"
#include "gridGrabbers.h"
#include "stateGrabber.h"
#include "gridObjects.h"
#include "valuePredictor.h"

grabberSet::grabberSet(const std::string &fld, coreObject *obj, bool step_only)
{
	auto ggb = makeGrabbers(fld, obj);
	if (ggb.size() > 0)
	{
		grab = std::move(ggb[0]);
	}
	else
	{
		grab = nullptr; //TODO:: make this into a default grabber so it grabs something
	}
	if (!step_only)
	{
		auto ggbst = makeStateGrabbers(fld, obj);
		if (ggbst.size() > 0)
		{
			stGrab = std::move(ggbst[0]);
		}
	}
	
}

grabberSet::grabberSet(index_t noffset, coreObject *obj)
{
	grab = createGrabber(noffset, obj);

	stGrab = std::make_shared<stateGrabber>(noffset, obj);

}

grabberSet::grabberSet(std::shared_ptr<gridGrabber> ggrab, std::shared_ptr<stateGrabber> stgrab):grab(ggrab),stGrab(stgrab)
{

}

grabberSet::~grabberSet() = default;

/** clone function
*@param[in] ggb a pointer to another gridGrabber function if we are cloning on existing object
*@return a shared_ptr to another GridGrabber*/
std::shared_ptr<grabberSet> grabberSet::clone(std::shared_ptr<grabberSet> ggb) const
{
	if (ggb == nullptr)
	{
		ggb = std::make_shared<grabberSet>(grab->clone(), (stGrab) ? stGrab->clone():nullptr);
	}
	else
	{
		ggb->updateGrabbers(grab->clone(), (stGrab) ? stGrab->clone() : nullptr);
	}
	if (predictor)
	{
		ggb->predictor = std::make_unique<valuePredictor<coreTime, double>>(*predictor);
	}
	
	return ggb;
}

void grabberSet::updateGrabbers(std::shared_ptr<gridGrabber> ggrab, std::shared_ptr<stateGrabber> stgrab)
{
	grab = ggrab;
	stGrab = stgrab;
}

void grabberSet::updateField(const std::string &fld)
{
	grab->updateField(fld);
	if (stGrab)
	{
		stGrab->updateField(fld);
	}
}
/** actually go and get the data
*@return the value produced by the grabber*/
double grabberSet::grabData()
{

	auto lastOutput = grab->grabData();
	if (predictor)
	{
		predictor->update(lastOutput, grab->getTime());
	}
	return lastOutput;
}
/** @brief grab a vector of data
*@param[out] data the vector to store the data in
*/
void grabberSet::grabData(std::vector<double> &data)
{
	grab->grabData(data);
}

double grabberSet::grabData(const stateData &sD, const solverMode &sMode)
{
	if (stGrab)
	{
		return stGrab->grabData(sD, sMode);
	}
	else
	{
		if (predictor)
		{
			return predictor->predict(sD.time);
		}
		return grab->grabData();
	}
	
}

void grabberSet::outputPartialDerivatives(const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
	if (stGrab)
	{
		stGrab->outputPartialDerivatives(sD, ad, sMode);
	}
	
}
void grabberSet::getDesc(std::vector<std::string > &desc_list) const
{
	grab->getDesc(desc_list);
}
const std::string &grabberSet::getDesc() const
{
	return grab->getDesc();
}
std::string grabberSet::getDesc()
{
	return grab->getDesc();
}
void grabberSet::setDescription(const std::string &newDesc)
{
	grab->setDescription(newDesc);
	
}
void grabberSet::updateObject(coreObject *obj, object_update_mode mode)
{
	grab->updateObject(obj, mode);
	if (stGrab)
	{
		stGrab->updateObject(obj, mode);
	}
	
}

void grabberSet::setGain(double newGain)
{
	grab->gain = newGain;
	if (stGrab)
	{
		stGrab->gain = newGain;
	}
}

coreObject * grabberSet::getObject() const
{
	return grab->getObject();
}

void grabberSet::getObjects(std::vector<coreObject *> &objects) const
{
	grab->getObjects(objects);
	if (stGrab)
	{
		stGrab->getObjects(objects);
	}
	
}

bool grabberSet::stateCapable() const
{
	if (stGrab)
	{
		return (stGrab->loaded);
	}
	return false;
}

bool grabberSet::hasJacobian() const
{
	if (stGrab)
	{
		if (stGrab->getJacobianMode() != jacobian_mode::none)
		{
			return true;
		}
	}
	return false;
}