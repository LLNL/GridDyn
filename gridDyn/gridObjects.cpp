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

#include "gridObjects.h"
#include "core/coreObjectTemplates.h"
#include "utilities/matrixData.h"
#include "core/coreExceptions.h"
#include "utilities/stackInfo.h"
#include "utilities/stringOps.h"
#include "utilities/vectorOps.hpp"
#include "core/objectInterpreter.h"
#include <cstdio>
#include <iostream>
#include <map>
#include <cassert>



//this functions is here since it depends on gridObject information
Lp offsetTable::getLocations (const stateData &sD, double d[], const solverMode &sMode, const gridObject *obj) const
{
    Lp Loc = getLocations(sD, sMode, obj);
  if ((sMode.local)||(sD.empty()))
    {
      Loc.destLoc = d;
      Loc.destDiffLoc = d + Loc.algSize;
    }
  else if (isDAE (sMode))
    {
      Loc.destLoc = d + Loc.algOffset;
      Loc.destDiffLoc = d + Loc.diffOffset;
    }
  else if (hasAlgebraic(sMode))
    {
      Loc.destLoc = d + Loc.algOffset;
      Loc.destDiffLoc = nullptr;
    }
  else if (hasDifferential (sMode))
    {
     
      Loc.destDiffLoc = d + Loc.diffOffset;
      Loc.destLoc = nullptr;
    }
  else
    {
      Loc.destLoc = d;
      Loc.destDiffLoc = d + Loc.algSize;
    }
  return Loc;
}

//this functions is here since it depends on gridObject information
Lp offsetTable::getLocations (const stateData &sD, const solverMode &sMode, const gridObject *obj) const
{
  Lp Loc;
  Loc.algOffset = offsetContainer[sMode.offsetIndex].algOffset;
  Loc.diffOffset = offsetContainer[sMode.offsetIndex].diffOffset;
  Loc.diffSize = offsetContainer[sMode.offsetIndex].total.diffSize;
  Loc.algSize = offsetContainer[sMode.offsetIndex].total.algSize;
  if ((sMode.local) || (sD.empty()))
    {
      Loc.time = obj->prevTime;
      Loc.algStateLoc = obj->m_state.data ();
      Loc.diffStateLoc = obj->m_state.data () + Loc.algSize;
      Loc.dstateLoc = obj->m_dstate_dt.data () + Loc.algSize;
      if (Loc.algOffset == kNullLocation)
        {
          Loc.algOffset = 0;
        }
      if (Loc.diffOffset == kNullLocation)
        {
          Loc.diffOffset = Loc.algSize;
        }
    }
  else if (isDAE (sMode))
    {
      Loc.time = sD.time;
      Loc.algStateLoc = sD.state + Loc.algOffset;
      Loc.diffStateLoc = sD.state + Loc.diffOffset;
      Loc.dstateLoc = sD.dstate_dt + Loc.diffOffset;
    }
  else if (hasAlgebraic (sMode))
    {
      Loc.time = sD.time;
      if (sD.state)
        {
          Loc.algStateLoc = sD.state + Loc.algOffset;
        }
      else
        {
          Loc.algStateLoc = sD.algState + Loc.algOffset;
        }
      if ((isDynamic(sMode)) && (sD.pairIndex != kNullLocation))
      {
          if (sD.diffState)
          {
              Loc.diffStateLoc = sD.diffState + offsetContainer[sD.pairIndex].diffOffset;
          }
          else if (sD.fullState)
          {
              Loc.diffStateLoc = sD.fullState + offsetContainer[sD.pairIndex].diffOffset;
          }
         
          if (sD.dstate_dt)
          {
              Loc.dstateLoc = sD.dstate_dt + offsetContainer[sD.pairIndex].diffOffset;
          }
      }
      else
      {
          Loc.diffStateLoc = obj->m_state.data() + offsetContainer[0].diffOffset;
          Loc.dstateLoc = obj->m_dstate_dt.data() + offsetContainer[0].diffOffset;
      }
      Loc.destDiffLoc = nullptr;

    }
  else if (hasDifferential (sMode))
    {
      Loc.time = sD.time;
      if (sD.state)
        {
          Loc.diffStateLoc = sD.state + Loc.diffOffset;
        }
      else
        {
          Loc.diffStateLoc = sD.diffState + Loc.diffOffset;
        }
      Loc.dstateLoc = sD.dstate_dt + Loc.diffOffset;
        if (sD.pairIndex != kNullLocation)
        {
            if (sD.algState)
            {
                Loc.algStateLoc = sD.algState + offsetContainer[sD.pairIndex].algOffset;
            }
            else if (sD.fullState)
            {
                Loc.algStateLoc = sD.fullState + offsetContainer[sD.pairIndex].algOffset;
            }
        }
      else
        {
          Loc.algStateLoc = obj->m_state.data () + offsetContainer[0].algOffset;
        }
      Loc.destLoc = nullptr;
    }
  else
    {
      Loc.time = obj->prevTime;
      Loc.algStateLoc = obj->m_state.data ();
      Loc.diffStateLoc = obj->m_state.data () + Loc.algSize;
      Loc.dstateLoc = obj->m_dstate_dt.data () + Loc.algSize;
      if (Loc.algOffset == kNullLocation)
        {
          Loc.algOffset = 0;
        }
      if (Loc.diffOffset == kNullLocation)
        {
          Loc.diffOffset = Loc.algSize;
        }
    }
  return Loc;
}

gridObject::gridObject (const std::string &objName) : coreObject (objName)
{
  offsets.setAlgOffset (0,cLocalSolverMode);
}

gridObject::~gridObject ()
{
  for (auto &so : subObjectList)
    {
      removeReference (so, this);
    }
}

