/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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


#include "generators/gridDynGenerator.h"
#include "gridCoreTemplates.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynPSS.h"
#include "submodels/otherGenModels.h"
#include "submodels/otherGovernors.h"
#include "objectFactoryTemplates.h"
#include "vectorOps.hpp"
#include "objectInterpreter.h"
#include "controllers/scheduler.h"
#include "gridBus.h"
#include "stringOps.h"
#include "variableGenerator.h"
#include "core/gridDynExceptions.h"
#include "matrixDataSparse.h"

//#include <set>
/*
For the dynamics states order matters for entries used across
multiple components and other parts of the program.

genModel
[theta, V, Id, Iq, delta, w]

exciter
[Ef]

governor --- Pm(t0) = Pset is stored externally as well
[Pm]
*/


static typeFactory<gridDynGenerator> gf ("generator", stringVec { "basic", "spinning" }, "basic");
static typeFactory<variableGenerator> vgf ("generator", stringVec { "variable", "renewable" });

using namespace gridUnits;

std::atomic<count_t> gridDynGenerator::genCount(0);
//default bus object
static gridBus defBus (1.0, 0);

gridDynGenerator::gridDynGenerator (const std::string &objName) : gridSecondary (objName)
{
  id = ++genCount;
  updateName ();
  opFlags.set (adjustable_P);
  opFlags.set (adjustable_Q);
  opFlags.set (local_voltage_control);
  opFlags.set (local_power_control);
}

gridDynGenerator::gridDynGenerator (dynModel_t dynModel, const std::string &objName) : gridDynGenerator (objName)
{

  buildDynModel (dynModel);
}
gridCoreObject *gridDynGenerator::clone (gridCoreObject *obj) const
{
  gridDynGenerator *gen = cloneBaseFactory<gridDynGenerator, gridSecondary> (this, obj,&gf);
  if (!(gen))
    {
      return obj;
    }

  for (auto &so : subObjectList)
    {
      bool fnd = false;
      for (auto &so2 : gen->subObjectList)
        {
          if (so2->locIndex == so->locIndex)
            {
              gen->add (so->clone (so2));
              fnd = true;
              break;
            }
        }
      if (!fnd)
        {
          gen->add (so->clone (nullptr));
        }
    }


  gen->baseVoltage = baseVoltage;
  gen->P          = P;
  gen->Q          = Q;
  gen->Pset               = Pset;
  gen->Qmax               = Qmax;
  gen->Qmin               = Qmin;
  gen->Pmax = Pmax;
  gen->Pmin = Pmin;
  gen->dPdt = dPdt;
  gen->dQdt = dQdt;
  gen->machineBasePower = machineBasePower;
  gen->participation = participation;
  gen->m_Rs = m_Rs;
  gen->m_Xs = m_Xs;
  gen->vRegFraction = vRegFraction;
  return gen;
}

gridDynGenerator::~gridDynGenerator ()
{

}

gridDynGenerator::dynModel_t gridDynGenerator::dynModelFromString (const std::string &dynModelType)
{
  auto str = convertToLowerCase (dynModelType);
  dynModel_t type = dynModel_t::invalid;
  if (str == "typical")
    {
      type = dynModel_t::typical;
    }
  else if (str == "simple")
    {
      type = dynModel_t::simple;
    }
  else if ((str == "model_only")||(str == "modelonly"))
    {
      type = dynModel_t::model_only;
    }
  else if (str == "transient")
    {
      type = dynModel_t::transient;
    }
  else if (str == "subtransient")
    {
      type = dynModel_t::detailed;
    }
  else if (str == "none")
    {
      type = dynModel_t::none;
    }

  else if (str == "dc")
    {
      type = dynModel_t::dc;
    }
  else if (str == "rewewable")
    {
      type = dynModel_t::renewable;
    }
  return type;
}

void gridDynGenerator::buildDynModel (dynModel_t dynModel)
{
  switch (dynModel)
    {
    case dynModel_t::simple:
      if (!(gov))
        {
          add (new gridDynGovernor ());
        }
      if (!(ext))
        {
          add (new gridDynExciter ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModelClassical ());
        }

      break;
    case dynModel_t::dc:
      if (!(gov))
        {
          add (new gridDynGovernorIeeeSimple ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModelClassical ());
        }

      break;
    case dynModel_t::typical:
      if (!(gov))
        {
          add (new gridDynGovernorIeeeSimple ());
        }
      if (!(ext))
        {
          add (new gridDynExciterIEEEtype1 ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModel4 ());
        }
      break;
    case dynModel_t::renewable:
      if (!(gov))
        {
          add (new gridDynGovernor ());
        }
      if (!(ext))
        {
          add (new gridDynExciter ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModelInverter ());
        }
      break;
    case dynModel_t::transient:
      if (!(gov))
        {
          add (new gridDynGovernorTgov1 ());
        }
      if (!(ext))
        {
          add (new gridDynExciterIEEEtype1 ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModel5 ());
        }
      break;
    case dynModel_t::subtransient:
      if (!(gov))
        {
          add (new gridDynGovernorTgov1 ());
        }
      if (!(ext))
        {
          add (new gridDynExciterIEEEtype1 ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModel6 ());
        }
    case dynModel_t::detailed:
      if (!(gov))
        {
          add (new gridDynGovernorTgov1 ());
        }
      if (!(ext))
        {
          add (new gridDynExciterIEEEtype1 ());
        }
      if (!(genModel))
        {
          add (new gridDynGenModel8 ());
        }
    case dynModel_t::model_only:
      if (!(genModel))
        {
          add (new gridDynGenModel4 ());
        }
    case dynModel_t::none:
      if (!(genModel))
        {
          add (new gridDynGenModel ());
        }
      break;
    case dynModel_t::invalid:
      break;
    default:
      break;
    }
}
void gridDynGenerator::pFlowObjectInitializeA (double time0, unsigned long /*flags*/)
{
  bus = static_cast<gridBus *> (find ("bus"));
  if (!bus)
    {
      bus = &defBus;
    }
  if (enabled)
    {
      if (opFlags[local_voltage_control])
        {
          if (bus->getType () != gridBus::busType::PQ)
            {
              bus->registerVoltageControl (this);
              opFlags.reset (indirect_voltage_control);
            }
          else if (opFlags[indirect_voltage_control])
            {
              remoteBus = bus;
              if (m_Vtarget < 0.6)
                {
                  m_Vtarget = remoteBus->get ("vtarget");
                }
              remoteBus->registerVoltageControl (this);
            }

        }
      else if (opFlags[remote_voltage_control])
        {
          if (m_Vtarget < 0.6)
            {
              m_Vtarget = remoteBus->get ("vtarget");
            }
          remoteBus->registerVoltageControl (this);
          if (remoteBus->getType () == gridBus::busType::PQ)
            {
              opFlags.set (indirect_voltage_control);
            }
        }
      //load up power control
      if (opFlags[local_power_control])
        {
          if (bus->getType () != gridBus::busType::PQ)
            {
              bus->registerPowerControl (this);
              opFlags.reset (indirect_voltage_control);
            }
        }
      else if (opFlags[remote_power_control])
        {
          //remote bus already configured
          remoteBus->registerPowerControl (this);
        }
      if (Pset < -kHalfBigNum)
        {
          Pset = P;
        }
    }
  else
    {
      P = 0;
      Q = 0;
    }
  prevTime = time0;
}

