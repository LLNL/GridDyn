/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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


#include "generators/gridDynGenerator.h"
#include "core/coreObjectTemplates.h"
#include "submodels/gridDynExciter.h"
#include "submodels/gridDynPSS.h"
#include "submodels/otherGenModels.h"
#include "submodels/otherGovernors.h"
#include "isocController.h"
#include "core/objectFactoryTemplates.h"
#include "utilities/vectorOps.hpp"
#include "core/objectInterpreter.h"
#include "controllers/scheduler.h"
#include "gridBus.h"
#include "utilities/stringOps.h"
#include "variableGenerator.h"
#include "core/coreExceptions.h"
#include "utilities/matrixDataSparse.h"

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

gridDynGenerator::gridDynGenerator (const std::string &objName) : gridSecondary (objName)
{
  setUserID(++genCount);
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
coreObject *gridDynGenerator::clone (coreObject *obj) const
{
  gridDynGenerator *gen = cloneBaseFactory<gridDynGenerator, gridSecondary> (this, obj,&gf);
  if (!(gen))
    {
      return obj;
    }

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
void gridDynGenerator::pFlowObjectInitializeA (coreTime time0, unsigned long /*flags*/)
{

  if (isConnected())
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

void gridDynGenerator::dynObjectInitializeA (coreTime time0, unsigned long flags)
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
  if (opFlags[isochronous_operation])
  {
	  bus->setFlag("compute_frequency",true);
	  //opFlags.set(uses_bus_frequency);
  }
  gridSecondary::dynObjectInitializeA (time0, flags);

}

void gridDynGenerator::loadSizes (const solverMode &sMode, bool dynOnly)
{

  auto soff = offsets.getOffsets (sMode);
  if (!isConnected())
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
          soff->total.jacSize = offsets.local().local.jacSize;
          soff->rjLoaded = true;
          return;
        }

      else
        {
		  auto &lc = offsets.local();
          soff->total.jacSize = lc.local.jacSize;
          soff->total.algRoots = lc.local.algRoots;
          soff->total.diffRoots = lc.local.diffRoots;
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
      for (auto &so : getSubObjects())
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
void gridDynGenerator::dynObjectInitializeB (const IOdata & inputs, const IOdata & desiredOutput, IOdata &fieldSet)
{
  double V = inputs[voltageInLocation];
  double theta = inputs[angleInLocation];


  if (desiredOutput.empty ())
    {
    }
  else
    {
      if (desiredOutput[0] > -100000)
        {
          P = desiredOutput[0];
        }

      if (desiredOutput[1] > -100000)
        {
          Q = desiredOutput[1];
        }
    }
  if (std::abs (P) > 1.2 * machineBasePower)
    {
      LOG_WARNING ("Requested Power output significantly greater than internal base power, may cause dynamic model instability, suggest updating base power");
    }
  //load the power set point
  if (opFlags[isochronous_operation])
  {
	  if (Pset > -kHalfBigNum)
	  {
		  isoc->setLevel(P - Pset);
		  isoc->setFreq(0.0);
	  }
	  else
	  {
		  isoc->setLevel(0.0);
		  isoc->setFreq(0.0);
		  Pset = P;
	  }
	 
  }
  else
  {
	  Pset = P;
  }
  
  double scale = systemBasePower / machineBasePower;
  IOdata inputArgs (4);
  IOdata localDesiredOutput (4);

  inputArgs[voltageInLocation] = V;
  inputArgs[angleInLocation] = theta;
  inputArgs[genModelPmechInLocation] = kNullVal;
  inputArgs[genModelEftInLocation] = kNullVal;

  localDesiredOutput[PoutLocation] = P * scale;
  localDesiredOutput[QoutLocation] = Q * scale;

  IOdata computedInputs (4);
  genModel->dynInitializeB (inputArgs, localDesiredOutput, computedInputs);
  m_Pmech = computedInputs[genModelPmechInLocation];

  m_Eft = computedInputs[genModelEftInLocation];
  genModel->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);

  Pset = m_Pmech / scale;
  if (isoc)
  {
	  Pset -= isoc->getOutput();
  }


  if ((ext) && (ext->isEnabled()))
    {
      inputArgs[voltageInLocation] = V;
      inputArgs[angleInLocation] = theta;
      inputArgs[exciterPmechInLocation] = m_Pmech;

	  localDesiredOutput[0] = m_Eft;
      ext->dynInitializeB (inputArgs, localDesiredOutput, computedInputs);

      ext->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
      //Vset=inputSetup[1];

    }
  if ((gov) && (gov->isEnabled()))
    {
      inputArgs[govOmegaInLocation] = m_baseFreq;
      inputArgs[govpSetInLocation] = kNullVal;

	  localDesiredOutput[0] = Pset * scale;
	  if (isoc)
	  {
		  localDesiredOutput[0] += isoc->getOutput()*scale;
	  }
      gov->dynInitializeB (inputArgs, localDesiredOutput, computedInputs);

      gov->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
    }

  if ((pss) && (pss->isEnabled()))
    {
      inputArgs[0] = m_baseFreq;
      inputArgs[1] = kNullVal;
	  localDesiredOutput[0] = 0;
      pss->dynInitializeB (inputArgs, desiredOutput, computedInputs);
      pss->guess (prevTime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);

    }

  inputArgs.resize(0);
  localDesiredOutput.resize(0);
  for (auto &sub: getSubObjects())
  {
	  if (sub->locIndex < 4)
	  {
		  continue;
	  }
	  if (sub->isEnabled())
	  {
		  sub->dynInitializeB(inputArgs, localDesiredOutput, computedInputs);
		  sub->guess(prevTime, m_state.data(), m_dstate_dt.data(), cLocalbSolverMode);
	  }
  }

  m_stateTemp = m_state.data ();
  m_dstate_dt_Temp = m_dstate_dt.data ();
  fieldSet.resize(2);
  fieldSet[PoutLocation] = P;
  fieldSet[QoutLocation] = Q;

}

// save an external state to the internal one
void gridDynGenerator::setState (coreTime ttime, const double state[], const double dstate_dt[],const solverMode &sMode)
{

  if (isDynamic (sMode))
    {
      for (auto &so : getSubObjects())
        {
          if (so->isEnabled())
            {
              so->setState (ttime, state, dstate_dt, sMode);
              so->guess (ttime, m_state.data (), m_dstate_dt.data (), cLocalbSolverMode);
            }
        }
      Pset += dPdt * (ttime - prevTime);
      Pset = valLimit (Pset, Pmin, Pmax);
    }
  else if (stateSize (sMode) > 0)
    {
      auto offset = offsets.getAlgOffset (sMode);
      Q = state[offset];
    }
  prevTime = ttime;
}


void gridDynGenerator::updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
	if ((isDynamic(sMode))&&(sD.updateRequired(subInputs.seqID)))
	{
		generateSubModelInputs(inputs, sD, sMode); //generate current input values
		for (auto &so : getSubObjects())
		{
			if (so->isEnabled())
			{
				so->updateLocalCache(subInputs.inputs[so->locIndex], sD, sMode);
			}
		}
		//generate updated input values which in many cases will be the same as before
		generateSubModelInputs(inputs, sD, sMode);
		double scale = machineBasePower / systemBasePower;
		P = -genModel->getOutput(subInputs.inputs[genmodel_loc], sD, sMode, PoutLocation)*scale;
		Q = -genModel->getOutput(subInputs.inputs[genmodel_loc], sD, sMode, QoutLocation)*scale;
	}
}

//copy the current state to a vector
void gridDynGenerator::guess (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      for (auto &so : getSubObjects())
        {
          if (so->isEnabled())
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


void gridDynGenerator::add (coreObject *obj)
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
	  throw(unrecognizedObjectException(this));
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
  else if (dynamic_cast<gridSource *>(obj))
  {
	  gridSource *src = static_cast<gridSource *>(obj);
	  if ((src->m_purpose == "power")|| (src->m_purpose == "pset"))
	  {
		  pSetControl = static_cast<gridSource *> (replaceSubObject(obj, pSetControl, pset_loc));
		  if (dynamic_cast<scheduler *>(pSetControl))
		  {
			  sched = static_cast<scheduler *>(pSetControl);
		  }
	  }
	  else if ((src->m_purpose == "voltage") || (src->m_purpose == "vset"))
	  {
		  vSetControl = static_cast<gridSource *> (replaceSubObject(obj, vSetControl, vset_loc));
	  }
	  else if ((pSetControl == nullptr) && (src->m_purpose.empty()))
	  {
		  pSetControl = static_cast<gridSource *> (replaceSubObject(obj, pSetControl, pset_loc));
	  }
	  else
	  {
		  throw(objectAddFailure(this));
	  }
  }
  else if (dynamic_cast<isocController *> (obj))
  {
	  isoc= static_cast<isocController *> (replaceSubObject(obj, isoc, isoc_control));
	  subInputLocs.inputLocs[isoc_control].resize(1);
	  subInputs.inputs[isoc_control].resize(1);
  }
  else
    {
	  throw(unrecognizedObjectException(this));
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
		  remove(oldObject);
        }
    }
 
  newObject->set("basefreq", m_baseFreq);
  newObject->locIndex = newIndex;
  addSubObject(newObject);
  if (opFlags[dyn_initialized])
    {
      offsets.unload (true);
      alert (this, OBJECT_COUNT_CHANGE);
    }
  if (newIndex >= subInputs.inputs.size())
  {
	  subInputs.inputs.resize(newIndex + 1);
	  subInputLocs.inputLocs.resize(newIndex + 1);
  }
  return newObject;
}


void gridDynGenerator::setRemoteBus (coreObject *newRemoteBus)
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
  if (isSameObject(remoteBus, getParent()))
  {
	  opFlags.reset(remote_voltage_control);
	  opFlags.set(local_voltage_control);
	  opFlags.reset(has_powerflow_adjustments);
  }
  else
  {
	  opFlags.set(remote_voltage_control);
	  opFlags.reset(local_voltage_control);
	  opFlags.set(has_powerflow_adjustments);
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
      coreObject *root = getRoot();
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
		  for (auto subobj : getSubObjects())
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
  else if (param == "pset")
  {
	  ret = unitConversion(getPmax(), puMW, unitType, systemBasePower);
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
  else
    {
      ret = gridSecondary::get (param, unitType);
    }
  return ret;
}

void gridDynGenerator::timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode)
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
      if (inputs[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              P = P * (inputs[voltageInLocation] / 0.8);
              Q = Q * (inputs[voltageInLocation] / 0.8);
            }
        }
    }
  else
    {

	  
      double omega = genModel->getFreq (emptyStateData, cLocalSolverMode);

      if ((gov) && (gov->isEnabled()))
        {
          gov->timestep (ttime, { omega, Pset / scale }, sMode);
		  m_Pmech = gov->getOutput();
        }

      if ((ext) && (ext->isEnabled()))
        {
          ext->timestep (ttime, { inputs[voltageInLocation], inputs[angleInLocation], m_Pmech, omega }, sMode);
		  m_Eft = ext->getOutput();
        }

      if ((pss) && (pss->isEnabled()))
        {
          pss->timestep (ttime, inputs, sMode);
        }
      //compute the residuals


      genModel->timestep (ttime, { inputs[voltageInLocation], inputs[angleInLocation], m_Eft, m_Pmech }, sMode);
      auto vals = genModel->getOutputs ({ inputs[voltageInLocation], inputs[angleInLocation], m_Eft, m_Pmech }, emptyStateData, cLocalSolverMode);
      P = vals[PoutLocation] * scale;
      Q = vals[QoutLocation] * scale;


    }
  //use this as the temporary state storage
  prevTime = ttime;
}

