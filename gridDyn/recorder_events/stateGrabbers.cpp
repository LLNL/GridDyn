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

#include "stateGrabber.h"
#include "gridBus.h"
#include "linkModels/gridLink.h"
#include "loadModels/gridLoad.h"
#include "relays/gridRelay.h"
#include "relays/sensor.h"
#include "generators/gridDynGenerator.h"
#include "grabberInterpreter.hpp"
#include "gridCondition.h"
#include "arrayDataSparse.h"


static grabberInterpreter<stateGrabber, stateOpGrabber, stateFunctionGrabber> sgInterpret ([](const std::string &fld, gridCoreObject *obj){
  return std::make_shared<stateGrabber> (fld, obj);
});

std::vector < std::shared_ptr < stateGrabber >> makeStateGrabbers (const std::string & command, gridCoreObject * obj)
{
  size_t rlc;
  std::shared_ptr<stateGrabber> sgb;
  std::vector < std::shared_ptr < stateGrabber >> v;
  auto gsplit = splitlineBracketTrim (command,",;");
  for (auto &cmd:gsplit)
    {
      if ((rlc = cmd.find_first_of (":(+-/*\\^")) != std::string::npos)
        {
          sgb = sgInterpret.interpretGrabberBlock (cmd, obj);
          if ((sgb)&&(sgb->loaded))
            {
              v.push_back (sgb);
            }
        }
      else
        {
          sgb = std::make_shared<stateGrabber> (cmd, obj);
          if ((sgb) && (sgb->loaded))
            {
              v.push_back (sgb);
            }
        }
    }
  return v;

}

stateGrabber::stateGrabber (std::string fld, gridCoreObject* obj)
{
  setInfo (fld, obj);
}

std::shared_ptr<stateGrabber> stateGrabber::clone (gridCoreObject *nobj, std::shared_ptr<stateGrabber> ggb) const
{
  if (ggb == nullptr)
    {
      ggb = std::make_shared<stateGrabber> ();
    }
  ggb->field = field;
  ggb->fptr = fptr;
  ggb->gain = gain;
  ggb->bias = bias;
  ggb->inputUnits = inputUnits;
  ggb->outputUnits = outputUnits;
  ggb->loaded = loaded;
  ggb->offset = offset;
  ggb->jacCapable = jacCapable;
  ggb->prevIndex = prevIndex;

  if (nobj)
    {
      ggb->updateObject (nobj);
    }
  else
    {
      ggb->updateObject (cobj);
    }
  return ggb;
}

int stateGrabber::setInfo (std::string fld, gridCoreObject* obj)
{
  field = fld;
  cobj = obj;
  makeLowerCase (fld);
  loaded = true;
  if (dynamic_cast<gridBus *> (obj))
    {
      busLoadInfo (fld);
    }
  else if (dynamic_cast<gridLink *> (obj))
    {
      linkLoadInfo (fld);
    }
  else if (dynamic_cast<gridSecondary *> (obj))
    {
      secondaryLoadInfo (fld);
    }
  else if (dynamic_cast<gridRelay *> (obj))
    {

      relayLoadInfo (fld);
    }
  else
    {
      loaded = false;
    }
  return (loaded) ? PARAMETER_FOUND : PARAMETER_NOT_FOUND;

}

void stateGrabber::busLoadInfo (const std::string &fld)
{
  if (fld == "voltage")
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridBus *> (cobj)->getVoltage (sD, sMode);
        };
      jacCapable = true;
      jacIfptr = [ = ](const stateData *, arrayData<double> *ad, const solverMode &sMode) {
          ad->assignCheckCol (0, static_cast<gridBus *> (cobj)->offsets.getVOffset (sMode), 1);
        };

    }
  else if (fld == "angle")
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridBus *> (cobj)->getAngle (sD->state, sMode);
        };
      jacCapable = true;
      jacIfptr = [ = ](const stateData *, arrayData<double> *ad, const solverMode &sMode) {
          ad->assignCheckCol (0, static_cast<gridBus *> (cobj)->offsets.getVOffset (sMode), 1);
        };
    }
  else if ((fld == "freq") || (fld == "frequency"))
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridBus *> (cobj)->getFreq (sD, sMode);
        };
    }
  else
    {
      loaded = false;
    }
}