void gridDynGenerator::dynObjectInitializeA (double time0, unsigned long flags)
{

  if (machineBasePower < 0)
    {
      machineBasePower = systemBasePower;
    }
  //automatically define a trivial generator model if none has been specified
  if (!genModel)
    {
      add (new gridDynGenModel ());
    }
  if (gov)
    {
      if (!genModel->checkFlag (gridDynGenModel::genModel_flags::internal_frequency_calculation))
        {
          opFlags.set (uses_bus_frequency);
        }
    }
  gridSecondary::dynObjectInitializeA (time0, flags);

}

void gridDynGenerator::loadSizes (const solverMode &sMode, bool dynOnly)
{

  auto soff = offsets.getOffsets (sMode);
  if (!enabled)
    {
      soff->reset ();
      soff->rjLoaded = true;
      soff->stateLoaded = true;
      return;
    }
  if (dynOnly)
    {
      soff->rootAndJacobianCountReset ();
      if (!isDynamic (sMode))
        {
          soff->total.jacSize = offsets.local->local.jacSize;
          soff->rjLoaded = true;
          return;
        }

      else
        {
          soff->total.jacSize = offsets.local->local.jacSize;
          soff->total.algRoots = offsets.local->local.algRoots;
          soff->total.diffRoots = offsets.local->local.diffRoots;
        }
    }
  else
    {
      soff->reset ();
      if (!isDynamic (sMode))
        {
          if ((!isDC (sMode))&&(opFlags[indirect_voltage_control]))
            {
              soff->total.jacSize = 2;
              soff->total.algSize = 1;
            }
          soff->rjLoaded = true;
          soff->stateLoaded = true;
          return;
        }

      soff->localLoad (false);
    }

  if (isDC (sMode))
    {
      //TODO determine what to include for DC dynamic modes
    }
  else
    {
      for (auto &so : subObjectList)
        {
          if (!(so->isLoaded (sMode,dynOnly)))
            {
              so->loadSizes (sMode,dynOnly);
            }
          if (dynOnly)
            {
              soff->addRootAndJacobianSizes (so->getOffsets (sMode));
            }
          else
            {
              soff->addSizes (so->getOffsets (sMode));
            }
        }
    }
  if (!dynOnly)
    {
      soff->stateLoaded = true;
    }
  soff->rjLoaded = true;

  if (isLocal (sMode))
    {
      SSize = stateSize (cLocalbSolverMode);
      m_state.resize (SSize);
      m_dstate_dt.resize (SSize,0);
      m_state_ind.resize (SSize,1.0);
      setOffset (0,cLocalbSolverMode);
    }
}

// initial conditions of dynamic states
void gridDynGenerator::dynObjectInitializeB (const IOdata &args, const IOdata &outputSet)
{
  double V = args[voltageInLocation];
  double theta = args[angleInLocation];


  if (outputSet.empty ())
    {
    }
  else
    {
      if (outputSet[0] > -100000)
        {
          P = outputSet[0];
        }

      if (outputSet[1] > -100000)
        {
          Q = outputSet[1];
        }
    }
  if (std::abs (P) > 1.2 * machineBasePower)
    {
      LOG_WARNING ("Requested Power output significantly greater than internal base power, may cause dynamic model instability, suggest updating base power");
    }
  //load the power set point
  Pset = P;
  double scale = systemBasePower / machineBasePower;
  IOdata inputArgs (4);
  IOdata desiredOutput (4);

  inputArgs[voltageInLocation] = V;
  inputArgs[angleInLocation] = theta;
  inputArgs[genModelPmechInLocation] = kNullVal;
  inputArgs[genModelEftInLocation] = kNullVal;

  desiredOutput[PoutLocation] = P * scale;
  desiredOutput[QoutLocation] = Q * scale;

  IOdata computedInputs (4);
  genModel->initializeB (inputArgs, desiredOutput, computedInputs);
  m_Pmech = computedInputs[genModelPmechInLocation];

  m_Eft = computedInputs[genModelEftInLocation];
  genModel->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
  Pset = m_Pmech / scale;

  if ((ext) && (ext->enabled))
    {
      inputArgs[voltageInLocation] = V;
      inputArgs[angleInLocation] = theta;
      inputArgs[exciterPmechInLocation] = m_Pmech;

      desiredOutput[0] = m_Eft;
      ext->initializeB (inputArgs, desiredOutput, computedInputs);

      ext->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
      //Vset=inputSetup[1];

    }
  if ((gov) && (gov->enabled))
    {
      inputArgs[govOmegaInLocation] = m_baseFreq;
      inputArgs[govpSetInLocation] = kNullVal;

      desiredOutput[0] = Pset * scale;
      gov->initializeB (inputArgs, desiredOutput, computedInputs);

      gov->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
    }

  if ((pss) && (pss->enabled))
    {
      inputArgs[0] = m_baseFreq;
      inputArgs[1] = kNullVal;
      desiredOutput[0] = 0;
      pss->initializeB (inputArgs, desiredOutput, computedInputs);
      pss->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);

    }


  m_stateTemp = m_state.data ();
  m_dstate_dt_Temp = m_dstate_dt.data ();
}