void gridDynGenerator::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[],const solverMode &sMode, double alpha)
{
  updateLocalCache (inputs, sD, sMode);
  
  if ((!sD.empty()) && (!isLocal (sMode)))
    {
	  for (auto &sub : getSubObjects())
	  {
		  if (sub->isEnabled())
		  {
			  static_cast<gridSubModel*>(sub)->algebraicUpdate(subInputs.inputs[sub->locIndex], sD, update, sMode,alpha);
		  }
	  }
    }
  else
    {
      stateData sD2(0.0, m_state.data());
	  for (auto &sub : getSubObjects())
	  {
		  if (sub->isEnabled())
		  {
			  static_cast<gridSubModel*>(sub)->algebraicUpdate(subInputs.inputs[sub->locIndex], sD2, m_state.data(), cLocalbSolverMode, alpha);
		  }
	  }

    }
}


change_code gridDynGenerator::powerFlowAdjust (const IOdata & /*inputs*/, unsigned long /*flags*/, check_level_t /*level*/)
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

void gridDynGenerator::alert (coreObject *object, int code)
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
  coreObject::alert(object, code);
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
	  opFlags.set(local_power_control, false);
	  opFlags.set(adjustable_P, false);
    }
  else if (flag == "no_control")
  {
	  opFlags.set(local_power_control, false);
	  opFlags.set(adjustable_P, false);
	  opFlags.set(adjustable_Q, false);
	  opFlags.set(remote_power_control, false);
	  opFlags.set(local_voltage_control, false);
	  opFlags.set(remote_voltage_control, false);
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
  else if ((flag == "isoc") || (flag == "isochronous"))
  {
	  opFlags.set(isochronous_operation, val);
	  if (val)
	  {
		  if (!isoc)
		  {
			  add(new isocController(getName()));
			  if (opFlags[dyn_initialized])
			  {
				  alert(isoc, UPDATE_REQUIRED);
			  }
		  }
		  else
		  {
			  isoc->activate(prevTime);
		  }
	  }
	  if (!val)
	  {
		  if (isoc)
		  {
			  isoc->deactivate();
		  }
	  }
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
  else if ((param == "p+")||(param == "adjustment"))
  {
	  powerAdjust(unitConversion(val, unitType, puMW, systemBasePower, baseVoltage));
  }
  else if (param == "qmax")
    {
      Qmax = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
    }
  else if (param == "qmin")
    {
      Qmin = unitConversion (val, unitType, puMW, systemBasePower, baseVoltage);
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
          ext->set (param, val,unitType);
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
          for (auto &so : getSubObjects())
            {
              so->set ("basepower", machineBasePower);
            }

        }
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
	  if (machineBasePower < 0)
	  {
		  machineBasePower = unitConversionPower(Pmax, puMW, MW, systemBasePower);
	  }
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
      coreObject *root = getRoot();
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
		  for (auto subobj : getSubObjects())
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





void gridDynGenerator::outputPartialDerivatives (const IOdata & /*inputs*/, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  double scale = machineBasePower / systemBasePower;
  if (!isDynamic (sMode))
    { //the bus is managing a remote bus voltage
      if (stateSize (sMode) > 0)
        {
          auto offset = offsets.getAlgOffset (sMode);
          ad.assign (QoutLocation,offset,-scale);
        }
      return;
    }

  matrixDataSparse<double> d;

  //compute the Jacobian


  genModel->outputPartialDerivatives (subInputs.inputs[genmodel_loc], sD,d, sMode);
  //only valid locations are the generator internal coupled states
  genModel->ioPartialDerivatives (subInputs.inputs[genmodel_loc],sD,d,subInputLocs.genModelInputLocsInternal,sMode);
  d.scale (scale);
  ad.merge (d);
  d.clear ();

}


count_t gridDynGenerator::outputDependencyCount(index_t num, const solverMode &sMode) const
{
	if (!isDynamic(sMode))
	{ //the bus is managing a remote bus voltage
		if (stateSize(sMode) > 0)
		{
			return (num == QoutLocation) ? 1 : 0;
		}
		return 0;
	}
	else
	{
		return 1 + genModel->outputDependencyCount(num, sMode);
	}
	
}

void gridDynGenerator::ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  double scale = machineBasePower / systemBasePower;
  if  (isDynamic (sMode))
    {
      matrixDataSparse<double> d;
      auto gmLocs = subInputLocs.genModelInputLocsExternal;
      gmLocs[voltageInLocation] = inputLocs[voltageInLocation];
      gmLocs[angleInLocation] = inputLocs[angleInLocation];
      genModel->ioPartialDerivatives (subInputs.inputs[genmodel_loc], sD,d, gmLocs,sMode);
      ad.merge (d, scale);
    }
  else
    {
      if (inputs[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              ad.assignCheckCol (PoutLocation,inputLocs[voltageInLocation], -P / 0.8);
              ad.assignCheckCol (QoutLocation, inputLocs[voltageInLocation], -Q / 0.8);
            }
        }
    }

}

IOdata gridDynGenerator::getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  double scale = machineBasePower / systemBasePower;
  IOdata output = { -P / scale, -Q / scale };
  if (isDynamic (sMode))       //use as a proxy for dynamic state
    {
      output = genModel->getOutputs (subInputs.inputs[genmodel_loc], sD, sMode);
    }
  else
    {
      if (opFlags[indirect_voltage_control])
        {
          auto offset = offsets.getAlgOffset (sMode);
          output[QoutLocation] = -sD.state[offset] / scale;
          if (inputs[voltageInLocation] < 0.8)
            {
              if (!opFlags[no_voltage_derate])
                {
                  output[PoutLocation] *= inputs[voltageInLocation] / 0.8;
                }
            }
        }
      else if (inputs[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output[PoutLocation] *= inputs[voltageInLocation] / 0.8;
              output[QoutLocation] *= inputs[voltageInLocation] / 0.8;
            }
        }
    }
  output[PoutLocation] *= scale;
  output[QoutLocation] *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), inputs[voltageInLocation], inputs[angleInLocation], output[PoutLocation]);
  return output;
}