coreObject *gridObject::clone (coreObject *obj) const
{
  gridObject *nobj = cloneBase<gridObject, coreObject> (this, obj);
  if (!(nobj))
    {
      return obj;
    }
  nobj->m_inputSize = m_inputSize;
  nobj->m_outputSize = m_outputSize;
  nobj->opFlags = opFlags;
  nobj->m_baseFreq = m_baseFreq;
  nobj->systemBasePower = systemBasePower;

  if (nobj->subObjectList.empty())
  {
	  for (const auto &so : subObjectList)
	  {
		  try
		  {
			  nobj->add(so->clone());
		  }
		  catch (const unrecognizedObjectException &)
		  {
			  //this likely means that the parent will take care of it itself
		  }
	  }
  }
  else
  {
	  auto csz = nobj->subObjectList.size();
	  //clone the subObjects
	  for (size_t ii = 0; ii < subObjectList.size(); ++ii)
	  {

		  if (subObjectList[ii]->locIndex != kNullLocation)
		  {
			  bool fnd = false;
			  for (size_t kk=0;kk<csz;++kk)
			  {
				  if (nobj->subObjectList[kk]->locIndex == subObjectList[ii]->locIndex)
				  {
					  if (typeid(nobj->subObjectList[kk]) == typeid(subObjectList[ii])) //make sure the types are same before cloning
					  {
						  subObjectList[ii]->clone(nobj->subObjectList[kk]);
						  fnd = true;
						  break;
					  }
				  }
			  }
			  if (!fnd)
			  {
				  try
				  {
					  nobj->add(subObjectList[ii]->clone());
				  }
				  catch (const unrecognizedObjectException &)
				  {
					  //this likely means that the parent will take care of it itself
				  }
			  }
		  }
		  else
		  {
			  if (ii >= csz)
			  {
				  try
				  {
					nobj->add(subObjectList[ii]->clone());
				  }
				  catch (const unrecognizedObjectException &)
				  {
					  //this likely means that the parent will take care of it itself
				  }
			  }
			  else
			  {
				  if (typeid(subObjectList[ii]) == typeid(nobj->subObjectList[ii]))
				  {
					  subObjectList[ii]->clone(nobj->subObjectList[ii]);
				  }
				  else
				  {
					  try
					  {
						nobj->add(subObjectList[ii]->clone());
					  }
					  catch (const unrecognizedObjectException &)
					  {
						  //this likely means that the parent will take care of it itself
					  }
				  }

			  }
		  }

	  }
  }
 
  return nobj;
}


void gridObject::updateObjectLinkages(coreObject *newRoot)
{
	for (auto &subobj : getSubObjects())
	{
		subobj->updateObjectLinkages(newRoot);
	}
}


void gridObject::pFlowInitializeA(coreTime time0, unsigned long flags)
{
	if (isEnabled())
	{
		pFlowObjectInitializeA(time0, flags);
		prevTime = time0;
		updateFlags(false);
		setupPFlowFlags();
	}

}

void gridObject::pFlowInitializeB()
{
	if (isEnabled())
	{
		pFlowObjectInitializeB();
		opFlags.set(pFlow_initialized);
	}
}

void gridObject::dynInitializeA(coreTime time0, unsigned long flags)
{
	if (isEnabled())
	{
		dynObjectInitializeA(time0, flags);
		prevTime = time0;
		updateFlags(true);
		setupDynFlags();
	}
}

void gridObject::dynInitializeB(const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
	if (isEnabled())
	{
		dynObjectInitializeB(inputs, desiredOutput, fieldSet);
		if (updatePeriod < maxTime)
		{
			opFlags.set(has_updates);
			nextUpdateTime = prevTime + updatePeriod;
			alert(this, UPDATE_REQUIRED);
		}
		opFlags.set(dyn_initialized);
	}
}


void gridObject::pFlowObjectInitializeA(coreTime time0, unsigned long flags)
{
	for (auto &subobj : subObjectList)
	{
		subobj->pFlowInitializeA(time0, flags);	
	}

}

void gridObject::pFlowObjectInitializeB()
{
	for (auto &subobj : subObjectList)
		{
			subobj->pFlowInitializeB();
		}
}

void gridObject::dynObjectInitializeA(coreTime time0, unsigned long flags)
{
		for (auto &subobj : subObjectList)
		{
			subobj->dynInitializeA(time0, flags);
		}
}

void gridObject::dynObjectInitializeB(const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{

		for (auto &subobj : subObjectList)
		{		
			subobj->dynInitializeB(inputs, desiredOutput, fieldSet);
		}

}

count_t gridObject::stateSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  count_t ssize = (hasAlgebraic(sMode)) ? (so->total.algSize + so->total.vSize + so->total.aSize) : 0;
  if (hasDifferential(sMode))
  {
      ssize+= so->total.diffSize;
  }
  return ssize;

}

count_t gridObject::stateSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  if (!so)
    {
      return 0;
    }
  count_t ssize = (hasAlgebraic(sMode)) ? (so->total.algSize + so->total.vSize + so->total.aSize) : 0;
  if (hasDifferential(sMode))
  {
      ssize += so->total.diffSize;
  }
  return ssize;

}

count_t gridObject::totalAlgSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode, false);
    }
  return so->total.algSize + so->total.vSize + so->total.aSize;
}

count_t gridObject::totalAlgSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? (so->total.algSize + so->total.vSize + so->total.aSize) : 0;
}

count_t gridObject::algSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  return so->total.algSize;
}

count_t gridObject::algSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? so->total.algSize : 0;
}

count_t gridObject::diffSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  return so->total.diffSize;
}

count_t gridObject::diffSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? so->total.diffSize : 0;
}

count_t gridObject::rootSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->rjLoaded))
    {
      loadSizes (sMode,true);
    }
  return so->total.algRoots + so->total.diffRoots;
}

count_t gridObject::rootSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);

  return (so) ? so->total.algRoots + so->total.diffRoots : 0;
}

count_t gridObject::jacSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->rjLoaded))
    {
      loadSizes (sMode,true);
    }
  return so->total.jacSize;
}

count_t gridObject::jacSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? so->total.jacSize : 0;
}

count_t gridObject::voltageStateCount (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  return so->total.vSize;
}

count_t gridObject::voltageStateCount (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? so->total.vSize : 0;
}

count_t gridObject::angleStateCount (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  return so->total.aSize;
}

count_t gridObject::angleStateCount (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  return (so) ? so->total.aSize : 0;
}

count_t gridObject::paramSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode, false);
    }
  return 0;
}

const solverOffsets *gridObject::getOffsets(const solverMode &sMode) const
{
    return offsets.getOffsets(sMode);
}

count_t gridObject::paramSize (const solverMode &) const
{
  return 0;
}