// save an external state to the internal one
void gridDynGenerator::setState (double ttime, const double state[], const double dstate_dt[],const solverMode &sMode)
{

  if (isDynamic (sMode))
    {
      for (auto &so : subObjectList)
        {
          if (so->enabled)
            {
              so->setState (ttime, state, dstate_dt, sMode);
              so->guess (ttime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
            }
        }
      Pset += dPdt * (ttime - prevTime);
      Pset = valLimit (Pset, Pmin, Pmax);
      auto out = getOutputs ({},nullptr,cLocalSolverMode);
      P = -out[PoutLocation];
      Q = -out[QoutLocation];
    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getAlgOffset (sMode);
      Q = state[offset];
    }
}

//copy the curernt state to a vector
void gridDynGenerator::guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      for (auto &so : subObjectList)
        {
          if (so->enabled)
            {
              so->guess (ttime, state, dstate_dt, sMode);
              so->guess (ttime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
            }
        }
    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getAlgOffset (sMode);
      state[offset] = -Q;
    }

}


void gridDynGenerator::add (gridCoreObject *obj)
{
  if (dynamic_cast<gridSubModel *> (obj))
    {
      return add (static_cast<gridSubModel *> (obj));
    }
  else if (dynamic_cast<gridBus *> (obj))
    {
      setRemoteBus (obj);
    }
  else
    {
	  throw(invalidObjectException(this));
    }
}

void gridDynGenerator::add (gridSubModel *obj)
{

  if (dynamic_cast<gridDynExciter *> (obj))
    {
      ext = static_cast<gridDynExciter *> (replaceSubObject (obj, ext, exciter_loc));
    }
  else if (dynamic_cast<gridDynGenModel *> (obj))
    {
      genModel = static_cast<gridDynGenModel *> (replaceSubObject (obj, genModel, genmodel_loc));
      if (m_Rs != 0.0)
        {
          obj->set ("rs", m_Rs);
        }
      if (m_Xs != 1.0)
        {
          obj->set ("xs", m_Xs);
        }
    }
  else if (dynamic_cast<gridDynGovernor *> (obj))
    {
      gov = static_cast<gridDynGovernor *> (replaceSubObject (obj, gov, governor_loc));
      //mesh up the Pmax and Pmin giving priority to the new gov
      double govpmax = gov->get ("pmax");
      double govpmin = gov->get ("pmin");
      if (govpmax < kHalfBigNum)
        {
          Pmax = govpmax * machineBasePower / systemBasePower;
          Pmin = govpmin * machineBasePower / systemBasePower;
        }
      else
        {
          gov->set ("pmax", Pmax * systemBasePower / machineBasePower);
          gov->set ("pmin", Pmin * systemBasePower / machineBasePower);
        }

    }
  else if (dynamic_cast<gridDynPSS *> (obj))
    {
      pss = static_cast<gridDynPSS *> (replaceSubObject (obj, pss, pss_loc));
    }

  else
    {
	  throw(invalidObjectException(this));
    }
  if (opFlags[dyn_initialized])
    {
      alert (this, STATE_COUNT_CHANGE);
    }

}

gridSubModel *gridDynGenerator::replaceSubObject (gridSubModel *newObject, gridSubModel *oldObject, index_t newIndex)
{
  if (oldObject)
    {
      if (newObject->getID () == oldObject->getID ())
        {
          return newObject;
        }
      else
        {
          for (auto subit = subObjectList.begin (); subit != subObjectList.end (); ++subit)
            {
              if ((*subit)->getID () == oldObject->getID ())
                {
                  subObjectList.erase (subit);
                  break;
                }
            }
          condDelete (oldObject, this);
        }
    }
  newObject->setParent (this);
  newObject->set ("basepower", machineBasePower);
  newObject->set("basefreq", m_baseFreq);
  newObject->locIndex = newIndex;
  subObjectList.push_back (newObject);
  if (opFlags[dyn_initialized])
    {
      offsets.unload (true);
      parent->alert (this, OBJECT_COUNT_CHANGE);
    }
  return newObject;
}


