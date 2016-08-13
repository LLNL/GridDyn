/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "gridObjects.h"
#include "gridCoreTemplates.h"
#include "stackInfo.h"
#include "stringOps.h"
#include <cstdio>
#include <iostream>
#include <map>
#include <cassert>


//this functions is here since it depends on gridObject information
Lp offsetTable::getLocations (const stateData *sD, double d[], const solverMode &sMode, const gridObject *obj) const
{
  Lp Loc;
  Loc.algOffset = offsetContainer[sMode.offsetIndex].algOffset;
  Loc.diffOffset = offsetContainer[sMode.offsetIndex].diffOffset;
  Loc.diffSize = offsetContainer[sMode.offsetIndex].total.diffSize;
  Loc.algSize = offsetContainer[sMode.offsetIndex].total.algSize;
  if ((sMode.local)||(!(sD)))
    {
      Loc.time = obj->prevTime;
      Loc.algStateLoc = obj->m_state.data ();
      Loc.destLoc = d;
      Loc.diffStateLoc = obj->m_state.data () + Loc.algSize;
      Loc.destDiffLoc = d + Loc.algSize;
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
      Loc.time = sD->time;
      Loc.algStateLoc = sD->state + Loc.algOffset;
      Loc.destLoc = d + Loc.algOffset;
      Loc.diffStateLoc = sD->state + Loc.diffOffset;
      Loc.destDiffLoc = d + Loc.diffOffset;
      Loc.dstateLoc = sD->dstate_dt + Loc.diffOffset;
    }
  else if (isAlgebraicOnly (sMode))
    {
      Loc.time = sD->time;
      if (sD->state)
        {
          Loc.algStateLoc = sD->state + Loc.algOffset;
        }
      else
        {
          Loc.algStateLoc = sD->algState + Loc.algOffset;
        }
      Loc.destLoc = d + Loc.algOffset;
      if (sD->fullState)
        {
          Loc.diffStateLoc = sD->fullState + offsetContainer[sD->pairIndex].diffOffset;
        }
      else if (sD->diffState)
        {
          Loc.diffStateLoc = sD->diffState + offsetContainer[sD->pairIndex].diffOffset;
        }
      else
        {
          Loc.diffStateLoc = obj->m_state.data () + offsetContainer[0].diffOffset;
        }
      Loc.destDiffLoc = nullptr;
      if (sD->dstate_dt)
        {
          Loc.dstateLoc = sD->dstate_dt + offsetContainer[sD->pairIndex].diffOffset;
        }
      else
        {
          Loc.dstateLoc = obj->m_dstate_dt.data () + offsetContainer[0].diffOffset;
        }

    }
  else if (isDifferentialOnly (sMode))
    {
      Loc.time = sD->time;
      if (sD->state)
        {
          Loc.diffStateLoc = sD->state + Loc.diffOffset;
        }
      else
        {
          Loc.diffStateLoc = sD->diffState + Loc.diffOffset;
        }
      Loc.dstateLoc = sD->dstate_dt + Loc.diffOffset;
      Loc.destDiffLoc = d + Loc.diffOffset;
      if (sD->fullState)
        {
          Loc.algStateLoc = sD->fullState + offsetContainer[sD->pairIndex].algOffset;
        }
      else if (sD->algState)
        {
          Loc.algStateLoc = sD->algState + offsetContainer[sD->pairIndex].algOffset;
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
      Loc.destLoc = d;
      Loc.diffStateLoc = obj->m_state.data () + Loc.algSize;
      Loc.destDiffLoc = d + Loc.algSize;
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

//this functions is here since it depends on gridObject information
Lp offsetTable::getLocations (const stateData *sD, const solverMode &sMode, const gridObject *obj) const
{
  Lp Loc;
  Loc.algOffset = offsetContainer[sMode.offsetIndex].algOffset;
  Loc.diffOffset = offsetContainer[sMode.offsetIndex].diffOffset;
  Loc.diffSize = offsetContainer[sMode.offsetIndex].total.diffSize;
  Loc.algSize = offsetContainer[sMode.offsetIndex].total.algSize;
  if ((sMode.local) || (!(sD)))
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
      Loc.time = sD->time;
      Loc.algStateLoc = sD->state + Loc.algOffset;
      Loc.diffStateLoc = sD->state + Loc.diffOffset;
      Loc.dstateLoc = sD->dstate_dt + Loc.diffOffset;
    }
  else if (hasAlgebraic (sMode))
    {
      Loc.time = sD->time;
      if (sD->state)
        {
          Loc.algStateLoc = sD->state + Loc.algOffset;
        }
      else
        {
          Loc.algStateLoc = sD->algState + Loc.algOffset;
        }
      if (sD->fullState)
        {
          Loc.diffStateLoc = sD->fullState + offsetContainer[sD->pairIndex].diffOffset;
        }
      else if (sD->diffState)
        {
          Loc.diffStateLoc = sD->diffState + offsetContainer[sD->pairIndex].diffOffset;
        }
      else
        {
          Loc.diffStateLoc = obj->m_state.data () + offsetContainer[0].diffOffset;
        }
      Loc.destDiffLoc = nullptr;
      if (sD->dstate_dt)
        {
          Loc.dstateLoc = sD->dstate_dt + offsetContainer[sD->pairIndex].diffOffset;
        }
      else
        {
          Loc.dstateLoc = obj->m_dstate_dt.data () + offsetContainer[0].diffOffset;
        }

    }
  else if (hasDifferential (sMode))
    {
      Loc.time = sD->time;
      if (sD->state)
        {
          Loc.diffStateLoc = sD->state + Loc.diffOffset;
        }
      else
        {
          Loc.diffStateLoc = sD->diffState + Loc.diffOffset;
        }
      Loc.dstateLoc = sD->dstate_dt + Loc.diffOffset;
      if (sD->fullState)
        {
          Loc.algStateLoc = sD->fullState + offsetContainer[sD->pairIndex].algOffset;
        }
      else if (sD->algState)
        {
          Loc.algStateLoc = sD->algState + offsetContainer[sD->pairIndex].algOffset;
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

gridObject::gridObject (const std::string &objName) : gridCoreObject (objName)
{
  offsets.setAlgOffset (0,cLocalSolverMode);
}

gridObject::~gridObject ()
{
  for (auto &so : subObjectList)
    {
      condDelete (so, this);
    }
}

gridCoreObject *gridObject::clone (gridCoreObject *obj) const
{
  gridObject *nobj = cloneBase<gridObject, gridCoreObject> (this, obj);
  if (!(nobj))
    {
      return obj;
    }
  nobj->opFlags = opFlags;
  nobj->m_baseFreq = m_baseFreq;
  return nobj;
}




count_t gridObject::stateSize (const solverMode &sMode)
{
  auto so = offsets.getOffsets (sMode);
  if (!(so->stateLoaded))
    {
      loadSizes (sMode,false);
    }
  if (isAlgebraicOnly (sMode))
    {
      return so->total.algSize + so->total.vSize + so->total.aSize;
    }
  else if (isDifferentialOnly (sMode))
    {
      return so->total.diffSize;
    }
  else
    {
      return so->total.algSize + so->total.vSize + so->total.aSize + so->total.diffSize;
    }

}

count_t gridObject::stateSize (const solverMode &sMode) const
{
  auto so = offsets.getOffsets (sMode);
  if (!so)
    {
      return 0;
    }
  if (isAlgebraicOnly (sMode))
    {
      return so->total.algSize + so->total.vSize + so->total.aSize;
    }
  else if (isDifferentialOnly (sMode))
    {
      return so->total.diffSize;
    }
  else
    {
      return so->total.algSize + so->total.vSize + so->total.aSize + so->total.diffSize;
    }

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
  return numParams;
}

count_t gridObject::paramSize (const solverMode &) const
{
  return numParams;
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
          so->setOffsets (no, sMode);
          no.increment (so->offsets.getOffsets (sMode));
        }
    }
}

void gridObject::setOffset (index_t newOffset, const solverMode &sMode)
{
  if (!subObjectList.empty ())
    {
      for (auto &so : subObjectList)
        {
          so->setOffset (newOffset, sMode);
          newOffset += so->stateSize (sMode);
        }
    }
  offsets.setOffset (newOffset, sMode);
}

bool gridObject::isLoaded (const solverMode &sMode,bool dynOnly) const
{
  return (dynOnly) ? offsets.isrjLoaded (sMode) : offsets.isLoaded (sMode);
}

//there isn't that many flags that we want to be user settable, most are controlled by the model so allowing them to be set by an external function
// might not be the best thing
int gridObject::setFlag (const std::string &flag, bool val)
{
  int out = FLAG_FOUND;
  if (flag == "error")
    {
      opFlags.set (error_flag, val);
    }
  else if (flag == "late_b_initialize")
    {
      opFlags.set (late_b_initialize, val);
    }
  else if (flag == "no_gridobject_set")
    {
      opFlags.set (no_gridobject_set, val);
    }
  else
    {
      out = gridCoreObject::setFlag (flag,val);
    }
  return out;
}

/* *INDENT-OFF* */
static std::map<std::string, operation_flags> flagmap
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
  {"extra_cascade", extra_cascading_flag},
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
  {"disable_flag_update", disable_flag_updates},
  {"flag_update_required", flag_update_required},
  { "differential_output", differential_output },
  {"multipart_calculation_capable", multipart_calculation_capable},
  {"pflow_constant_initialization", pflow_constant_initialization},
  {"dc_only", dc_only},
  {"dc_capable", dc_capable},
  {"dc_terminal2", dc_terminal2},
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
      return FLAG_NOT_FOUND;
    }
}

bool gridObject::checkFlag (index_t flagID) const
{
  return opFlags[flagID];
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

static stringVec locNumStrings {
  "status", "basefrequency"
};
static const stringVec locStrStrings {
  "status"
};

void gridObject::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
  getParamString<gridObject,gridCoreObject> (this,pstr,locNumStrings,locStrStrings,{},pstype);
}

int gridObject::set (const std::string &param,  const std::string &val)
{
  if (opFlags[no_gridobject_set])
    {
      return PARAMETER_NOT_FOUND;
    }
  int out = PARAMETER_FOUND;
  if (param == "status")
    {
      auto v2 = convertToLowerCase (val);
      if ((v2 == "on") || (v2 == "in")||(v2 == "enabled"))
        {
          if (!enabled)
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
          if (enabled)
            {

              if ((opFlags[has_pflow_states]) || (opFlags[has_dyn_states]))
                {
                  alert (this, STATE_COUNT_CHANGE);
                }
              disable ();
            }
        }
    }
  else if (param == "flags")
    {
      auto v = splitlineTrim (val);
      int ot;
      for (auto &temp : v)
        {
          makeLowerCase (temp);              //make the flags lower case
          ot = setFlag (temp, true);
          if (ot != PARAMETER_FOUND)
            {
              LOG_NORMAL ("flag " + temp + " not found");
            }
        }
    }
  else
    {
      out = gridCoreObject::set (param, val);
    }
  return out;
}

int gridObject::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  if (opFlags[no_gridobject_set])
    {
      return PARAMETER_NOT_FOUND;
    }
  int out = PARAMETER_FOUND;
  if ((param == "enabled")||(param == "status"))
    {
      if (val > 0)
        {
          if (!enabled)
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
          if (enabled)
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
    }
  else if ((param == "basefreq") || (param == "basefrequency"))
    {
      m_baseFreq = gridUnits::unitConversionFreq (val, unitType, gridUnits::rps);
    }
  else
    {
      out = gridCoreObject::set (param, val, unitType);
    }

  return out;
}




void gridObject::setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  prevTime = ttime;

  if (subObjectList.empty ())
    {
      auto so = offsets.getOffsets (sMode);
      if (isDAE (sMode))
        {
          if (!m_dstate_dt.empty ())
            {
              if (so->total.algSize > 0)
                {
                  std::copy (state + so->algOffset, state + so->algOffset + so->total.algSize, m_state.data ());
                }
              if (so->total.diffSize > 0)
                {
                  std::copy (state + so->diffOffset, state + so->diffOffset + so->total.diffSize, m_state.data () + so->total.algSize);
                  std::copy (dstate_dt + so->diffOffset, dstate_dt + so->diffOffset + so->total.diffSize, m_dstate_dt.data () + so->total.algSize);
                }
            }
        }
      else if (isAlgebraicOnly (sMode))
        {
          if (!m_state.empty ())
            {
              if (so->total.algSize > 0)
                {
                  std::copy (state + so->algOffset, state + so->algOffset + so->total.algSize, m_state.data ());
                }
            }
        }
      else if (isDifferentialOnly (sMode))
        {
          if (!m_dstate_dt.empty ())
            {
              if (so->total.diffSize > 0)
                {
                  std::copy (state + so->diffOffset, state + so->diffOffset + so->total.diffSize, m_state.data () + so->total.algSize);
                  std::copy (dstate_dt + so->diffOffset, dstate_dt + so->diffOffset + so->total.diffSize, m_dstate_dt.data () + so->total.algSize);
                }
            }
        }
    }
  else
    {
      for (auto &sub : subObjectList)
        {
          sub->setState (ttime, state, dstate_dt, sMode);
        }
    }

}
//for saving the state
void gridObject::guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (subObjectList.empty ())
    {
      auto so = offsets.getOffsets (sMode);
      count_t stateCount = static_cast<count_t> (m_state.size ());
      if (isDAE (sMode))
        {
          if (so->total.algSize > 0)
            {
              assert (so->algOffset != kNullLocation);
              std::copy (m_state.begin (), m_state.begin () + so->total.algSize, state + so->algOffset);
            }
          if (so->total.diffSize > 0)
            {
              if (so->diffOffset == kNullLocation)
                {
                  printf ("%s::%s in mode %d %d ds=%d, do=%d\n",parent->getName ().c_str (),name.c_str (),isLocal (sMode),isDAE (sMode),static_cast<int> (so->total.diffSize),static_cast<int> (so->diffOffset));
                  printStackTrace ();
                }
              assert (so->diffOffset != kNullLocation);
              std::copy (m_state.begin () + so->total.algSize, m_state.begin () + stateCount, state + so->diffOffset);
              std::copy (m_dstate_dt.data () + so->total.algSize, m_dstate_dt.data () + stateCount, dstate_dt + so->diffOffset);
            }
        }
      else if (isAlgebraicOnly (sMode))
        {
          if (so->total.algSize > 0)
            {
              assert (so->algOffset != kNullLocation);
              std::copy (m_state.begin (), m_state.begin () + so->total.algSize, state + so->algOffset);
            }
        }
      else if (isDifferentialOnly (sMode))
        {
          if (so->total.diffSize > 0)
            {
              assert (so->diffOffset != kNullLocation);
              std::copy (m_state.begin () + algSize (cLocalSolverMode), m_state.begin () + stateCount, state + so->diffOffset);
              std::copy (m_dstate_dt.data () + algSize (cLocalSolverMode), m_dstate_dt.data () + stateCount, dstate_dt + so->diffOffset);
            }
        }
    }
  else
    {
      for (auto &sub : subObjectList)
        {
          sub->guess (ttime, state, dstate_dt, sMode);
        }
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
      if (sub->enabled)
        {
          if (!(sub->isLoaded (sMode, dynOnly)))
            {
              sub->loadSizes (sMode, dynOnly);
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
  if (!enabled)
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
              so->local.aSize = offsets.local->local.aSize;
              so->local.vSize = offsets.local->local.vSize;
              so->local.algSize = offsets.local->local.algSize;
            }
          if (hasDifferential (sMode))
            {
              so->local.diffSize = offsets.local->local.diffSize;
            }
        }
    }
  if (!(so->rjLoaded))
    {
      so->local.algRoots = offsets.local->local.algRoots;
      so->local.diffRoots = offsets.local->local.diffRoots;
      so->local.jacSize = offsets.local->local.jacSize;
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
      if (so->enabled)
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
          if (subobj->enabled)
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
      if ((so->enabled)&&(opFlags[has_constraints]))
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


void gridObject::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  auto so = offsets.getOffsets (sMode);
  auto mxsize = offsets.maxIndex (sMode);
  std::string prefix2 = prefix + name + ':';

  if (mxsize > 0)
    {
      if (static_cast<index_t> (stNames.size ()) < mxsize)
        {
          stNames.resize (mxsize);
        }
    }
  else
    {
      return;
    }
  if (hasAlgebraic (sMode))
    {

      for (size_t kk = 0; kk < so->local.algSize; kk++)
        {
          stNames[so->algOffset + kk] =
            prefix2 + "alg_state_" + std::to_string (kk);
        }
      for (size_t kk = 0; kk < so->local.vSize; kk++)
        {
          stNames[so->vOffset + kk] =
            prefix2 + "voltage_state_" + std::to_string (kk);
        }
      for (size_t kk = 0; kk < so->local.aSize; kk++)
        {
          stNames[so->aOffset + kk] =
            prefix2 + "angle_state_" + std::to_string (kk);
        }
    }
  if (!isAlgebraicOnly (sMode))
    {
      if (so->local.diffSize > 0)
        {
          for (size_t kk = 0; kk < so->local.diffSize; kk++)
            {
              stNames[so->diffOffset + kk] =
                prefix2 + "diff_state_" + std::to_string (kk);
            }
        }
    }
  if (!subObjectList.empty ())
    {
      for (auto &subobj : subObjectList)
        {
          subobj->getStateName (stNames, sMode,prefix2 + ':');
        }
    }
}

void gridObject::updateFlags (bool dynamicsFlags)
{
  for (auto &so:subObjectList)
    {
      if (so->enabled)
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

void printStateNames (gridObject *obj,const solverMode &sMode)
{
  std::vector < std::string> sNames;
  obj->getStateName (sNames, sMode);
  int kk = 0;
  for (auto &sn : sNames)
    {
      std::cout << kk++ << ' ' << sn << '\n';
    }
}