void gridObject::setOffsets (const solverOffsets &newOffsets, const solverMode &sMode)
{

  offsets.setOffsets (newOffsets,sMode);

  if (!subObjectList.empty ())
    {
      solverOffsets no (newOffsets);
      no.localIncrement (offsets.getOffsets (sMode));
      for (auto &so : subObjectList)
        {
            if (so->isEnabled())
            {
                so->setOffsets(no, sMode);
                no.increment(so->offsets.getOffsets(sMode));
            }
          
        }
    }
}

void gridObject::setOffset (index_t newOffset, const solverMode &sMode)
{
  if (!subObjectList.empty ())
    {
      for (auto &so : subObjectList)
        {
            if (so->isEnabled())
            {
                so->setOffset(newOffset, sMode);
                newOffset += so->stateSize(sMode);
            }
         
        }
    }
  offsets.setOffset (newOffset, sMode);
}

bool gridObject::isLoaded (const solverMode &sMode,bool dynOnly) const
{
  return (dynOnly) ? offsets.isrjLoaded (sMode) : offsets.isLoaded (sMode);
}

/* *INDENT-OFF* */
static const std::map<std::string, operation_flags> user_settable_flags
{
    { "use_bus_frequency", uses_bus_frequency },
    { "late_b_initialize", late_b_initialize },
    {"error",error_flag},
    {"no_gridobject_set",no_gridobject_set },
    { "disable_flag_update", disable_flag_updates },
    { "flag_update_required", flag_update_required },
    { "pflow_init_required", pflow_init_required },
    {"sampled_only",no_dyn_states},
};


//there isn't that many flags that we want to be user settable, most are controlled by the model so allowing them to be set by an external function
// might not be the best thing
void gridObject::setFlag (const std::string &flag, bool val)
{
    auto ffind = user_settable_flags.find(flag);
    if (ffind != user_settable_flags.end())
    {
        opFlags.set(ffind->second,val);
		if (flag == "sampled_only")
		{
			if (opFlags[pFlow_initialized])
			{
				offsets.unload();
			}
		}
    }
    else if (flag == "connected")
    {
        if (val)
        {
            if (!isConnected())
            {
                reconnect();
            }
        }
        else if (isConnected())
        {
            disconnect();
        }
    }
    else if (flag == "disconnected")
    {
        if (val)
        {
            if (isConnected())
            {
                disconnect();
            }
        }
        else if (!isConnected())
        {
            reconnect();
        }
    }
    else if (subObjectSet(flag, val))
    {
        return;
    }
    else
    {
        coreObject::setFlag(flag, val);
    }
}

static const std::vector<index_t> parentSettableFlags{ sampled_only, no_gridobject_set, separate_processing };

void gridObject::parentSetFlag(index_t flagID, bool val, coreObject *checkParent)
{
	if (isSameObject(getParent(),checkParent))
	{
		if (std::binary_search(parentSettableFlags.begin(), parentSettableFlags.end(), flagID))
		{
			opFlags[flagID] = val;
		}
	}
}

/* *INDENT-OFF* */
static const std::map<std::string, operation_flags> flagmap
{
  {"updates",has_updates},
  { "constraints", has_constraints},
  {"roots", has_roots},
  {"alg_roots", has_alg_roots},
  { "voltage_adjustments", has_powerflow_adjustments},
  {"preex", preEx_requested},
  {"use_bus_frequency", uses_bus_frequency},
  {"pflow_states", has_pflow_states},
  {"dyn_states", has_dyn_states},
  {"differential_states", has_differential_states},
  {"not_cloneable", not_cloneable},
  {"remote_voltage_control", remote_voltage_control},
  {"local_voltage_control", local_voltage_control},
  {"indirect_voltage_control", indirect_voltage_control},
  { "remote_power_control", remote_voltage_control },
  { "local_power_control", local_power_control },
  { "indirect_power_control", indirect_power_control },
  {"pflow_initialized", pFlow_initialized},
  {"dyn_initialized", dyn_initialized},
  {"armed", object_armed_flag},
  {"late_b_initialize", late_b_initialize},
  {"object_flag1", object_flag1},
  {"object_flag2", object_flag2},
  {"object_flag3", object_flag3},
  {"object_flag4", object_flag4},
  {"object_flag5", object_flag5},
  {"object_flag6", object_flag6},
  {"object_flag7", object_flag7},
  {"object_flag8", object_flag8},
  {"object_flag9", object_flag9},
  {"object_flag10", object_flag10},
  {"object_flag11", object_flag11 },
  { "object_flag12", object_flag12},
  {"state_change", state_change_flag},
  {"object_change", object_change_flag},
  {"constraint_change", constraint_change_flag},
  {"root_change", root_change_flag},
  {"jacobian_count_change", jacobian_count_change_flag},
  {"slack_bus_change", slack_bus_change},
  {"voltage_control_change", voltage_control_change },
  {"error", error_flag},
  {"connectivity_change", connectivity_change_flag},
  {"no_pflow_states", no_pflow_states},
  {"disconnected",disconnected},
  {"no_dyn_states", no_dyn_states},
  {"sampled_only", no_dyn_states},
  {"disable_flag_update", disable_flag_updates},
  {"flag_update_required", flag_update_required},
  { "differential_output", differential_output },
  {"multipart_calculation_capable", multipart_calculation_capable},
  {"pflow_init_required", pflow_init_required},
  {"dc_only", dc_only},
  {"dc_capable", dc_capable},
  {"dc_terminal2", dc_terminal2},
  {"separate_processing",separate_processing},
  {"three_phase_only", three_phase_only},
  {"three_phase_capable", three_phase_capable},
  {"three_phase_terminal2", three_phase_terminal2}
};

/* *INDENT-ON* */

bool gridObject::getFlag (const std::string &param) const
{
  auto ffind = flagmap.find (param);
  if (ffind != flagmap.end ())
    {
      return opFlags[ffind->second];
    }
  else
    {
	  return coreObject::getFlag(param);
    }
}

bool gridObject::checkFlag (index_t flagID) const
{
  return opFlags.test(flagID);
}

bool gridObject::hasStates(const solverMode &sMode) const
{
    return (stateSize(sMode) > 0);
}

bool gridObject::isArmed () const
{
  return opFlags[object_armed_flag];
}

bool gridObject::isConnected () const
{
  return !(opFlags[disconnected]);
}

void gridObject::reconnect ()
{
  opFlags.set (disconnected,false);
}

void gridObject::disconnect ()
{
  opFlags.set (disconnected);
}