void gridDynGenerator::setRemoteBus (gridCoreObject *newRemoteBus)
{
  gridBus* newRbus = dynamic_cast<gridBus *> (newRemoteBus);
  if (!newRbus)
    {
      return;
    }
  if ((remoteBus) && (newRbus->getID () == remoteBus->getID ()))
    {
      return;
    }
  auto prevRbus = remoteBus;
  remoteBus = newRbus;
  //update the flags as appropriate
  if (remoteBus->getID () != parent->getID ())
    {
      opFlags.set (remote_voltage_control);
      opFlags.reset (local_voltage_control);
      opFlags.set (has_powerflow_adjustments);
    }
  else
    {
      opFlags.reset (remote_voltage_control);
      opFlags.set (local_voltage_control);
      opFlags.reset (has_powerflow_adjustments);
    }
  if (opFlags[pFlow_initialized])
    {
      if (opFlags[adjustable_Q])
        {
          remoteBus->registerVoltageControl (this);
          if (prevRbus)
            {
              prevRbus->removeVoltageControl (this);
            }
        }
      if (opFlags[adjustable_P])
        {
          remoteBus->registerPowerControl (this);
          if (prevRbus)
            {
              prevRbus->removePowerControl (this);
            }
        }
    }
}
// set properties
void gridDynGenerator::set (const std::string &param,  const std::string &val)
{

  if (param == "dynmodel")
    {
      auto dmodel = dynModelFromString (val);
      if (dmodel == dynModel_t::invalid)
        {
		  throw(invalidParameterValue());
        }
      buildDynModel (dmodel);
    }
  else if (param == "remote")
    {
      gridCoreObject *root = parent->find ("root");
      setRemoteBus (locateObject (val, root, false));
    }
  else if (param == "remote_power_control")
    {
      opFlags.set (remote_power_control);
      opFlags.reset (local_power_control);
    }
  else if (param == "p")
    {
      if (val == "max")
        {
          P = Pmax;
        }
      else if (val == "min")
        {
          P = Pmin;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else if (param == "q")
    {
      if (val == "max")
        {
          Q = Qmax;
        }
      else if (val == "min")
        {
          Q = Qmin;
        }
      else
        {
		  throw(invalidParameterValue());
        }
    }
  else
    {
	  try
	  {
		  gridSecondary::set(param, val);
	  }
	  catch (gridDynException &)
	  {
		  for (auto subobj : subObjectList)
		  {
			  subobj->setFlag("no_gridobject_set");
			  try
			  {
				  subobj->set(param, val);
				  subobj->setFlag("no_gridobject_set", false);
				  break;
			  }
			  catch (gridDynException &)
			  {
				  subobj->setFlag("no_gridobject_set", false);
			  }
			  

		  }
	  }
    }

}

double gridDynGenerator::get (const std::string &param, units_t unitType) const
{
  double ret = kNullVal;
  if (param == "vcontrolfrac")
    {
      ret = vRegFraction;
    }
  else if (param == "vtarget")
    {
      ret = m_Vtarget;
    }
  else if (param == "participation")
    {
      ret = participation;
    }
  else if (param == "pmax")
    {
      ret = unitConversion (getPmax (),puMW,unitType,systemBasePower);
    }
  else if (param == "pmin")
    {
      ret = unitConversion (getPmin (), puMW, unitType, systemBasePower);
    }
  else if (param == "qmax")
    {
      ret = unitConversion (getQmax (), puMW, unitType, systemBasePower);
    }
  else if (param == "qmin")
    {
      ret = unitConversion (getQmin (), puMW, unitType, systemBasePower);
    }
  else if (param == "pset")
    {
      ret = unitConversion (getPset (), puMW, unitType, systemBasePower);
    }
  else
    {
      ret = gridSecondary::get (param, unitType);
    }
  return ret;
}

void gridDynGenerator::timestep (double ttime, const IOdata &args, const solverMode &sMode)
{
  if (Pset < -kHalfBigNum)
    {
      Pset = P;
    }
  Pset = Pset + dPdt * (ttime - prevTime);
  Pset = (Pset > Pmax) ? Pmax : ((Pset < Pmin) ? Pmin : Pset);
  double scale = machineBasePower / systemBasePower;
  if (!isDynamic (sMode))
    {

      P = Pset;
      Q = Q + dQdt * (ttime - prevTime);
      Q = (Q > Qmax) ? Qmax : ((Q < Qmin) ? Qmin : Q);
      if (args[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              P = P * (args[voltageInLocation] / 0.8);
              Q = Q * (args[voltageInLocation] / 0.8);
            }
        }
    }
  else
    {

      double omega = genModel->getFreq ((stateData *)(nullptr), cLocalSolverMode);

      if ((gov) && (gov->enabled))
        {
          gov->timestep (ttime, { omega, Pset / scale }, sMode);
		  m_Pmech = gov->getOutput();
        }

      if ((ext) && (ext->enabled))
        {
          ext->timestep (ttime, { args[voltageInLocation], args[angleInLocation], m_Pmech, omega }, sMode);
		  m_Eft = ext->getOutput();
        }

      if ((pss) && (pss->enabled))
        {
          pss->timestep (ttime, args, sMode);
        }
      //compute the residuals


      genModel->timestep (ttime, { args[voltageInLocation], args[angleInLocation], m_Eft, m_Pmech }, sMode);
      auto vals = genModel->getOutputs ({ args[voltageInLocation], args[angleInLocation], m_Eft, m_Pmech }, nullptr, cLocalSolverMode);
      P = vals[PoutLocation] * scale;
      Q = vals[QoutLocation] * scale;


    }
  //use this as the temporary state storage
  prevTime = ttime;
}

void gridDynGenerator::algebraicUpdate (const IOdata &args, const stateData *sD, double update[],const solverMode &sMode, double alpha)
{
  generateSubModelInputs (args, sD, sMode);
  //TODO::PT allow algebraic update for the other subModels (none currently have algebraic states though governors soon might)
  if ((sD) && (!isLocal (sMode)))
    {
      genModel->algebraicUpdate (subInputs.genModelInputs, sD, update, sMode, alpha);
    }
  else
    {
      stateData sD2;
      sD2.state = m_state.data ();
      genModel->algebraicUpdate (subInputs.genModelInputs, &sD2, m_state.data (), cLocalbSolverMode, alpha);

    }
}


change_code gridDynGenerator::powerFlowAdjust (const IOdata & /*args*/, unsigned long /*flags*/, check_level_t /*level*/)
{
  if (opFlags[at_limit])
    {
      double V = remoteBus->getVoltage ();
      if (Q == Qmax)
        {
          if (V < m_Vtarget)
            {
              opFlags.reset (at_limit);
              return change_code::parameter_change;
            }
        }
      else if (V > m_Vtarget)
        {
          opFlags.reset (at_limit);
          return change_code::parameter_change;
        }
    }
  else
    {
      if (Q > Qmax)
        {
          opFlags.set (at_limit);
          Q = Qmax;
          return change_code::parameter_change;
        }
      else if (Q < Qmin)
        {
          opFlags.set (at_limit);
          Q = Qmin;
          return change_code::parameter_change;
        }
    }
  return change_code::no_change;
}

void gridDynGenerator::powerAdjust (double adjustment)
{
  P = P + adjustment;
  Pset = Pset + adjustment;
  if (P > Pmax)
    {
      P = Pmax;
      Pset = Pmax;
    }
  else if (P < Pmin)
    {
      P = Pmin;
      Pset = Pmin;
    }
}


static std::map<int, int> alertFlags {
  std::make_pair (FLAG_CHANGE, 1),
  std::make_pair (STATE_COUNT_INCREASE, 3),
  std::make_pair (STATE_COUNT_DECREASE, 3),
  std::make_pair (STATE_COUNT_CHANGE, 3),
  std::make_pair (ROOT_COUNT_INCREASE, 2),
  std::make_pair (ROOT_COUNT_DECREASE, 2),
  std::make_pair (ROOT_COUNT_CHANGE, 2),
  std::make_pair (OBJECT_COUNT_INCREASE, 2),
  std::make_pair (OBJECT_COUNT_DECREASE, 2),
  std::make_pair (OBJECT_COUNT_CHANGE, 2),
  std::make_pair (JAC_COUNT_INCREASE, 4),
  std::make_pair (JAC_COUNT_DECREASE, 4),
  std::make_pair (JAC_COUNT_CHANGE, 4),
  std::make_pair (CONSTRAINT_COUNT_DECREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_INCREASE, 1),
  std::make_pair (CONSTRAINT_COUNT_CHANGE, 1),
};

void gridDynGenerator::alert (gridCoreObject *object, int code)
{
  if ((code >= MIN_CHANGE_ALERT)&& (code < MAX_CHANGE_ALERT))
    {
      auto res = alertFlags.find (code);
      if (res != alertFlags.end ())
        {
          int flagNum = res->second;
          updateFlags ();
          if (flagNum == 3)
            {
              offsets.unload ();
            }
          else if (flagNum == 2)
            {
              offsets.rjUnload (true);
            }
          else if (flagNum == 4)
            {
              offsets.rjUnload (false);
            }

        }
    }
  if (parent)
    {
      parent->alert (object, code);
    }
}

void gridDynGenerator::setFlag (const std::string &flag, bool val)
{

  if (flag == "capabiltycurve")
    {
      opFlags.set (use_capability_curve,val);
    }
  else if ((flag == "variable")||(flag == "variablegen"))
    {
      opFlags.set (variable_generation, val);
    }
  else if ((flag == "reserve")||(flag == "reservecapable"))
    {
      opFlags.set (reserve_capable, val);
    }
  else if ((flag == "agc")||(flag == "agccapble"))
    {
      opFlags.set (agc_capable, val);
    }
  else if (flag == "indirect_voltage_control")
    {
      opFlags.set (indirect_voltage_control,val);
    }
  else
    {
      gridSecondary::setFlag (flag,val);
    }
}

void gridDynGenerator::set (const std::string &param, double val,units_t unitType)
{
  if (param.length () == 1)
    {
      switch (param[0])
        {
        case 'p':
          P = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
          break;
        case 'q':
          Q = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
		  break;
        case 'r':
          m_Rs = val;
          if (genModel)
            {
              genModel->set (param, val,unitType);
            }
          break;
        case 'x':
          m_Xs = val;
          if (genModel)
            {
              genModel->set (param, val,unitType);
            }
          break;
        case 'h':
        case 'm':
        case 'd':
          if (genModel)
            {
              genModel->set (param, val,unitType);
            }
          else
            {
			  throw(unrecognizedParameter());
            }
          break;
        default:
			throw(unrecognizedParameter());
        }
	  return;
    }

  if (param == "pset")
    {
      Pset = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "qmax")
    {
      Qmax = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "qmin")
    {
      Qmin = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "adjustment")
    {
      powerAdjust (val);
    }
  else if (param == "xs")
    {
      m_Xs = val;
      if (genModel)
        {
          genModel->set ("xs", val);
        }
    }
  else if (param == "rs")
    {
      m_Rs = val;
      if (genModel)
        {
          genModel->set ("rs", val);
        }
    }
  else if (param == "eft")
    {
      m_Eft = val;
    }
  else if (param == "vref")
    {
      if (ext)
        {
          ext->set (param, val);
        }
      else
        {
          m_Vtarget = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
        }
    }
  else if ((param == "rating") || (param == "base") || (param == "mbase"))
    {
      machineBasePower = unitConversion (val, unitType, MVAR, systemBasePower, baseVoltage);
      opFlags.set (independent_machine_base);
      if (genModel)
        {
          genModel->set ("base", machineBasePower);
        }
    }
  else if (param == "dpdt")
    {
      dPdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "dqdt")
    {
      dQdt = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "basepower")
    {
      systemBasePower = unitConversion (val, unitType, gridUnits::MW);
      if (opFlags[independent_machine_base])
        {

        }
      else
        {
          machineBasePower = systemBasePower;
          for (auto &so : subObjectList)
            {
              so->set ("basepower", machineBasePower);
            }

        }
    }
  else if (param == "basevoltage")
    {
      baseVoltage = unitConversion (val, unitType, gridUnits::kV);

    }
  else if ((param == "basefrequency") || (param == "basefreq"))
    {
      m_baseFreq = unitConversionFreq (val, unitType, rps);
      if (genModel)
        {
          genModel->set (param, m_baseFreq);
        }
      if (gov)
        {
          gov->set (param, m_baseFreq);
        }
    }
  else if (param == "participation")
    {
      participation = val;
    }
  else if ((param == "vcontrolfrac")||(param == "vregfraction") || (param == "vcfrac"))
    {
      vRegFraction = val;

    }
  else if (param == "pmax")
    {
      Pmax  = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (gov)
        {
          gov->set (param, Pmax * systemBasePower / machineBasePower);
        }

    }
  else if (param == "pmin")
    {
      Pmin = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
      if (gov)
        {
          gov->set ("pmin", Pmin * systemBasePower / machineBasePower);
        }
    }
  else if (param == "vtarget")
    {
      m_Vtarget = unitConversion (val, unitType, puV, systemBasePower, baseVoltage);
    }
  else if (param == "capabiltycurve")
    {
      opFlags.set (use_capability_curve,(val > 0.00001));
    }
  else if (param == "remote")
    {
      gridCoreObject *root = parent->find ("root");
      setRemoteBus (root->findByUserID ("bus", static_cast<index_t> (val)));
    }
  else
  {
		  if (sched)
		  {
			  sched->setFlag("no_gridobject_set");
			  try 
			  {
				  sched->set(param, val, unitType);
				  sched->setFlag("no_gridobject_set", false);
				  return;
			  }
			  catch (const unrecognizedParameter &)
			  {
				  sched->setFlag("no_gridobject_set", false);
				  //Yes I am ignoring the exception
			  }
			  
		  }
		  for (auto subobj : subObjectList)
	      {
				  subobj->setFlag("no_gridobject_set");
				  try
				  {
					  subobj->set(param, val, unitType);
					  subobj->setFlag("no_gridobject_set", false);
					  return;
				  }
				  catch (const unrecognizedParameter &)
				  {
					  subobj->setFlag("no_gridobject_set", false);
				  }
				  
		  }
		  
		  gridSecondary::set(param, val, unitType);
	  }

  }

void gridDynGenerator::setCapabilityCurve (std::vector<double> Ppts, std::vector<double> Qminpts, std::vector<double> Qmaxpts)
{
  if ((Ppts.size () == Qminpts.size ())&&(Ppts.size () == Qmaxpts.size ()))
    {
      PC = Ppts;
      minQPC = Qminpts;
      maxQPC = Qmaxpts;
      opFlags.set (use_capability_curve);
    }
}

void gridDynGenerator::updateFlags (bool dynOnly)
{

  gridObject::updateFlags (dynOnly);
  loadSizes (cLocalSolverMode,true);
}





void gridDynGenerator::outputPartialDerivatives (const IOdata & /*args*/, const stateData *sD, matrixData<double> *ad, const solverMode &sMode)
{
  double scale = machineBasePower / systemBasePower;
  if (!isDynamic (sMode))
    { //the bus is managing a remote bus voltage
      if (stateSize (sMode) > 0)
        {
          auto offset = offsets.getAlgOffset (sMode);
          ad->assign (QoutLocation,offset,-scale);
        }
      return;
    }

  matrixDataSparse<double> d;

  //compute the Jacobians


  genModel->outputPartialDerivatives (subInputs.genModelInputs, sD,&d, sMode);
  //only valid locations are the generator internal coupled states
  genModel->ioPartialDerivatives (subInputs.genModelInputs,sD,&d,subInputLocs.genModelInputLocsInternal,sMode);
  d.scale (scale);
  ad->merge (&d);
  d.clear ();

}

void gridDynGenerator::ioPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  double scale = machineBasePower / systemBasePower;
  if  (isDynamic (sMode))
    {
      matrixDataSparse<double> d;
      auto gmLocs = subInputLocs.genModelInputLocsExternal;
      gmLocs[voltageInLocation] = argLocs[voltageInLocation];
      gmLocs[angleInLocation] = argLocs[angleInLocation];
      genModel->ioPartialDerivatives (subInputs.genModelInputs, sD,&d, gmLocs,sMode);
      ad->merge (&d, scale);
    }
  else
    {
      if (args[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              ad->assignCheckCol (PoutLocation,argLocs[voltageInLocation], -P / 0.8);
              ad->assignCheckCol (QoutLocation, argLocs[voltageInLocation], -Q / 0.8);
            }
        }
    }

}

IOdata gridDynGenerator::getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode)
{
  generateSubModelInputs (args, sD, sMode);
  double scale = machineBasePower / systemBasePower;
  IOdata output = { -P / scale, -Q / scale };
  if (isDynamic (sMode))       //use as a proxy for dynamic state
    {
      output = genModel->getOutputs (subInputs.genModelInputs, sD, sMode);
    }
  else
    {
      if (opFlags[indirect_voltage_control])
        {
          auto offset = offsets.getAlgOffset (sMode);
          output[QoutLocation] = -sD->state[offset] / scale;
          if (args[voltageInLocation] < 0.8)
            {
              if (!opFlags[no_voltage_derate])
                {
                  output[PoutLocation] *= args[voltageInLocation] / 0.8;
                }
            }
        }
      else if (args[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output[PoutLocation] *= args[voltageInLocation] / 0.8;
              output[QoutLocation] *= args[voltageInLocation] / 0.8;
            }
        }
    }
  output[PoutLocation] *= scale;
  output[QoutLocation] *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), args[voltageInLocation], args[angleInLocation], output[PoutLocation]);
  return output;
}