void stateGrabber::linkLoadInfo (const std::string &fld)
{
  std::string fieldStr;
  int num = trailingStringInt (fld, fieldStr, 1);
  if (fieldStr == "angle")
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridLink *> (cobj)->getAngle (sD->state, sMode);
        };
    }
  else if ((fieldStr == "impedance") || (fieldStr == "z"))
    {
      if (num == 1)
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (cobj)->getTotalImpedance (1);
            };
        }
      else
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (cobj)->getTotalImpedance (2);
            };
        }

    }
  else if ((fieldStr == "admittance") || (fieldStr == "y"))
    {
      if (num == 1)
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return 1.0 / static_cast<gridLink *> (cobj)->getTotalImpedance (1);
            };
        }
      else
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return 1.0 / static_cast<gridLink *> (cobj)->getTotalImpedance (2);
            };
        }

    }
  else if ((fieldStr == "current") || (fieldStr == "i"))
    {
      if (num == 1)
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (cobj)->getCurrent (1);
            };
        }
      else
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (cobj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (cobj)->getCurrent (2);
            };
        }

    }
  else
    {
      loaded = false;
    }
}
void stateGrabber::relayLoadInfo (const std::string &fld)
{
  std::string fieldStr;
  int num = trailingStringInt (fld, fieldStr, 0);
  if ((fieldStr == "value") || (fieldStr == "output") || (fieldStr == "o"))
    {
      //dgptr = &gridLink::getAngle;
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridRelay *> (cobj)->getOutput (sD, sMode, static_cast<index_t> (num));
        };
    }
  else if ((fieldStr == "block") || (fieldStr == "b"))
    {
      if (dynamic_cast<sensor *> (cobj))
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              return static_cast<sensor *> (cobj)->getBlockOutput (sD, sMode, num);
            };
        }
      else
        {
          loaded = false;
        }
    }
  else if ((fieldStr == "input") || (fieldStr == "i"))
    {
      if (dynamic_cast<sensor *> (cobj))
        {
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              return static_cast<sensor *> (cobj)->getInput (sD, sMode, num);
            };
        }
      else
        {
          loaded = false;
        }
    }
  else if ((fieldStr == "condition") || (fieldStr == "c"))
    {
      //dgptr = &gridLink::getAngle;
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return (static_cast<gridRelay *> (cobj))->getCondition (num)->getVal (1, sD, sMode);
        };
    }
  else
    {
      if (dynamic_cast<sensor *> (cobj))
        {
          //try to lookup named output for sensors
          index_t outIndex = static_cast<sensor *> (cobj)->lookupOutput (fld);
          if (outIndex != kNullLocation)
            {
              fptr = [ = ](const stateData *sD, const solverMode &sMode) {
                  return static_cast<sensor *> (cobj)->getOutput (sD, sMode, outIndex);
                };
            }
          else
            {
              loaded = false;
            }
        }
      else if (dynamic_cast<gridDynGenerator *> (cobj))
        {
          if (fld == "freq")
            {
              fptr = [ = ](const stateData *sD, const solverMode &sMode) {
                  return static_cast<gridDynGenerator *> (cobj)->getFreq (sD, sMode);
                };
            }
          else if (fld == "angle")
            {
              fptr = [ = ](const stateData *sD, const solverMode &sMode) {
                  return static_cast<gridDynGenerator *> (cobj)->getAngle (sD, sMode);
                };
            }
          else
            {
              loaded = false;
            }
        }
      else
        {
          loaded = false;
        }

    }
}