static const stringVec locNumStrings {
  "status", "basefrequency","basepower"
};
static const stringVec locStrStrings {
  "status"
};

void gridObject::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridObject,coreObject> (this,pstr,locNumStrings,locStrStrings,{},pstype);
}

void gridObject::set (const std::string &param,  const std::string &val)
{
  if (opFlags[no_gridobject_set])
    {
      throw(unrecognizedParameter());
    }

  if (param == "status")
    {
      auto v2 = convertToLowerCase (val);
      if ((v2 == "on") || (v2 == "in")||(v2 == "enabled"))
        {
          if (!isEnabled())
            {
              enable ();
              if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
                {
                  alert (this, STATE_COUNT_CHANGE);
                }
            }
        }
      else if ((v2 == "off") || (v2 == "out")||(v2 == "disabled"))
        {
          if (isEnabled())
            {

              if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
                {
                  alert (this, STATE_COUNT_CHANGE);
                }
              disable ();
            }
        }
      else if (v2 == "connected")
      {
          if (!isConnected())
          {
              reconnect();
          }
      }
      else if (v2 == "disconnected")
      {
          if (isConnected())
          {
              disconnect();
          }
      }
    }
  else if (param == "flags")
    {
      setMultipleFlags(this, val);
    }
  else if (subObjectSet(param, val))
  {
      return;
  }
  else
    {
      coreObject::set (param, val);
    }
  
}

auto hasParameterPath(const std::string &param)
{
    return (param.find_last_of(":?") != std::string::npos);
}

bool gridObject::subObjectSet(const std::string &param, double val, gridUnits::units_t unitType)
{
    if (hasParameterPath(param))
    {
        objInfo pinfo(param, this);
        if (pinfo.m_obj)
        {
            if (pinfo.m_unitType != gridUnits::units_t::defUnit)
            {
                pinfo.m_obj->set(pinfo.m_field, val, pinfo.m_unitType);
            }
            else
            {
                pinfo.m_obj->set(pinfo.m_field, val, unitType);
            }
            return true;
        }
        else
        {
            throw(unrecognizedParameter());
        }
    }
    else
    {
        return false;
    }
}