double gridDynGenerator::getRealPower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{

  double scale = machineBasePower / systemBasePower;
  double output = -P / scale;
  if (isDynamic (sMode))            //use as a proxy for dynamic state
    {
      generateSubModelInputs (args, sD, sMode);
      output = genModel->getOutput (subInputs.genModelInputs, sD, sMode,0);
    }
  else
    {
      if (opFlags[indirect_voltage_control])
        {
          if (args[voltageInLocation] < 0.8)
            {
              if (!opFlags[no_voltage_derate])
                {
                  output *= args[voltageInLocation] / 0.8;
                }
            }
        }
      else if (args[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output *= args[voltageInLocation] / 0.8;
            }
        }
    }
  output *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), args[voltageInLocation], args[angleInLocation], output[PoutLocation]);
  return output;
}
double gridDynGenerator::getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode)
{

  double scale = machineBasePower / systemBasePower;
  double output =  -Q / scale;
  if (isDynamic (sMode))            //use as a proxy for dynamic state
    {
      generateSubModelInputs (args, sD, sMode);
      output = genModel->getOutput (subInputs.genModelInputs, sD, sMode,1);
    }
  else
    {
      if (opFlags[indirect_voltage_control])
        {
          auto offset = offsets.getAlgOffset (sMode);
          output = -sD->state[offset] / scale;
        }
      else if (args[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output *= args[voltageInLocation] / 0.8;
            }
        }
    }
  output *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), args[voltageInLocation], args[angleInLocation], output[PoutLocation]);
  return output;
}
double gridDynGenerator::getRealPower () const
{
  return -P;
}
double gridDynGenerator::getReactivePower () const
{
  return -Q;
}


