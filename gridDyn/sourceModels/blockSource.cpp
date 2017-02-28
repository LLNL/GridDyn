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

#include "sourceModels/otherSources.h"
#include "core/coreObjectTemplates.h"
#include "submodels/gridControlBlocks.h"
#include "core/coreExceptions.h"

#include <algorithm>

blockSource::blockSource(const std::string &objName):gridSource(objName)
{

	}

	coreObject * blockSource::clone(coreObject *obj) const 
	{
		blockSource *blkSrc = cloneBase<blockSource, gridSource>(this, obj);
	
		blkSrc->maxStepSize = maxStepSize;
		return blkSrc;
	}

	void blockSource::add(coreObject *obj)
	{
		if (dynamic_cast<basicBlock *>(obj))
		{
			if (blk)
			{
				gridObject::remove(blk);
			}
			blk = static_cast<basicBlock *>(obj);
			addSubObject(blk);
		}
		if (dynamic_cast<gridSource *>(obj))
		{
			if (src)
			{
				gridObject::remove(src);
			}
			src = static_cast<gridSource *>(obj);
			addSubObject(src);
		}
		else
		{
			coreObject::add(obj); //just pass it to the core to do the appropriate thing(probably throw an exception)
		}
	}

	void blockSource::remove(coreObject *obj)
	{
		if (isSameObject(src, obj))
		{
			gridObject::remove(obj);
			src = nullptr;
			return;
		}

		if (isSameObject(blk, obj))
		{
			gridObject::remove(obj);
			blk = nullptr;
			return;
		}
	}

	
	 void blockSource::dynObjectInitializeA(coreTime time0, unsigned long flags)
	 {
		 if (blk)
		 {
			 blk->dynInitializeA(time0, flags);
		 }
		 if (src)
		 {
			 src->dynInitializeA(time0, flags);
		 }

	 }

	 void blockSource::dynObjectInitializeB(const IOdata & /*inputs*/, const IOdata &desiredOutput, IOdata &fieldSet)
	 {
		 if (desiredOutput.empty())
		 {
			 if (src)
			 {
				 src->dynInitializeB(noInputs, noInputs, fieldSet);
			 }
			 if (blk)
			 {
				 blk->dynInitializeB(fieldSet, noInputs, fieldSet);
			 }
			 m_output = fieldSet[0];
		 }
		 else
		 {
			 m_output = desiredOutput[0];
			 if (blk)
			 {
				 blk->dynInitializeB(noInputs, desiredOutput, fieldSet);
			 }
			 if (src)
			 {
				 src->dynInitializeB(noInputs, fieldSet, fieldSet);
			 }
		 }
		 if (maxStepSize > kBigNum / 2.0)
		 {
			 if (blk)
			 {
				 maxStepSize = blk->get("maxstepsize");
			 }
		 }
	 }

	 void blockSource::setFlag(const std::string &flag, bool val) 
	 {
		 if (subObjectSet(flag,val))
		 {
			 return;
		 }
		 else
		 {
			 try
			 {
				 gridSource::setFlag(flag, val);
			 }
			 catch (const unrecognizedParameter &)
			 {
				 if (src)
				 {
					 src->set(flag, val);
				 }
			 }
		 }
	 }

	 void blockSource::set(const std::string &param, const std::string &val) 
	 {
		 if (subObjectSet(param, val))
		 {
			 return;
		 }
		 else
		 {
			 try
			 {
				 gridSource::set(param, val);
			 }
			 catch (const unrecognizedParameter &)
			 {
				 if (src)
				 {
					 src->set(param, val);
				 }
			 }
		 }
	 }
	 void blockSource::set(const std::string &param, double val, gridUnits::units_t unitType)
	 {
		 if (subObjectSet(param, val,unitType))
		 {
			 return;
		 }
		 else if (param == "maxstepsize")
		 {
			 maxStepSize = val;
		 }
		 else
		 {
			 try
			 {
				 gridSource::set(param, val, unitType);
			 }
			 catch(const unrecognizedParameter & )
			 {
				 if (src)
				 {
					 src->set(param, val, unitType);
				 }
			 }
		 }
	 }
	 double blockSource::get(const std::string &param, gridUnits::units_t unitType) const 
	 {
		 double rval = gridSource::get(param, unitType);
		 if (rval == kNullVal)
		 {
			 if (src)
			 {
				 return src->get(param, unitType);
			 }
		 }
		 return rval;
	 }

	// void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

	 void blockSource::residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) 
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 if (src)
		 {
			 src->residual(inputs, sD, resid, sMode);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
		 }
		 if (blk)
		 {
			 
			 blk->residElements(srcOut, srcDout, sD, resid, sMode);
		 }
		 
	 }

	 void blockSource::derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 if (src)
		 {
			 src->derivative(inputs, sD, deriv, sMode);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
		 }
		 if (blk)
		 {

			 blk->derivElements(srcOut, srcDout, sD, deriv, sMode);
		 }
	 }

	 void blockSource::algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) 
	 {
		 double srcOut = m_output;
		 if (src)
		 {
			 src->algebraicUpdate(inputs, sD, update, sMode,alpha);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
		 }
		 if (blk)
		 {

			 blk->algElements(srcOut,  sD, update, sMode);
		 }
	 }

	 void blockSource::jacobianElements(const IOdata &inputs, const stateData &sD,
		matrixData<double> &ad,
		const IOlocs &inputLocs, const solverMode &sMode)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 index_t srcLoc = kNullLocation;
		 if (src)
		 {
			 src->jacobianElements(inputs, sD, ad, inputLocs,sMode);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
			 srcLoc = src->getOutputLoc(sMode, 0);
		 }
		 if (blk)
		 {

			 blk->jacElements(srcOut, srcDout, sD, ad, srcLoc, sMode);
		 }
	 }

	 void blockSource::timestep(coreTime ttime, const IOdata &inputs, const solverMode &sMode) 
	 {
		
		 while (prevTime < ttime)
		 {
			 auto ntime = std::min(prevTime + maxStepSize, ttime);
			 double srcOut = m_output;
			 if (src)
			 {
				 src->timestep(ntime, inputs, sMode);
				 srcOut = getOutput(0);
			 }
			 if (blk)
			 {
				 blk->step(ntime, srcOut);
			 }
		 }
	 }

	 void blockSource::rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 if (src)
		 {
			 src->rootTest(inputs, sD, roots, sMode);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
		 }
		 if (blk)
		 {

			 blk->rootTest({ srcOut,srcDout }, sD, roots, sMode);
		 }
	 }
	 void blockSource::rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 if (src)
		 {
			 src->rootTrigger(ttime,inputs,rootMask, sMode);
			 srcOut = src->getOutput(0);
			 srcDout = src->getDoutdt(noInputs,emptyStateData,sMode,0);
		 }
		 if (blk)
		 {

			 blk->rootTrigger(ttime,{ srcOut,srcDout }, rootMask, sMode);
		 }
	 }

	 change_code blockSource::rootCheck(const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 change_code ret = change_code::no_change;
		 if (src)
		 {
			 auto iret=src->rootCheck(inputs, sD, sMode,level);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
			 ret = std::max(iret, ret);
		 }
		 if (blk)
		 {

			 auto iret=blk->rootCheck({ srcOut,srcDout }, sD, sMode,level);
			 ret = std::max(iret, ret);
		 }
		 return ret;
	 }


	 void blockSource::updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
	 {
		 double srcOut = m_output;
		 double srcDout = 0.0;
		 if (src)
		 {
			 src->updateLocalCache(inputs, sD, sMode);
			 srcOut = src->getOutput(inputs, sD, sMode, 0);
			 srcDout = src->getDoutdt(inputs,sD, sMode, 0);
		 }
		 if (blk)
		 {

			 blk->updateLocalCache({ srcOut, srcDout }, sD, sMode);
		 }
	 }

	 
	 /** set the output level
	 @param[in] newLevel the level to set the output at
	 */
	 void blockSource::setLevel(double newLevel)
	 {
		 if (src)
		 {
			 src->setLevel(newLevel);
		 }
	 }

	 IOdata blockSource::getOutputs(const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
	 {
		 if (blk)
		 {
			 return blk->getOutputs(noInputs, sD, sMode);
		 }
		 else if (src)
		 {
			 return src->getOutputs(noInputs, sD, sMode);
		 }
		 else
		 {
			 return gridSource::getOutputs(noInputs, sD, sMode);
		 }
	 }

	 double blockSource::getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num) const
	 {
		 if (blk)
		 {
			 return blk->getOutput(noInputs, sD, sMode,num);
		 }
		 else if (src)
		 {
			 return src->getOutput(inputs, sD, sMode,num);
		 }
		 else
		 {
			 return gridSource::getOutput(inputs, sD, sMode,num);
		 }
	 }

	 double blockSource::getOutput(index_t num) const
	 {
		 if (blk)
		 {
			 return blk->getOutput(num);
		 }
		 else if (src)
		 {
			 return src->getOutput(num);
		 }
		 else
		 {
			 return gridSource::getOutput(num);
		 }
	 }

	 double blockSource::getDoutdt(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num) const
	 {
		 if (blk)
		 {
			 return blk->getDoutdt(noInputs,sD, sMode,num);
		 }
		 else if (src)
		 {
			 return src->getDoutdt(inputs,sD,sMode,num);
		 }
		 else
		 {
			 return gridSource::getDoutdt(inputs,sD,sMode,num);
		 }
	 }

	 coreObject* blockSource::find(const std::string &object) const
	 {
		 if (object == "source")
		 {
			 return src;
		 }
		 else if (object == "block")
		 {
			 return blk;
		 }
		 else
		 {
			 return gridObject::find(object);
		 }
	 }


	 coreObject* blockSource::getSubObject(const std::string & typeName, index_t num) const
	 {
		 if (typeName == "source")
		 {
			 return (num == 0) ? src : nullptr;
			 
		 }
		 else if (typeName == "block")
		 {
			 return (num == 0) ? blk : nullptr;
		 }
		 else
		 {
			 return gridObject::getSubObject(typeName,num);
		 }
	 }

	 