double gridDynGenerator::getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{

  double scale = machineBasePower / systemBasePower;
  double output = -P / scale;
  if (isDynamic (sMode))            //use as a proxy for dynamic state
    {
      output = genModel->getOutput (subInputs.inputs[genmodel_loc], sD, sMode,0);
    }
  else
    {
      if (opFlags[indirect_voltage_control])
        {
          if (inputs[voltageInLocation] < 0.8)
            {
              if (!opFlags[no_voltage_derate])
                {
                  output *= inputs[voltageInLocation] / 0.8;
                }
            }
        }
      else if (inputs[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output *= inputs[voltageInLocation] / 0.8;
            }
        }
    }
  output *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), inputs[voltageInLocation], inputs[angleInLocation], output[PoutLocation]);
  return output;
}
double gridDynGenerator::getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{

  double scale = machineBasePower / systemBasePower;
  double output;
  if (isDynamic (sMode))            //use as a proxy for dynamic state
    {
      output = genModel->getOutput (subInputs.inputs[genmodel_loc], sD, sMode,1);
    }
  else
    {
	  output = -Q / scale;
      if (opFlags[indirect_voltage_control])
        {
          auto offset = offsets.getAlgOffset (sMode);
          output = -sD.state[offset] / scale;
        }
      else if (inputs[voltageInLocation] < 0.8)
        {
          if (!opFlags[no_voltage_derate])
            {
              output *= inputs[voltageInLocation] / 0.8;
            }
        }
    }
  output *= scale;
  //printf("t=%f (%s ) V=%f T=%f, P=%f\n", ttime, parent->name.c_str(), inputs[voltageInLocation], inputs[angleInLocation], output[PoutLocation]);
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
void gridDynGenerator::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  if ((!isDynamic (sMode))&&(opFlags[indirect_voltage_control]))
    { //the bus is managing a remote bus voltage
      double V = remoteBus->getVoltage (sD, sMode);
      auto offset = offsets.getAlgOffset (sMode);
      // printf("Q=%f\n",sD.state[offset]);
      if (!opFlags[at_limit])
        {
          resid[offset] = sD.state[offset] - (V - m_Vtarget) * vRegFraction * 10000;
        }
      else
        {
          resid[offset] = sD.state[offset] + Q;
        }
      return;
    }
  if ((isDynamic (sMode))||(opFlags[has_pflow_states]))
    {
      //compute the residuals
      updateLocalCache(inputs, sD, sMode);
	  for (auto &sub : getSubObjects())
	  {
		  if ((sub) && (sub->isEnabled()))
		  {
			  static_cast<gridSubModel*>(sub)->residual(subInputs.inputs[sub->locIndex], sD, resid, sMode);
		  }
	  }
    }
}