// compute the residual for the dynamic states
void gridDynGenerator::residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
  if ((!isDynamic (sMode))&&(opFlags[indirect_voltage_control]))
    { //the bus is managing a remote bus voltage
      double V = remoteBus->getVoltage (sD, sMode);
      auto offset = offsets.getAlgOffset (sMode);
      // printf("Q=%f\n",sD->state[offset]);
      if (!opFlags[at_limit])
        {
          resid[offset] = sD->state[offset] - (V - m_Vtarget) * vRegFraction * 10000;
        }
      else
        {
          resid[offset] = sD->state[offset] + Q;
        }
      return;
    }
  if ((isDynamic (sMode))||(opFlags[has_pflow_states]))
    {
      //compute the residuals
      generateSubModelInputs (args, sD, sMode);
      genModel->residual (subInputs.genModelInputs, sD, resid, sMode);

      if ((ext) && (ext->enabled))
        {
          ext->residual (subInputs.exciterInputs, sD, resid, sMode);
        }
      if ((gov) && (gov->enabled))
        {
          gov->residual (subInputs.governorInputs, sD, resid, sMode);
        }

      if ((pss) && (pss->enabled))
        {
          pss->residual (subInputs.pssInputs, sD, resid, sMode);
        }
    }
}

void gridDynGenerator::derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode)
{

  generateSubModelInputs (args, sD, sMode);
  //compute the residuals

  genModel->derivative (subInputs.genModelInputs, sD, deriv, sMode);

  if ((ext) && (ext->enabled))
    {
      ext->derivative (subInputs.exciterInputs, sD, deriv,sMode);
    }

  if ((gov) && (gov->enabled))
    {
      gov->residual ( subInputs.governorInputs, sD, deriv, sMode);
    }


  if ((pss) && (pss->enabled))
    {
      pss->residual (subInputs.pssInputs, sD, deriv, sMode);
    }
}

void gridDynGenerator::jacobianElements (const IOdata &args,const stateData *sD,
                                         matrixData<double> *ad,
                                         const IOlocs &argLocs,const solverMode &sMode)
{
  if  ((!isDynamic (sMode)) && (opFlags[indirect_voltage_control]))
    { //the bus is managing a remote bus voltage
      auto Voff = remoteBus->getOutputLoc (sMode,voltageInLocation);
      auto offset = offsets.getAlgOffset (sMode);
      if (!opFlags[at_limit])
        {
          //resid[offset] = sD->state[offset] - (V - m_Vtarget)*remoteVRegFraction * 10000;
          ad->assignCheck (offset, offset, 1);
          ad->assignCheck (offset,Voff,-vRegFraction * 10000);
        }
      else
        {
          ad->assignCheck (offset, offset, 1.0);
        }

      return;
    }
  if ((!isDynamic (sMode)) && (!opFlags[has_pflow_states]))
    {
      return;
    }
  generateSubModelInputs (args, sD, sMode);
  generateSubModelInputLocs (argLocs, sD, sMode);

  //compute the Jacobians

  genModel->jacobianElements ( subInputs.genModelInputs, sD, ad, subInputLocs.genModelInputLocsAll, sMode);
  if ((ext) && (ext->enabled))
    {
      ext->jacobianElements (subInputs.exciterInputs, sD, ad,  subInputLocs.exciterInputLocs, sMode);
    }
  if ((gov) && (gov->enabled))
    {
      gov->jacobianElements ( subInputs.governorInputs, sD,  ad,  subInputLocs.governorInputLocs, sMode);
    }
  if ((pss) && (pss->enabled))
    {
      pss->jacobianElements ( subInputs.pssInputs, sD, ad,  subInputLocs.pssInputLocs, sMode);
    }
}