bool gridObject::subObjectSet(const std::string &param, const std::string &val)
{
    if (hasParameterPath(param))
    {
        objInfo pinfo(param, this);
        if (pinfo.m_obj)
        {
            pinfo.m_obj->set(pinfo.m_field, val);
        }
        else
        {
            throw(unrecognizedParameter());
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool gridObject::subObjectSet(const std::string &flag, bool val)
{
    if (hasParameterPath(flag))
    {
        objInfo pinfo(flag, this);
        if (pinfo.m_obj)
        {
            pinfo.m_obj->setFlag(pinfo.m_field, val);
        }
        else
        {
            throw(unrecognizedParameter());
        }
        return true;
    }
    else
    {
        return false;
    }
}

double gridObject::subObjectGet(const std::string &param, gridUnits::units_t unitType) const
{
    if (hasParameterPath(param))
    {
        objInfo pinfo(param, this);
        if (pinfo.m_obj)
        {
            if (pinfo.m_unitType != gridUnits::units_t::defUnit)
            {
                return pinfo.m_obj->get(pinfo.m_field, pinfo.m_unitType);
            }
            else
            {
                return pinfo.m_obj->get(pinfo.m_field, unitType);
            }
        }
        else
        {
            throw(unrecognizedParameter());
        }
    }
    else
    {
        return kNullVal;
    }
}

void gridObject::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (opFlags[no_gridobject_set])
    {
        throw(unrecognizedParameter());
    }

  if ((param == "enabled")||(param == "status"))
    {
      if (val > 0.1)
        {
          if (!isEnabled())
            {
              enable ();
              if (opFlags[has_dyn_states])
                {
                  alert (this, STATE_COUNT_CHANGE);
                }
            }
        }
      else
        {
          if (isEnabled())
            {

              if (opFlags[has_dyn_states])
                {
                  alert (this, STATE_COUNT_CHANGE);
                }
              disable ();
            }
        }
    }
  else if (param == "connected")
    {
      if (val > 0.1)
        {
          if (!isConnected ())
            {
              reconnect ();
            }
        }
      else
        {
          if (isConnected ())
            {
              disconnect ();
            }
        }
    }
  else if (param == "basepower")
    {
      systemBasePower = gridUnits::unitConversion (val, unitType,gridUnits::MW);
      setAll("all", "basepower", systemBasePower);
    }
  else if ((param == "basefreq") || (param == "basefrequency"))
    {
      m_baseFreq = gridUnits::unitConversionFreq (val, unitType, gridUnits::rps);
      setAll("all", "basefreq", systemBasePower);
    }
  else if (subObjectSet(param, val, unitType))
  {
      return;
  }
  else
    {
      coreObject::set (param, val, unitType);
    }

}

void gridObject::setAll(const std::string & type, std::string param, double val, gridUnits::units_t unitType)
{
    if ((type == "all") || (type == "sub")||(type=="object"))
    {
        for (auto &so : subObjectList)
        {
            so->set(param, val, unitType);
        }
    }
}

double gridObject::get(const std::string &param, gridUnits::units_t unitType) const
{
    
    double out = kNullVal;
    if (param=="basefrequency")
    {
        out = gridUnits::unitConversionFreq(m_baseFreq,gridUnits::rps,unitType);
    }
    else if (param=="basepower")
    {
        out = gridUnits::unitConversion(systemBasePower, gridUnits::MVAR, unitType,systemBasePower);
    }
	else if (param == "subobjectcount")
	{
		out = static_cast<double>(subObjectList.size());
	}
    else
    {
        out = subObjectGet(param, unitType);
        if (out == kNullVal)
        {
            out = coreObject::get(param, unitType);
        }
        
    }
    return out;
}


void gridObject::addSubObject(gridObject *obj)
{
    if (!obj)
    {
        return;
    }
    for (auto &sobj : subObjectList)
    {
        if (isSameObject(sobj,obj))
        {
            return;
        }
    }
    obj->setParent(this);
    obj->addOwningReference();
    obj->m_baseFreq = m_baseFreq;
    obj->systemBasePower = systemBasePower;
    subObjectList.push_back(obj);
}

void gridObject::remove(coreObject *obj)
{
	if (!subObjectList.empty())
	{
		auto rmobj = std::find_if(subObjectList.begin(), subObjectList.end(), [obj](coreObject *so) {return isSameObject(so,obj); });
		if (rmobj != subObjectList.end())
		{
			removeReference(*rmobj, this);
			subObjectList.erase(rmobj);
		}
	}
}


void gridObject::reset(reset_levels level)
{
	for (auto &subobj : subObjectList)
	{
		subobj->reset(level);
	}
}

change_code gridObject::powerFlowAdjust(const IOdata & inputs, unsigned long flags, check_level_t level)
{
	auto ret = change_code::no_change;

		for (auto &subobj : subObjectList)
		{
			if (!(subobj->checkFlag(has_powerflow_adjustments)))
			{
				continue;
			}
			auto iret = subobj->powerFlowAdjust(inputs, flags, level);
			if (iret > ret)
			{
				ret = iret;
			}
		}
	return ret;
}

void gridObject::setState (coreTime ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  prevTime = ttime;
  if (!hasStates(sMode)) //use the const version of stateSize
  {
      return;
  }
  const auto so = offsets.getOffsets(sMode);
  const auto &localStates = (subObjectList.empty()) ? (so->total) : (so->local);

  if (hasAlgebraic(sMode))
  {
      if (localStates.algSize > 0)
      {
          std::copy(state + so->algOffset, state + so->algOffset + localStates.algSize, m_state.data());
      }
  }
  if (localStates.diffSize > 0)
  {
      if (isDifferentialOnly(sMode))
      {
          std::copy(state + so->diffOffset, state + so->diffOffset + localStates.diffSize, m_state.data() +algSize(cLocalSolverMode));
          std::copy(dstate_dt + so->diffOffset, dstate_dt + so->diffOffset + localStates.diffSize, m_dstate_dt.data() + algSize(cLocalSolverMode));
      }
      else
      {
          std::copy(state + so->diffOffset, state + so->diffOffset + localStates.diffSize, m_state.data() + localStates.algSize);
          std::copy(dstate_dt + so->diffOffset, dstate_dt + so->diffOffset + localStates.diffSize, m_dstate_dt.data() + localStates.algSize);
      }
  }

      for (auto &sub : subObjectList)
      {
          sub->setState(ttime, state, dstate_dt, sMode);
      }

}
//for saving the state
void gridObject::guess(coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
    if (!hasStates(sMode))
    {
        return;
    }
    const auto so = offsets.getOffsets(sMode);
    const auto &localStates = (subObjectList.empty()) ? (so->total) : (so->local);

    if (hasAlgebraic(sMode))
    {
        if (localStates.algSize > 0)
        {
            assert(so->algOffset != kNullLocation);
            std::copy(m_state.begin(), m_state.begin() + localStates.algSize, state + so->algOffset);
        }
    }
    if (localStates.diffSize > 0)
    {
        if (isDifferentialOnly(sMode))
        {
            assert(so->diffOffset != kNullLocation);
            index_t localAlgSize = algSize(cLocalSolverMode);
            std::copy(m_state.begin() + localAlgSize, m_state.begin() + localAlgSize + localStates.diffSize, state + so->diffOffset);
            std::copy(m_dstate_dt.data() + localAlgSize, m_dstate_dt.data() + localAlgSize + localStates.diffSize, dstate_dt + so->diffOffset);
        }
        else
        {
            if (so->diffOffset == kNullLocation)
            {
                printf("%s::%s in mode %d %d ds=%d, do=%d\n", getParent()->getName().c_str(), getName().c_str(), isLocal(sMode), isDAE(sMode), static_cast<int> (so->total.diffSize), static_cast<int> (so->diffOffset));
                printStackTrace();
            }
            assert(so->diffOffset != kNullLocation);
            count_t stateCount = localStates.algSize + localStates.diffSize;
            std::copy(m_state.begin() + localStates.algSize, m_state.begin() + stateCount, state + so->diffOffset);
            std::copy(m_dstate_dt.data() + localStates.algSize, m_dstate_dt.data() + stateCount, dstate_dt + so->diffOffset);
        }
    }
    
        for (auto &sub : subObjectList)
        {
            sub->guess(ttime, state, dstate_dt, sMode);
        }
}

void gridObject::setupPFlowFlags ()
{

  auto ss = stateSize (cPflowSolverMode);
  opFlags.set (has_pflow_states,(ss > 0));

}

void gridObject::setupDynFlags ()
{
  auto ss = stateSize (cDaeSolverMode);

  opFlags.set (has_dyn_states,(ss > 0));
  auto so = offsets.getOffsets (cDaeSolverMode);
  if (so->total.algRoots > 0)
    {
      opFlags.set (has_alg_roots);
      opFlags.set (has_roots);
    }
  else if (so->total.diffRoots > 0)
    {
      opFlags.reset (has_alg_roots);
      opFlags.set (has_roots);
    }
  else
    {
      opFlags.reset (has_alg_roots);
      opFlags.reset (has_roots);
    }
}

double gridObject::getState (index_t offset) const
{
  if ((offset != kNullLocation)&&(offset < m_state.size ()))
    {
      return m_state[offset];
    }
  else
    {
      return kNullVal;
    }
}

void gridObject::loadSizesSub (const solverMode &sMode, bool dynOnly)
{
  auto so = offsets.getOffsets (sMode);
  if (dynOnly)
    {
      so->total.algRoots = so->local.algRoots;
      so->total.diffRoots = so->local.diffRoots;
      so->total.jacSize = so->local.jacSize;
    }
  else
    {
      so->localLoad (false);
    }
  for (auto &sub : subObjectList)
    {
      if (sub->isEnabled())
        {
          if (!(sub->isLoaded (sMode, dynOnly)))
            {
              sub->loadSizes (sMode, dynOnly);
            }
		  if (sub->checkFlag(sampled_only))
		  {
			  continue;
		  }
          if (dynOnly)
            {
              so->addRootAndJacobianSizes (sub->offsets.getOffsets (sMode));
            }
          else
            {
              so->addSizes (sub->offsets.getOffsets (sMode));
            }
        }
    }
  if (!dynOnly)
    {
      so->stateLoaded = true;

    }
  so->rjLoaded = true;
}


void gridObject::loadSizes (const solverMode &sMode, bool dynOnly)
{
  if (isLoaded (sMode,dynOnly))
    {
      return;
    }
  auto so = offsets.getOffsets (sMode);
  if (!isEnabled())
    {
      so->reset ();
      so->stateLoaded = true;
      so->rjLoaded = true;
      return;
    }
  if ((!isDynamic (sMode))&&(opFlags[no_pflow_states] ))
    {
      so->stateReset ();
      so->stateLoaded = true;
      return;
    }
  if ((isDynamic (sMode)) && (opFlags[no_dyn_states]))
    {
      so->stateReset ();
      so->stateLoaded = true;
    }
  auto &lc = offsets.local();
  if (dynOnly)
    {
      if (!isLocal (sMode))    //don't reset if it is the local offsets
        {
          so->rootAndJacobianCountReset ();
        }
    }
  else
    {
      if (!(so->stateLoaded))
        {
          if (!isLocal (sMode))        //don't reset if it is the local offsets
            {
              so->stateReset ();
            }
          if (hasAlgebraic (sMode))
            {
              
              so->local.aSize = lc.local.aSize;
              so->local.vSize = lc.local.vSize;
              so->local.algSize = lc.local.algSize;
            }
          if (hasDifferential (sMode))
            {
              so->local.diffSize = lc.local.diffSize;
            }
        }
    }
  if (!(so->rjLoaded))
    {
      so->local.algRoots = lc.local.algRoots;
      so->local.diffRoots = lc.local.diffRoots;
      so->local.jacSize = lc.local.jacSize;
    }
  if (opFlags[sampled_only]) //no states 
  {
      if (sMode == cLocalSolverMode)
      {
          for (auto &sub : subObjectList)
          {
              sub->setFlag("sampled_only");
          }
      }
      else
      {
          so->local.reset();
          so->total.reset();
      }
      
     
  }
  if (subObjectList.empty ())
    {
      if (dynOnly)
        {
          so->total.algRoots = so->local.algRoots;
          so->total.diffRoots = so->local.diffRoots;
          so->total.jacSize = so->local.jacSize;
          so->rjLoaded = true;
        }
      else
        {
          so->localLoad (true);
        }
    }
  else
    {
      loadSizesSub (sMode, dynOnly);
    }
}


void gridObject::getTols (double tols[], const solverMode &sMode)
{
  for (auto &so:subObjectList)
    {
      if (so->isEnabled())
        {
          so->getTols (tols,sMode);
        }
    }
}

void gridObject::getVariableType (double sdata[], const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (subObjectList.empty ())
    {
      if (so->total.algSize > 0)
        {
          auto offset = so->algOffset;
          for (count_t kk = 0; kk < so->total.algSize; ++kk)
            {
              sdata[offset + kk] = ALGEBRAIC_VARIABLE;
            }
        }
      if (so->total.diffSize > 0)
        {
          auto offset = so->diffOffset;
          for (count_t kk = 0; kk < so->total.diffSize; ++kk)
            {
              sdata[offset + kk] = DIFFERENTIAL_VARIABLE;
            }
        }
    }
  else
    {
      if (so->local.algSize > 0)
        {
          auto offset = so->algOffset;
          for (count_t kk = 0; kk < so->local.algSize; ++kk)
            {
              sdata[offset + kk] = ALGEBRAIC_VARIABLE;
            }
        }
      if (so->local.diffSize > 0)
        {
          auto offset = so->diffOffset;
          for (count_t kk = 0; kk < so->local.diffSize; ++kk)
            {
              sdata[offset + kk] = DIFFERENTIAL_VARIABLE;
            }
        }
      for (auto &subobj : subObjectList)
        {
          if (subobj->isEnabled())
            {
              subobj->getVariableType (sdata, sMode);
            }
        }
    }
}

void gridObject::getConstraints (double constraints[], const solverMode &sMode)
{
  for (auto &so : subObjectList)
    {
      if ((so->isEnabled())&&(so->checkFlag(has_constraints)))
        {
          so->getConstraints (constraints, sMode);
        }
    }
}

void gridObject::setRootOffset (index_t Roffset, const solverMode &sMode)
{

  offsets.setRootOffset (Roffset, sMode);
  auto so = offsets.getOffsets (sMode);
  auto nR = so->local.algRoots + so->local.diffRoots;
  for (auto &ro : subObjectList)
    {
      ro->setRootOffset (Roffset + nR, sMode);
      nR += ro->rootSize (sMode);
    }
}

//TODO:: Make this function actually work
void gridObject::setParamOffset (index_t Poffset, const solverMode &sMode)
{

  offsets.setParamOffset (Poffset);
  //auto so = offsets.getOffsets (sMode);
  for (auto &ro : subObjectList)
    {
      ro->setParamOffset (Poffset,sMode);
      //nR += ro->paramCount(sMode);
    }
}

static const stringVec emptyStr{};

stringVec gridObject::localStateNames() const
{
    return emptyStr;
}



index_t gridObject::findIndex(const std::string & field, const solverMode &sMode) const
{
    auto so = offsets.getOffsets(sMode);
    if (field.compare(0, 5, "state") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt(field, 0));
        if (stateSize(sMode) > num)
        {
            if (so->algOffset != kNullLocation)
            {
                return so->algOffset + num;
            }
            else
            {
                return kNullLocation;
            }
        }
        else
        {
            return kInvalidLocation;
        }
    }
    if (field.compare(0, 3, "alg") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt(field, 0));
        if (so->total.algSize > num)
        {
            if (so->algOffset != kNullLocation)
            {
                return so->algOffset + num;
            }
            else
            {
                return kNullLocation;
            }
        }
        else if (opFlags[dyn_initialized] == false)
        {
            return kNullLocation;
        }
        else
        {
            return kInvalidLocation;
        }
    }
    if (field.compare(0, 4, "diff") == 0)
    {
        auto num = static_cast<index_t> (stringOps::trailingStringInt(field, 0));
        if (so->total.diffSize > num)
        {
            if (so->diffOffset != kNullLocation)
            {
                return so->diffOffset + num;
            }
            else
            {
                return kNullLocation;
            }
        }
        else if (opFlags[dyn_initialized] == false)
        {
            return kNullLocation;
        }
        else
        {
            return kInvalidLocation;
        }
    }
    auto stateNames = localStateNames();
    for (index_t nn = 0; nn < stateNames.size(); ++nn)
    {
        if (field == stateNames[nn])
        {
			auto &lc = offsets.local();
            if (nn < lc.local.algSize)
            {
                if (so->algOffset != kNullLocation)
                {
                    return so->algOffset + nn;
                }
                else
                {
                    return kNullLocation;
                }
            }
            else if (nn - lc.local.algSize < lc.local.diffSize)
            {
                if (so->diffOffset != kNullLocation)
                {
                    return so->diffOffset + nn - lc.local.algSize;
                }
                else
                {
                    return kNullLocation;
                }
            }
            else if (opFlags[dyn_initialized] == false)
            {
                return kNullLocation;
            }
            else
            {
                return kInvalidLocation;
            }
        }
    }
    for (auto &subobj : subObjectList)
    {
        auto ret = subobj->findIndex(field, sMode);
        if (ret != kInvalidLocation)
        {
            return ret;
        }
    }
    return kInvalidLocation;
}