void gridDynGenerator::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{

	updateLocalCache(inputs, sD, sMode);
  //compute the residuals
  for (auto &sub : getSubObjects())
  {
	  if ((sub) && (sub->isEnabled()))
	  {
		  static_cast<gridSubModel*>(sub)->derivative(subInputs.inputs[sub->locIndex], sD, deriv, sMode);
	  }
  }
  
}

void gridDynGenerator::jacobianElements (const IOdata &inputs,const stateData &sD,
                                         matrixData<double> &ad,
                                         const IOlocs &inputLocs,const solverMode &sMode)
{
  if  ((!isDynamic (sMode)) && (opFlags[indirect_voltage_control]))
    { //the bus is managing a remote bus voltage
      auto Voff = remoteBus->getOutputLoc (sMode,voltageInLocation);
      auto offset = offsets.getAlgOffset (sMode);
      if (!opFlags[at_limit])
        {
          //resid[offset] = sD.state[offset] - (V - m_Vtarget)*remoteVRegFraction * 10000;
          ad.assignCheck (offset, offset, 1);
          ad.assignCheck (offset,Voff,-vRegFraction * 10000);
        }
      else
        {
          ad.assignCheck (offset, offset, 1.0);
        }

      return;
    }
  if ((!isDynamic (sMode)) && (!opFlags[has_pflow_states]))
    {
      return;
    }
  updateLocalCache(inputs, sD, sMode);
  generateSubModelInputLocs (inputLocs, sD, sMode);

  //compute the Jacobian
  for (auto &sub : getSubObjects())
  {
	  if ((sub) && (sub->isEnabled()))
	  {
		  sub->jacobianElements(subInputs.inputs[sub->locIndex], sD,ad,subInputLocs.inputLocs[sub->locIndex], sMode);
	  }
  }

}