void gridDynGenerator::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{

  std::string prefix2 = prefix + name;
  if ((!isDynamic (sMode))&&(stateSize (sMode) > 0))
    {
      auto offset = offsets.getAlgOffset (sMode);
      stNames[offset] = prefix2 + ":Q";
      return;
    }

  prefix2 += "::";
  for (auto &so : subObjectList)
    {
      if (so->enabled)
        {
          so->getStateName (stNames, sMode, prefix2);
        }
    }
}

void gridDynGenerator::rootTest (const IOdata &args, const stateData *sD, double root[], const solverMode &sMode)
{
  generateSubModelInputs (args, sD, sMode);
  if (genModel->checkFlag (has_roots))
    {
      genModel->rootTest (subInputs.genModelInputs, sD, root, sMode);
    }
  if ((ext) && (ext->checkFlag (has_roots)))
    {
      ext->rootTest ( subInputs.exciterInputs, sD, root,  sMode);
    }
  if ((gov) && (gov->checkFlag (has_roots)))
    {
      gov->rootTest ( subInputs.governorInputs, sD, root,  sMode);
    }
  if ((pss) && (pss->checkFlag (has_roots)))
    {
      pss->rootTest (subInputs.pssInputs, sD, root, sMode);
    }
}

change_code gridDynGenerator::rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level)
{
  auto ret = change_code::no_change;
  generateSubModelInputs (args, sD, sMode);

  if (genModel->checkFlag (has_alg_roots))
    {
      auto ret2 = genModel->rootCheck (subInputs.genModelInputs, sD, sMode, level);
      if (ret2 > ret)
        {
          ret = ret2;
        }
    }

  if ((ext) && (ext->checkFlag (has_alg_roots)))
    {
      auto ret2 = ext->rootCheck (subInputs.exciterInputs, sD, sMode, level);
      if (ret2 > ret)
        {
          ret = ret2;
        }
    }
  if ((gov) && (gov->checkFlag (has_alg_roots)))
    {

      auto ret2 = gov->rootCheck (subInputs.governorInputs,sD, sMode,level);
      if (ret2 > ret)
        {
          ret = ret2;
        }

    }
  if ((pss) && (pss->checkFlag (has_alg_roots)))
    {
      auto ret2 = pss->rootCheck (subInputs.pssInputs,sD,sMode,level);
      if (ret2 > ret)
        {
          ret = ret2;
        }
    }
  return ret;
}
void gridDynGenerator::rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  if (genModel->checkFlag (has_roots))
    {
      genModel->rootTrigger (ttime, args, rootMask, sMode);
    }
  if ((ext) && (ext->checkFlag (has_roots)))
    {
      ext->rootTrigger (ttime, args, rootMask, sMode);
    }
  if ((gov) && (gov->checkFlag (has_roots)))
    {
      double omega = genModel->getFreq (nullptr, cLocalSolverMode);
      gov->rootTrigger (ttime, { omega, Pset }, rootMask, sMode);

    }
  if ((pss) && (pss->checkFlag (has_roots)))
    {
      pss->rootTrigger (ttime, args, rootMask, sMode);

    }
}


index_t gridDynGenerator::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  for (auto &so : subObjectList)
    {
      ret = static_cast<gridSubModel *> (so)->findIndex (field,sMode);
      if (ret != kInvalidLocation)
        {
          break;
        }
    }
  return ret;
}

gridCoreObject *gridDynGenerator::find (const std::string &object) const
{
  if (object == "bus")
    {
      if (bus)
        {
          return bus;
        }
      else
        {
          return parent->find (object);
        }
    }
  if (object == "genmodel")
    {
      return genModel;
    }
  if (object == "exciter")
    {
      return ext;
    }
  if (object == "governor")
    {
      return gov;
    }

  if (object == "pss")
    {
      return pss;
    }
  if ((object == "generator") || (object == name))
    {
      return const_cast<gridDynGenerator *> (this);
    }
  else
    {
      if (parent)
        {
          return parent->find (object);
        }
    }
  return nullptr;
}

gridCoreObject *gridDynGenerator::getSubObject (const std::string &typeName, index_t num) const
{
  if ((typeName == "subobject")||(typeName == "submodel"))
    {
      if (static_cast<size_t> (num) < subObjectList.size ())
        {
          return subObjectList[num];
        }
    }
  else if (typeName == "submodelcode")
    {
      for (auto &sub:subObjectList)
        {
          if (sub->locIndex == num)
            {
              return sub;
            }
        }
    }
  return find (typeName);
}

double gridDynGenerator::getAdjustableCapacityUp (const double time) const
{
  if (sched)
    {
      return (sched->getMax (time) - Pset);
    }
  else
    {
      return Pmax - Pset;
    }
}

double gridDynGenerator::getAdjustableCapacityDown (double time) const
{
  if (sched)
    {
      return (Pset - sched->getMin (time));
    }
  else
    {
      return (Pset - Pmin);
    }
}

IOdata gridDynGenerator::predictOutputs (double ptime, const IOdata & /*args*/, const stateData *, const solverMode &)
{
  IOdata out (2);
  out[PoutLocation] = Pset;
  out[QoutLocation] = Q;

  if (ptime > prevTime + 1.00)
    {
      if (sched)
        {
          const double Ppred = sched->predict (ptime);
          out[PoutLocation] = Ppred;
        }

    }
  return out;
}

double gridDynGenerator::getPmax (const double time) const
{
  if (sched)
    {
      return sched->getMax (time);
    }
  else
    {
      return Pmax;
    }
}

double gridDynGenerator::getQmax (const double /*time*/, double /*Ptest*/) const
{
  if (opFlags[use_capability_curve])
    {
      return Qmax;
    }
  else
    {
      return Qmax;
    }
}