void gridObject::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  auto so = offsets.getOffsets (sMode);
  auto mxsize = offsets.maxIndex (sMode);
  std::string prefix2 = prefix + getName() + ':';

  if (mxsize > 0)
    {
      ensureSizeAtLeast(stNames, mxsize + 1);
    }
  else
    {
      return;
    }
  auto stateNames = localStateNames();
  size_t stsize = stateNames.size();
  size_t ss = 0;
  if (hasAlgebraic (sMode))
    {
          for (size_t kk = 0; kk < so->local.vSize; kk++)
          {
              if (stsize > ss)
              {
                  stNames[so->vOffset + kk] =prefix2 + stateNames[ss];
                  ++ss;
              }
              else
              {
                  stNames[so->vOffset + kk] =
                      prefix2 + "voltage_state_" + std::to_string(kk);
              }
          }
		  ss = offsets.local().local.vSize;
          for (size_t kk = 0; kk < so->local.aSize; kk++)
          {
              if (stsize > ss)
              {
                  stNames[so->aOffset + kk] =prefix2 + stateNames[ss];
                  ++ss;
              }
              else
              {
                  stNames[so->aOffset + kk] =
                      prefix2 + "angle_state_" + std::to_string(kk);
              }
          }
		  ss = offsets.local().local.vSize + offsets.local().local.aSize;
          for (size_t kk = 0; kk < so->local.algSize; kk++)
          {
              if (stsize > ss)
              {
                  stNames[so->algOffset + kk] = prefix2 + stateNames[ss];
                  ++ss;
              }
              else
              {
                  stNames[so->algOffset + kk] =
                      prefix2 + "alg_state_" + std::to_string(kk);
              }
             
          }
          
    }
  if (!isAlgebraicOnly (sMode))
    {
      if (so->local.diffSize > 0)
        {
		  ss = offsets.local().local.algSize + offsets.local().local.vSize + offsets.local().local.aSize;
          for (size_t kk = 0; kk < so->local.diffSize; kk++)
            {
              if (stsize > ss)
              {
                  stNames[so->diffOffset + kk] = prefix2 + stateNames[ss];
                  ++ss;
              }
              else
              {
                  stNames[so->diffOffset + kk] =
                      prefix2 + "diff_state_" + std::to_string(kk);
              }
            }
        }
    }

      for (auto &subobj : subObjectList)
        {
          subobj->getStateName (stNames, sMode,prefix2 + ':');
        }
}