static IOdata kNullVec;
void stateGrabber::secondaryLoadInfo (const std::string &fld)
{
  if ((fld == "realpower") || (fld == "power") || (fld == "p"))
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridSecondary *> (cobj)->getRealPower (kNullVec, sD, sMode);
        };
      jacCapable = true;
      jacIfptr = [ = ](const stateData *sD, arrayData<double> *ad, const solverMode &sMode) {
          arrayDataSparse b;
          static_cast<gridSecondary *> (cobj)->outputPartialDerivatives (kNullVec, sD, &b, sMode);
          ad->copyTranslateRow (&b, 0, 0);
        };
    }
  else if ((fld == "reactivepower") || (fld == "reactive") || (fld == "q"))
    {
      fptr = [ = ](const stateData *sD, const solverMode &sMode) {
          return static_cast<gridSecondary *> (cobj)->getReactivePower (kNullVec, sD, sMode);
        };
      jacCapable = true;
      jacIfptr = [ = ](const stateData *sD, arrayData<double> *ad, const solverMode &sMode) {
          arrayDataSparse b;
          static_cast<gridSecondary *> (cobj)->outputPartialDerivatives (kNullVec, sD, &b, sMode);
          ad->copyTranslateRow (&b, 1, 0);
        };
    }
  else
    {
      offset = static_cast<gridSecondary *> (cobj)->findIndex (fld, cLocalbSolverMode);
      if (offset != kInvalidLocation)
        {
          prevIndex = 1;
          fptr = [ = ](const stateData *sD, const solverMode &sMode) {
              if (sMode.offsetIndex != prevIndex)
                {
                  offset = static_cast<gridSecondary *> (cobj)->findIndex (field, sMode);
                  prevIndex = sMode.offsetIndex;
                }
              return (offset != kNullLocation) ? sD->state[offset] : -kBigNum;
            };
          jacCapable = true;
          jacIfptr = [ = ](const stateData *, arrayData<double> *ad, const solverMode &) {
              ad->assignCheckCol (0, offset, 1.0);
            };
        }
      else
        {
          loaded = false;
        }
    }
}

void stateGrabber::areaLoadInfo (const std::string & /*fld*/)
{

}

double stateGrabber::grabData (const stateData *sD, const solverMode &sMode)
{
  if (loaded)
    {
      double val = fptr (sD, sMode);
      val = std::fma (val, gain, bias);
      return val;
    }
  else
    {
      return kNullVal;
    }

}

void stateGrabber::updateObject (gridCoreObject *obj)
{
  setInfo (field, obj);
}


void stateGrabber::outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  if (!jacCapable)
    {
      return;
    }
  if (gain != 1.0)
    {
      arrayDataSparse bd;

      jacIfptr (sD, &bd, sMode);
      bd.scale (gain);
      ad->merge (&bd);
    }
  else
    {
      jacIfptr (sD, ad, sMode);
    }

}


void customStateGrabber::setGrabberFunction (std::function<double(const stateData *sD, const solverMode &sMode)> nfptr)
{
  fptr = nfptr;
  loaded = true;
}

std::shared_ptr<stateGrabber> customStateGrabber::clone (gridCoreObject *nobj, std::shared_ptr<stateGrabber > ggb) const
{
  std::shared_ptr<customStateGrabber> cgb;
  if (ggb == nullptr)
    {
      cgb = std::make_shared<customStateGrabber> ();
    }
  else
    {
      if (std::dynamic_pointer_cast<customStateGrabber> (ggb))
        {
          cgb = std::dynamic_pointer_cast<customStateGrabber> (ggb);
        }
      else
        {
          return stateGrabber::clone (nobj, ggb);
        }
    }
  stateGrabber::clone (nobj, cgb);
  cgb->fptr = fptr;

  return cgb;
}

stateFunctionGrabber::stateFunctionGrabber (std::shared_ptr<stateGrabber> ggb, std::string func)
{
  function_name = func;
  if (ggb)
    {
      bgrabber = ggb;
    }
  opptr = get1ArgFunction (func);
  jacCapable = (bgrabber->jacCapable);
  loaded = bgrabber->loaded;
}


int stateFunctionGrabber::setInfo (std::string /*fld*/, gridCoreObject*)
{
  return LOADED;
}



std::shared_ptr<stateGrabber> stateFunctionGrabber::clone (gridCoreObject *nobj, std::shared_ptr<stateGrabber> ggb) const
{
  std::shared_ptr<stateFunctionGrabber> fgb;
  if (ggb == nullptr)
    {
      fgb = std::make_shared<stateFunctionGrabber> ();
    }
  else
    {
      if (std::dynamic_pointer_cast<stateFunctionGrabber> (ggb))
        {
          fgb = std::dynamic_pointer_cast<stateFunctionGrabber> (ggb);
        }
      else
        {
          return stateGrabber::clone (nobj, ggb);
        }
    }
  fgb->bgrabber = bgrabber->clone (nobj, nullptr);
  fgb->function_name = function_name;
  fgb->opptr = opptr;
  stateGrabber::clone (nobj, fgb);
  return fgb;
}

double stateFunctionGrabber::grabData (const stateData *sD, const solverMode &sMode)
{
  double val;

  double temp = bgrabber->grabData (sD, sMode);
  val = opptr (temp);
  val = std::fma (val, gain, bias);
  return val;
}