double gridDynGenerator::getPmin (const double time) const
{
  if (sched)
    {
      return sched->getMin (time);
    }
  else
    {
      return Pmin;
    }
}
double gridDynGenerator::getQmin (const double /*time*/, double /*Ptest*/) const
{
  if (opFlags[use_capability_curve])
    {
      return Qmin;
    }
  else
    {
      return Qmin;
    }
}


double gridDynGenerator::getFreq (const stateData *sD, const solverMode &sMode, index_t *freqOffset) const
{
   return  genModel->getFreq (sD, sMode,freqOffset);
}

double gridDynGenerator::getAngle(const stateData *sD, const solverMode &sMode, index_t *angleOffset) const
{
	return  genModel->getAngle(sD, sMode, angleOffset);
}

gridDynGenerator::subModelInputs::subModelInputs():genModelInputs(4),exciterInputs(3),governorInputs(3)
{
	
  }

gridDynGenerator::subModelInputLocs::subModelInputLocs() : genModelInputLocsAll(4), genModelInputLocsInternal(4), genModelInputLocsExternal(4),  exciterInputLocs(3), governorInputLocs(3)
{
	genModelInputLocsExternal[genModelEftInLocation] = kNullLocation;
	genModelInputLocsExternal[genModelPmechInLocation] = kNullLocation;
	genModelInputLocsInternal[voltageInLocation] = kNullLocation;
	genModelInputLocsInternal[angleInLocation] = kNullLocation;
}

void gridDynGenerator::generateSubModelInputs(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	if ((sD) && ((sD->seqID == subInputs.seqID) && (sD->seqID != 0)))
	{
		return;
	}
	if (args.empty())
	{
		auto out = bus->getOutputs(sD, sMode);
		subInputs.genModelInputs[voltageInLocation] = out[voltageInLocation];
		subInputs.genModelInputs[angleInLocation] = out[angleInLocation];
		subInputs.exciterInputs[exciterVoltageInLocation] = out[voltageInLocation];
		subInputs.governorInputs[govOmegaInLocation] = out[frequencyInLocation];
	}
	else
	{
		subInputs.genModelInputs[voltageInLocation] = args[voltageInLocation];
		subInputs.genModelInputs[angleInLocation] = args[angleInLocation];
		subInputs.exciterInputs[exciterVoltageInLocation] = args[voltageInLocation];
		if (args.size() > frequencyInLocation)
		{
			subInputs.governorInputs[govOmegaInLocation] = args[frequencyInLocation];
		}
		
	}
	if (!opFlags[uses_bus_frequency])
	{
		subInputs.governorInputs[govOmegaInLocation] = genModel->getFreq(sD, sMode);
	}
	

	double Pcontrol = pSetControlUpdate(args,sD,sMode);
	Pcontrol = valLimit(Pcontrol, Pmin, Pmax);
	subInputs.governorInputs[govpSetInLocation] = Pcontrol*systemBasePower/machineBasePower;

	subInputs.exciterInputs[exciterVsetInLocation] = vSetControlUpdate(args,sD,sMode);
	double Eft = m_Eft;
	if ((ext) && (ext->enabled))
	{
		Eft = ext->getOutput(subInputs.exciterInputs, sD, sMode, 0);
	}
	subInputs.genModelInputs[genModelEftInLocation] = Eft;
	double pmech = m_Pmech;
	if ((gov) && (gov->enabled))
	{
		pmech = gov->getOutput(subInputs.governorInputs, sD, sMode, 0);
	}

	subInputs.genModelInputs[genModelPmechInLocation] = pmech;
	if (sD)
	{
		subInputs.seqID = sD->seqID;
	}
	
}


void gridDynGenerator::generateSubModelInputLocs(const IOlocs &argLocs, const stateData *sD, const solverMode &sMode)
{
	if ((sD) && ((sD->seqID == subInputLocs.seqID) && (sD->seqID != 0)))
	{
		return;
	}
	
		subInputLocs.genModelInputLocsAll[voltageInLocation] = argLocs[voltageInLocation];
		subInputLocs.genModelInputLocsAll[angleInLocation] = argLocs[angleInLocation];
		subInputLocs.genModelInputLocsExternal[voltageInLocation] = argLocs[voltageInLocation];
		subInputLocs.genModelInputLocsExternal[angleInLocation] = argLocs[angleInLocation];

		if ((ext) && (ext->enabled))
		{
			subInputLocs.exciterInputLocs[exciterVoltageInLocation] = argLocs[voltageInLocation];
			subInputLocs.exciterInputLocs[exciterVsetInLocation] = vSetLocation(sMode);
			subInputLocs.genModelInputLocsAll[genModelEftInLocation]=ext->getOutputLoc( sMode, 0);
		}
		else
		{
			subInputLocs.genModelInputLocsAll[genModelEftInLocation] = kNullLocation;
		}
		subInputLocs.genModelInputLocsInternal[genModelEftInLocation] = subInputLocs.genModelInputLocsAll[genModelEftInLocation];
		if ((gov) && (gov->enabled))
		{
			if (genModel->checkFlag(uses_bus_frequency))
			{
				subInputLocs.governorInputLocs[govOmegaInLocation] = argLocs[frequencyInLocation];
			}
			else
			{
				genModel->getFreq(sD, sMode, &(subInputLocs.governorInputLocs[govOmegaInLocation]));
			}
			subInputLocs.governorInputLocs[govpSetInLocation] = pSetLocation(sMode);
			subInputLocs.genModelInputLocsAll[genModelPmechInLocation]=gov->getOutputLoc(sMode, 0);
		}
		else
		{
			subInputLocs.genModelInputLocsAll[genModelPmechInLocation] = kNullLocation;
		}
		subInputLocs.genModelInputLocsInternal[genModelPmechInLocation] = subInputLocs.genModelInputLocsAll[genModelPmechInLocation];
		subInputs.seqID = sD->seqID;
}

double gridDynGenerator::pSetControlUpdate(const IOdata & /*args*/, const stateData *sD, const solverMode &)
{
	double val = (sD) ? (Pset + dPdt * (sD->time - prevTime)) : Pset;
	return val;
}

double gridDynGenerator::vSetControlUpdate(const IOdata & /*args*/, const stateData *, const solverMode &)
{
	return 1.0;
}

index_t gridDynGenerator::pSetLocation(const solverMode &)
{
	return kNullLocation;
}
index_t gridDynGenerator::vSetLocation(const solverMode &)
{
	return kNullLocation;
}