void gridDynGenerator::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{

  std::string prefix2 = prefix + getName();
  if ((!isDynamic (sMode))&&(stateSize (sMode) > 0))
    {
      auto offset = offsets.getAlgOffset (sMode);
      stNames[offset] = prefix2 + ":Q";
      return;
    }

  prefix2 += "::";
  for (auto &so : getSubObjects())
    {
      if (so->isEnabled())
        {
          so->getStateName (stNames, sMode, prefix2);
        }
    }
}

void gridDynGenerator::rootTest (const IOdata &inputs, const stateData &sD, double root[], const solverMode &sMode)
{
	updateLocalCache(inputs, sD, sMode);

  for (auto &sub : getSubObjects())
  {
	  if ((sub) && (sub->checkFlag(has_roots)))
	  {
		  sub->rootTest(subInputs.inputs[sub->locIndex], sD, root,sMode);
	  }
  }
}

change_code gridDynGenerator::rootCheck ( const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
  auto ret = change_code::no_change;
  updateLocalCache(inputs, sD, sMode);

  for (auto &sub:getSubObjects())
  {
	  if ((sub) && (sub->checkFlag(has_alg_roots)))
	  {
		  auto ret2 = sub->rootCheck(subInputs.inputs[sub->locIndex], sD, sMode, level);
		  if (ret2 > ret)
		  {
			  ret = ret2;
		  }
	  }
  }
  
  return ret;
}
void gridDynGenerator::rootTrigger (coreTime ttime, const IOdata & /*inputs*/, const std::vector<int> &rootMask, const solverMode &sMode)
{
	for (auto &sub : getSubObjects())
	{
		if ((sub) && (sub->checkFlag(has_roots)))
		{
			sub->rootTrigger(ttime,subInputs.inputs[sub->locIndex], rootMask, sMode);
		}
	}

}