void gridObject::updateFlags (bool dynamicsFlags)
{
  for (auto &so:subObjectList)
    {
      if (so->isEnabled())
        {
          opFlags |= so->cascadingFlags ();
        }
    }
  if (dynamicsFlags)
    {
      setupDynFlags ();
    }
  else
    {
      setupPFlowFlags ();
    }

  opFlags.reset (flag_update_required);
}

void gridObject::updateLocalCache(const IOdata & inputs, const stateData &sD, const solverMode &sMode)
{
    for (auto &sub : subObjectList)
    {
        sub->updateLocalCache(inputs, sD, sMode);
    }
}



coreObject* gridObject::find(const std::string &object) const
{
    auto foundobj = std::find_if(subObjectList.begin(), subObjectList.end(), [object](gridObject *obj) {return (obj->getName() == object); });
    if (foundobj == subObjectList.end())
    {
        return coreObject::find(object);
    }
    else
    {
        return *foundobj;
    }
}

coreObject* gridObject::getSubObject(const std::string & typeName, index_t num) const
{
    if ((typeName == "sub") || (typeName == "subobject") || (typeName == "object"))
    {
        if (num < subObjectList.size())
        {
            return subObjectList[num];
        }
    }
    return nullptr;
}

coreObject * gridObject::findByUserID(const std::string & typeName, index_t searchID) const
{
    if ((typeName == "sub") || (typeName == "subobject") || (typeName == "object"))
    {
        auto foundobj=std::find_if(subObjectList.begin(), subObjectList.end(), [searchID](gridObject *obj) {return (obj->getUserID() == searchID); });
        if (foundobj == subObjectList.end())
        {
            return nullptr;
        }
        else
        {
            return *foundobj;
        }
    }
    return coreObject::findByUserID(typeName, searchID);
}


void gridObject::timestep(coreTime ttime, const IOdata & inputs, const solverMode & sMode)
{
    prevTime = ttime;

    for (auto &subobj : subObjectList)
    {
        if (subobj->currentTime() < ttime)
        {
			if (!subobj->checkFlag(separate_processing))
			{
				subobj->timestep(ttime, inputs, sMode);
			}
            
        }
    }

}


void gridObject::ioPartialDerivatives(const IOdata & /*inputs*/, const stateData &, matrixData<double> &, const IOlocs & /*inputLocs*/, const solverMode & /*sMode*/)
{
    /* there is no way to determine partial derivatives of the output with respect to input in a default manner
    therefore the default is not dependencies
    */
}

void gridObject::outputPartialDerivatives(const IOdata & /*inputs*/, const stateData &, matrixData<double> &ad, const solverMode &sMode)
{
    /* assume the output is a state and compute accordingly*/
    for (index_t kk = 0; kk < m_outputSize; ++kk)
    {
        index_t oLoc = getOutputLoc(sMode, kk);
        ad.assignCheckCol(kk, oLoc, 1.0);
    }
}