void stateFunctionGrabber::updateObject (gridCoreObject *obj)
{
  if (bgrabber)
    {
      bgrabber->updateObject (obj);
    }
}

gridCoreObject *stateFunctionGrabber::getObject () const
{
  if (bgrabber)
    {
      return bgrabber->getObject ();
    }
  else
    {
      return nullptr;
    }
}

void stateFunctionGrabber::outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{

  if (!jacCapable)
    {
      return;
    }
  arrayDataSparse d1;
  bgrabber->outputPartialDerivatives (sD, &d1, sMode);

  double temp = bgrabber->grabData (sD, sMode);
  double t1 = opptr (temp);
  double t2 = opptr (temp + 1e-7);
  double dodI = (t2 - t1) / 1e-7;
  d1.scale (dodI * gain);
  ad->merge (&d1);
}

stateOpGrabber::stateOpGrabber (std::shared_ptr<stateGrabber> ggb1, std::shared_ptr<stateGrabber> ggb2, std::string op)
{
  op_name = op;
  if (ggb1)
    {
      bgrabber1 = ggb1;
    }
  if (ggb2)
    {
      bgrabber2 = ggb2;
    }
  opptr = get2ArgFunction (op);
  jacCapable = (bgrabber1->jacCapable)&&(bgrabber2->jacCapable);
  loaded = ((ggb1->loaded) && (ggb2->loaded));
}


int stateOpGrabber::setInfo (std::string /*fld*/, gridCoreObject* )
{
  return LOADED;
}

std::shared_ptr<stateGrabber> stateOpGrabber::clone (gridCoreObject *nobj, std::shared_ptr<stateGrabber> ggb) const
{
  std::shared_ptr<stateOpGrabber> ogb;
  if (ggb == nullptr)
    {
      ogb = std::make_shared<stateOpGrabber> ();
    }
  else
    {
      if (std::dynamic_pointer_cast<stateOpGrabber> (ggb))
        {
          ogb = std::dynamic_pointer_cast<stateOpGrabber> (ggb);
        }
      else
        {
          return stateGrabber::clone (nobj, ggb);
        }
    }
  ogb->bgrabber1 = bgrabber1->clone (nobj, nullptr);
  ogb->bgrabber2 = bgrabber2->clone (nobj, nullptr);
  ogb->op_name = op_name;
  ogb->opptr = opptr;
  stateGrabber::clone (nobj, ogb);
  return ogb;
}

double stateOpGrabber::grabData (const stateData *sD, const solverMode &sMode)
{
  double val;

  double temp1 = bgrabber1->grabData (sD, sMode);
  double temp2 = bgrabber2->grabData (sD, sMode);
  val = opptr (temp1, temp2);
  val = std::fma (val, gain, bias);
  return val;
}



void stateOpGrabber::updateObject (gridCoreObject *obj)
{
  if (bgrabber1)
    {
      bgrabber1->updateObject (obj);
    }
  if (bgrabber2)
    {
      bgrabber2->updateObject (obj);
    }
}

void stateOpGrabber::updateObject (gridCoreObject *obj, int num)
{
  if (num == 1)
    {
      if (bgrabber1)
        {
          bgrabber1->updateObject (obj);
        }
    }
  else if (num == 2)
    {
      if (bgrabber2)
        {
          bgrabber2->updateObject (obj);
        }
    }

}

gridCoreObject *stateOpGrabber::getObject () const
{
  if (bgrabber1)
    {
      return bgrabber1->getObject ();
    }
  else
    {
      return nullptr;
    }
}

void stateOpGrabber::outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{

  if (!jacCapable)
    {
      return;
    }
  arrayDataSparse d1;
  arrayDataSparse d2;
  bgrabber1->outputPartialDerivatives (sD, &d1, sMode);
  bgrabber2->outputPartialDerivatives (sD, &d2, sMode);

  double temp1 = bgrabber1->grabData (sD, sMode);
  double temp2 = bgrabber2->grabData (sD, sMode);

  double t1 = opptr (temp1,temp2);
  double t2 = opptr (temp1 + 1e-7,temp2);
  double t3 = opptr (temp1, temp2 + 1e-7);
  double dodI = (t2 - t1) / 1e-7;

  d1.scale (dodI * gain);
  dodI = (t3 - t1) / 1e-7;
  d2.scale (dodI * gain);

  ad->merge (&d1);
  ad->merge (&d2);
}