index_t gridDynGenerator::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  for (auto &so : getSubObjects())
    {
	  if (!so)
	  {
		  continue;
	  }
      ret = so->findIndex (field,sMode);
      if (ret != kInvalidLocation)
        {
          break;
        }
    }
  return ret;
}

coreObject *gridDynGenerator::find (const std::string &object) const
{
  if (object == "bus")
    {
	  if (!bus)
	  {
		return getParent()->find("bus");
	  }
       return bus;
    }
  if (object == "genmodel")
    {
      return genModel;
    }
  if (object == "exciter")
    {
      return ext;
    }
  if (object == "pset")
  {
	  return pSetControl;
  }
  if (object == "vset")
  {
	  return vSetControl;
  }
  if (object == "governor")
    {
      return gov;
    }
  if (object == "sched")
  {
	  return sched;
  }
  if (object == "pss")
    {
      return pss;
    }
  if ((object == "isoc") || (object == "isoccontrol"))
  {
	  return isoc;
  }
  if ((object == "generator") || (object == getName()))
    {
      return const_cast<gridDynGenerator *> (this);
    }
  else
    {
	  return gridObject::find(object);
    }
}

coreObject *gridDynGenerator::getSubObject (const std::string &typeName, index_t num) const
{
  if (typeName == "submodelcode")
    {
      for (auto &sub:getSubObjects())
        {
          if (sub->locIndex == num)
            {
              return sub;
            }
        }
	  return nullptr;
    }
  else
  {
	  return gridObject::getSubObject(typeName, num);
  }
}

double gridDynGenerator::getAdjustableCapacityUp (coreTime time) const
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

double gridDynGenerator::getAdjustableCapacityDown (coreTime time) const
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