count_t gridObject::outputDependencyCount(index_t num, const solverMode &sMode) const
{
	/* assume the output is a state and act accordingly*/
	
		index_t oLoc = getOutputLoc(sMode, num);
		return (oLoc == kInvalidLocation) ? 0 : 1;
}

void gridObject::preEx(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag(preEx_requested)))
        {
            continue;
        }
		if (!subobj->checkFlag(separate_processing))
		{
			subobj->preEx(inputs, sD, sMode);
		}
    }
}


void gridObject::residual(const IOdata & inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
	for (auto &sub : subObjectList)
	{
		if (!sub->checkFlag(separate_processing))
		{
			if (sub->stateSize(sMode) > 0)
			{

				sub->residual(inputs, sD, resid, sMode);
			}
		}
		
	}
}

void gridObject::derivative(const IOdata & inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
	for (auto &sub : subObjectList)
	{
		if (!sub->checkFlag(separate_processing))
		{
			if (sub->diffSize(sMode) > 0)
			{
				sub->derivative(inputs, sD, deriv, sMode);
			}
		}

	}
}

void gridObject::algebraicUpdate(const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha)
{
	for (auto &sub : subObjectList)
	{
		if (!sub->checkFlag(separate_processing))
		{
			if (sub->algSize(sMode) > 0)
			{
				sub->algebraicUpdate(inputs, sD, update, sMode, alpha);
			}
		}

	}
}

void gridObject::jacobianElements(const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs & inputLocs, const solverMode &sMode)
{
	for (auto &sub : subObjectList)
	{
		if (!sub->checkFlag(separate_processing))
		{
			if (sub->stateSize(sMode) > 0)
			{
				sub->jacobianElements(inputs, sD, ad, inputLocs, sMode);
			}
		}

	}
}
void gridObject::rootTest(const IOdata & inputs, const stateData &sD, double roots[], const solverMode &sMode)
{

    for (auto &subobj : subObjectList)
    {
		if (!subobj->checkFlag(separate_processing))
		{
			if (!(subobj->checkFlag(has_roots)))
			{
				continue;
			}
			subobj->rootTest(inputs, sD, roots, sMode);
		}
    }
}

void gridObject::rootTrigger(coreTime ttime, const IOdata & inputs, const std::vector<int> & rootMask, const solverMode & sMode)
{

    for (auto &subobj : subObjectList)
    {
        if (!(subobj->checkFlag(has_roots)))
        {
            continue;
        }
		if (!subobj->checkFlag(separate_processing))
		{
			subobj->rootTrigger(ttime, inputs, rootMask, sMode);
		}
    }
}

change_code gridObject::rootCheck(const IOdata & inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
    auto ret = change_code::no_change;

        for (auto &subobj : subObjectList)
        {
            if (!(subobj->checkFlag(has_roots)))
            {
                continue;
            }
			if (!subobj->checkFlag(separate_processing))
			{
				ret = std::max(subobj->rootCheck(inputs, sD, sMode, level), ret);
			}
        }
    return ret;
}


double gridObject::getOutput(const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode, index_t Num) const
{
    if (Num > m_outputSize)
    {
        return kNullVal;
    }
    Lp Loc = offsets.getLocations(sD, sMode, this);
    if (opFlags[differential_output])
    {
        if (Loc.diffSize > Num)
        {
            return Loc.diffStateLoc[Num];
        }
        else
        {
            return kNullVal;
        }
    }
    else
    { //if differential flag was not specified try algebraic state values then differential

        if (Loc.algSize > Num)
        {
            return Loc.algStateLoc[Num];
        }
        else if (Loc.diffSize + Loc.algSize > Num)
        {
            return Loc.diffStateLoc[Num - Loc.algSize];
        }
        else
        {
            return (m_state.size() > Num) ? m_state[Num] : kNullVal;
        }
    }

}


double gridObject::getOutput(index_t Num) const
{
    return getOutput(noInputs, emptyStateData, cLocalSolverMode, Num);
}


IOdata gridObject::getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    IOdata mout(m_outputSize);
    for (count_t pp = 0; pp < m_outputSize; ++pp)
    {
        mout[pp] = getOutput(inputs, sD, sMode, pp);
    }
    return mout;

}

//static IOdata kNullVec;



double gridObject::getDoutdt(const IOdata &/*inputs*/,const stateData &sD, const solverMode &sMode, index_t Num) const
{
    if (Num > m_outputSize)
    {
        return kNullVal;
    }
    Lp Loc = offsets.getLocations(sD, sMode, this);
    if (opFlags[differential_output])
    {
        return Loc.dstateLoc[Num];
    }
    else
    {
        if (Loc.algSize > Num)
        {
            return 0.0;
        }
        else if (Loc.diffSize + Loc.algSize > Num)
        {
            return Loc.dstateLoc[Num - Loc.algSize];
        }
        else
        {
            return 0.0;
        }
    }

}

index_t gridObject::getOutputLoc(const solverMode &sMode, index_t Num) const
{
    if (Num > m_outputSize)
    {
        return kNullLocation;
    }

    if (opFlags[differential_output])
    {
        if (Num < diffSize(sMode))
        {
            return offsets.getDiffOffset(sMode) + Num;
        }
        Num -= diffSize(sMode);
        return offsets.getAlgOffset(sMode) + Num - diffSize(sMode);
    }

    auto so = offsets.getOffsets(sMode);
    if (so->total.algSize > Num)
    {
        return so->algOffset + Num;
    }
    if (so->total.diffSize + so->total.algSize > Num)
    {
        return so->diffOffset - so->total.algSize + Num;
    }

    return kNullLocation;
}

IOlocs gridObject::getOutputLocs(const solverMode &sMode) const
{
    IOlocs oloc(m_outputSize);

    if (!isLocal(sMode))
    {
        for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
            oloc[pp] = getOutputLoc(sMode, pp);
        }
    }
    else
    {
        for (count_t pp = 0; pp < m_outputSize; ++pp)
        {
            oloc[pp] = kNullLocation;
        }
    }
    return oloc;

}

void printStateNames(gridObject *obj, const solverMode &sMode)
{
    std::vector < std::string> sNames;
    obj->getStateName(sNames, sMode);
    int kk = 0;
    for (auto &sn : sNames)
    {
        std::cout << kk++ << ' ' << sn << '\n';
    }
}