IOdata gridDynGenerator::predictOutputs (coreTime predictionTime, const IOdata & /*inputs*/, const stateData &, const solverMode &) const
{
  IOdata out (2);
  out[PoutLocation] = Pset;
  out[QoutLocation] = Q;

  if (predictionTime > prevTime + timeOneSecond)
    {
      if (sched)
        {
          const double Ppred = sched->predict (predictionTime);
          out[PoutLocation] = Ppred;
        }

    }
  return out;
}

double gridDynGenerator::getPmax (const coreTime time) const
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

double gridDynGenerator::getQmax (const coreTime /*time*/, double /*Ptest*/) const
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

double gridDynGenerator::getPmin (const coreTime time) const
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
double gridDynGenerator::getQmin (const coreTime /*time*/, double /*Ptest*/) const
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


double gridDynGenerator::getFreq (const stateData &sD, const solverMode &sMode, index_t *freqOffset) const
{
   return  genModel->getFreq (sD, sMode,freqOffset);
}

double gridDynGenerator::getAngle(const stateData &sD, const solverMode &sMode, index_t *angleOffset) const
{
	return  genModel->getAngle(sD, sMode, angleOffset);
}

gridDynGenerator::subModelInputs::subModelInputs():inputs(6)
{
	inputs[genmodel_loc].resize(4);
	inputs[exciter_loc].resize(3);
	inputs[governor_loc].resize(3);
  }

gridDynGenerator::subModelInputLocs::subModelInputLocs() :  genModelInputLocsInternal(4), genModelInputLocsExternal(4), inputLocs(6)
{
	inputLocs[genmodel_loc].resize(4);
	inputLocs[exciter_loc].resize(3);
	inputLocs[governor_loc].resize(3);

	genModelInputLocsExternal[genModelEftInLocation] = kNullLocation;
	genModelInputLocsExternal[genModelPmechInLocation] = kNullLocation;
	genModelInputLocsInternal[voltageInLocation] = kNullLocation;
	genModelInputLocsInternal[angleInLocation] = kNullLocation;
}

void gridDynGenerator::generateSubModelInputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{

	if (!sD.updateRequired(subInputs.seqID))
	{
		return;
	}
	if (inputs.empty())
	{
		auto out = bus->getOutputs(noInputs,sD, sMode);
		subInputs.inputs[genmodel_loc][voltageInLocation] = out[voltageInLocation];
		subInputs.inputs[genmodel_loc][angleInLocation] = out[angleInLocation];
		subInputs.inputs[exciter_loc][exciterVoltageInLocation] = out[voltageInLocation];
		subInputs.inputs[governor_loc][govOmegaInLocation] = out[frequencyInLocation];
		if (isoc)
		{
			subInputs.inputs[isoc_control][0] = out[frequencyInLocation]- 1.0;
		}
	}
	else
	{
		subInputs.inputs[genmodel_loc][voltageInLocation] = inputs[voltageInLocation];
		subInputs.inputs[genmodel_loc][angleInLocation] = inputs[angleInLocation];
		subInputs.inputs[exciter_loc][exciterVoltageInLocation] = inputs[voltageInLocation];
		if (inputs.size() > frequencyInLocation)
		{
			subInputs.inputs[governor_loc][govOmegaInLocation] = inputs[frequencyInLocation];
		}
		if (isoc)
		{
			subInputs.inputs[isoc_control][0] = inputs[frequencyInLocation] -1.0;
		}
		
	}
	if (!opFlags[uses_bus_frequency])
	{
		subInputs.inputs[governor_loc][govOmegaInLocation] = genModel->getFreq(sD, sMode);
		if (isoc)
		{
			subInputs.inputs[isoc_control][0] = genModel->getFreq(sD, sMode)-1.0;
		}
	}
	
	double scale = systemBasePower / machineBasePower;
	double Pcontrol = pSetControlUpdate(inputs,sD,sMode);
	Pcontrol = valLimit(Pcontrol, Pmin, Pmax);
	
	subInputs.inputs[governor_loc][govpSetInLocation] = Pcontrol*scale;

	subInputs.inputs[exciter_loc][exciterVsetInLocation] = vSetControlUpdate(inputs,sD,sMode);
	double Eft = m_Eft;
	if ((ext) && (ext->isEnabled()))
	{
		Eft = ext->getOutput(subInputs.inputs[exciter_loc], sD, sMode, 0);
	}
	subInputs.inputs[genmodel_loc][genModelEftInLocation] = Eft;
	double pmech = Pcontrol*scale;
	if ((gov) && (gov->isEnabled()))
	{
		pmech = gov->getOutput(subInputs.inputs[governor_loc], sD, sMode, 0);
	}

	subInputs.inputs[genmodel_loc][genModelPmechInLocation] = pmech;
	
	if (!sD.empty())
	{
		subInputs.seqID = sD.seqID;
	}
	
}


void gridDynGenerator::generateSubModelInputLocs(const IOlocs &inputLocs, const stateData &sD, const solverMode &sMode)
{

	if (!sD.updateRequired(subInputLocs.seqID))
	{
		return;
	}

	subInputLocs.inputLocs[genmodel_loc][voltageInLocation] = inputLocs[voltageInLocation];
	subInputLocs.inputLocs[genmodel_loc][angleInLocation] = inputLocs[angleInLocation];
		subInputLocs.genModelInputLocsExternal[voltageInLocation] = inputLocs[voltageInLocation];
		subInputLocs.genModelInputLocsExternal[angleInLocation] = inputLocs[angleInLocation];

		if ((ext) && (ext->isEnabled()))
		{
			subInputLocs.inputLocs[exciter_loc][exciterVoltageInLocation] = inputLocs[voltageInLocation];
			subInputLocs.inputLocs[exciter_loc][exciterVsetInLocation] = vSetLocation(sMode);
			subInputLocs.inputLocs[genmodel_loc][genModelEftInLocation]=ext->getOutputLoc( sMode, 0);
		}
		else
		{
			subInputLocs.inputLocs[genmodel_loc][genModelEftInLocation] = kNullLocation;
		}
		subInputLocs.genModelInputLocsInternal[genModelEftInLocation] = subInputLocs.inputLocs[genmodel_loc][genModelEftInLocation];
		if ((gov) && (gov->isEnabled()))
		{
			if (genModel->checkFlag(uses_bus_frequency))
			{
				subInputLocs.inputLocs[governor_loc][govOmegaInLocation] = inputLocs[frequencyInLocation];
			}
			else
			{
				index_t floc;
				genModel->getFreq(sD, sMode, &floc);
				subInputLocs.inputLocs[governor_loc][govOmegaInLocation] = floc;
			}
			subInputLocs.inputLocs[governor_loc][govpSetInLocation] = pSetLocation(sMode);
			subInputLocs.inputLocs[genmodel_loc][genModelPmechInLocation]=gov->getOutputLoc(sMode, 0);
		}
		else
		{
			subInputLocs.inputLocs[genmodel_loc][genModelPmechInLocation] = pSetLocation(sMode);
		}
		subInputLocs.genModelInputLocsInternal[genModelPmechInLocation] = subInputLocs.inputLocs[genmodel_loc][genModelPmechInLocation];
		
		if (isoc)
		{
			subInputLocs.inputLocs[isoc_control][0] = subInputLocs.inputLocs[governor_loc][govOmegaInLocation];
		}
		subInputs.seqID = sD.seqID;
}

double gridDynGenerator::pSetControlUpdate(const IOdata & inputs, const stateData &sD, const solverMode &sMode)
{
	double val;
	if (pSetControl)
	{
		val = pSetControl->getOutput(inputs, sD, sMode);
		
	}
	else
	{
		val = (!sD.empty()) ? (Pset + dPdt * (sD.time - prevTime)) : Pset;
	}
	if (opFlags[isochronous_operation])
	{
		isoc->setLimits(Pmin - val, Pmax - val);
		isoc->setFreq(subInputs.inputs[isoc_control][0]);
		
		val = val + isoc->getOutput()*machineBasePower/systemBasePower;
	}
	return val;
}

double gridDynGenerator::vSetControlUpdate(const IOdata & inputs, const stateData &sD, const solverMode &sMode)
{
	return (vSetControl) ? vSetControl->getOutput(inputs, sD, sMode) : 1.0;
}

index_t gridDynGenerator::pSetLocation(const solverMode &sMode)
{

	return (pSetControl) ? pSetControl->getOutputLoc(sMode) : kNullLocation;
}
index_t gridDynGenerator::vSetLocation(const solverMode &sMode)
{

	return (vSetControl) ? vSetControl->getOutputLoc(sMode) : kNullLocation;
}