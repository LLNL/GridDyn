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
#include "relays/gridRelay.h"
#include "relays/sensor.h"
#include "gridArea.h"
#include "generators/gridDynGenerator.h"
#include "grabberInterpreter.hpp"
#include "gridCondition.h"
#include "matrixDataScale.h"
#include "matrixDataSparse.h"
#include "core/helperTemplates.h"
#include <map>

static grabberInterpreter<stateGrabber, stateOpGrabber, stateFunctionGrabber> sgInterpret ([](const std::string &fld, gridCoreObject *obj){
  return std::make_shared<stateGrabber> (fld, obj);
});

std::vector < std::shared_ptr < stateGrabber >> makeStateGrabbers (const std::string & command, gridCoreObject * obj)
{
  std::vector < std::shared_ptr < stateGrabber >> v;
  auto gsplit = splitlineBracketTrim (command,",;");
  for (auto &cmd:gsplit)
    {
      if (cmd.find_first_of (":(+-/*\\^?") != std::string::npos)
        {
          auto sgb = sgInterpret.interpretGrabberBlock (cmd, obj);
          if ((sgb)&&(sgb->loaded))
            {
              v.push_back (sgb);
            }
        }
      else
        {
          auto sgb = std::make_shared<stateGrabber> (cmd, obj);
          if ((sgb) && (sgb->loaded))
            {
              v.push_back (sgb);
            }
        }
    }
  return v;

}

stateGrabber::stateGrabber( gridCoreObject* obj):cobj(obj)
{

}

stateGrabber::stateGrabber (std::string fld, gridCoreObject* obj):cobj(obj)
{
	stateGrabber::updateField(fld);
}

std::shared_ptr<stateGrabber> stateGrabber::clone (std::shared_ptr<stateGrabber> ggb) const
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
  ggb->cobj = cobj;
  
  return ggb;
}

int stateGrabber::updateField (std::string fld)
{
  field = fld;
  makeLowerCase (fld);
  loaded = true;
  if (dynamic_cast<gridBus *> (cobj))
    {
      busLoadInfo (fld);
    }
  else if (dynamic_cast<gridLink *> (cobj))
    {
      linkLoadInfo (fld);
    }
  else if (dynamic_cast<gridSecondary *> (cobj))
    {
      secondaryLoadInfo (fld);
    }
  else if (dynamic_cast<gridRelay *> (cobj))
    {
      relayLoadInfo (fld);
    }
  else
    {
      loaded = false;
    }
  return (loaded) ? PARAMETER_FOUND : PARAMETER_NOT_FOUND;

}

using namespace gridUnits;

/** map of all the alternate strings that can be used*/
/* *INDENT-OFF* */
static const std::map<std::string, std::string> stringTranslate
{
	{ "voltage","v" },
	{ "vol","v" },
	{ "link","linkreal" },
	{ "linkp","linkreal" },
	{ "loadq","loadreactive" },
	{ "loadreactivepower","loadreactive" },
	{ "load","loadreal" },
	{ "loadp","loadreal" },
	{ "reactivegen","genreactive" },
	{ "genq","genreactive" },
	{ "gen","genreal" },
	{ "generation","genreal" },
	{ "genp","genreal" },
	{ "realgen","genreal" },
	{ "f","freq" },
	{ "frequency","freq" },
	{ "omega","freq" },
	{ "a","angle" },
	{ "phase","angle" },
	{ "busgenerationreal","busgenreal" },
	{ "busp","busgenreal" },
	{ "busgen","busgenreal" },
	{ "busgenerationreactive","busgenreactive" },
	{ "busq","busgenreactive" },
	{ "linkrealpower","linkreal" },
	{ "linkp1","linkreal" },
	{ "linkq","linkreactive" },
	{ "linkreactivepower","linkreactive" },
	{ "linkrealpower1","linkreal" },
	{ "linkq1","linkreactive" },
	{ "linkreactivepower1","linkreactive" },
	{ "linkreal1","linkreal" },
	{ "linkreactive1","linkreactive" },
	{ "linkrealpower2","linkreal2" },
	{ "linkq2","linkreactive2" },
	{ "linkreactivepower2","linkreactive2" },
	{ "linkp2","linkreal2" },
	{ "p","real" },
	{ "q","reactive" },
	{ "impedance","z" },
	{ "admittance","y" },
	{ "impedance1","z" },
	{ "admittance1","y" },
	{ "z1","z" },
	{ "y1","y" },
	{ "impedance2","z2" },
	{ "admittance2","y2" },
	{ "status","connected" },
	{ "breaker","switch" },
	{ "breaker1","switch" },
	{ "switch1","switch" },
	{ "breaker2","switch2" },
	{ "i","current" },
	{ "i1","current" },
	{ "current1","current" },
	{ "i2","current2" },
	{ "imagcurrent1","imagcurrent" },
	{ "realcurrent1","realcurrent" },
	{ "lossreal","loss" },
	{ "angle1","angle" },
	{ "absangle1","absangle" },
	{ "voltage1","voltage" },
	{ "v1","voltage" },
	{ "v2","voltage2" },
};

#define FUNCTION_SIGNATURE [](gridCoreObject *obj, const stateData *sD, const solverMode &sMode)
#define FUNCTION_SIGNATURE_OBJ_ONLY [](gridCoreObject *obj, const stateData *, const solverMode &)

#define JAC_FUNCTION_SIGNATURE [](gridCoreObject *obj, const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
#define JAC_FUNCTION_SIGNATURE_NO_STATE [](gridCoreObject *obj, const stateData *, matrixData<double> &ad, const solverMode &sMode)

static const std::map<std::string, fstateobjectPair> objectFunctions
{
	{ "connected",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<double>(static_cast<gridObject *>(obj)->isConnected()); },defUnit } },
	{ "enabled",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<double>(static_cast<gridObject *>(obj)->enabled); },defUnit } },
	{ "armed",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<double>(static_cast<gridObject *>(obj)->isArmed()); },defUnit } },

};

static const std::map<std::string, fstateobjectPair> busFunctions
{
	{ "v",{ FUNCTION_SIGNATURE{return static_cast<gridBus *> (obj)->getVoltage(sD, sMode); }, puV } },
	{ "angle",{ FUNCTION_SIGNATURE{return static_cast<gridBus *>(obj)->getAngle(sD, sMode); }, rad } },
	{ "freq",{ FUNCTION_SIGNATURE{return static_cast<gridBus *>(obj)->getFreq(sD, sMode); },puHz } },
	{ "genreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getLoadReactive(); }, puMW } },
	{ "linkreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getLinkReal(); }, puMW } },
	{ "linkreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridBus *>(obj)->getLinkReactive(); }, puMW } },
};

static const std::map<std::string, objJacFunction> busJacFunctions
{
	{ "v",JAC_FUNCTION_SIGNATURE_NO_STATE{ ad.assignCheckCol(0, static_cast<gridBus *> (obj)->getOutputLoc(sMode,voltageInLocation), 1.0); }  },
	{ "angle", JAC_FUNCTION_SIGNATURE_NO_STATE{ ad.assignCheckCol(0, static_cast<gridBus *> (obj)->getOutputLoc(sMode,angleInLocation), 1.0); } },
};

static const std::map<std::string, fstateobjectPair> areaFunctions
{
	{ "avgfreq",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getAvgFreq(); },puHz } },
	{ "genreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getGenerationReal(); }, puMW } },
	{ "genreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getGenerationReactive(); }, puMW } },
	{ "loadreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getLoadReal(); }, puMW } },
	{ "loadreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getLoadReactive(); }, puMW } },
	{ "loss",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getLoss(); }, puMW } },
	{ "tieflow",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridArea *>(obj)->getTieFlowReal(); }, puMW } },
};


static const IOdata kNullVec;
static const  std::function<double(gridCoreObject *, const stateData *, const solverMode &)> rpower = FUNCTION_SIGNATURE{ return static_cast<gridSecondary *> (obj)->getRealPower(kNullVec, sD, sMode); };
static const  std::function<double(gridCoreObject *, const stateData *, const solverMode &)> qpower = FUNCTION_SIGNATURE{ return static_cast<gridSecondary *> (obj)->getReactivePower(kNullVec, sD, sMode); };

static const std::map<std::string, fstateobjectPair> loadFunctions
{
	{ "real",{rpower, puMW } },
	{ "reactive",{qpower, puMW } },
	{ "loadreal",{ rpower,puMW } },
	{ "loadreactive",{qpower, puMW } }
};

static const std::map<std::string, fstateobjectPair> genFunctions
{
	{ "real",{ rpower, puMW } },
	{ "reactive",{ qpower, puMW } },
	{ "genreal",{ rpower,puMW } },
	{ "genreactive",{qpower, puMW } },
	{ "pset",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getPset(); },puMW } },
	{ "adjup",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityUp(); },puMW } },
	{ "adjdown",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getAdjustableCapacityDown(); },puMW } },
	{ "pmax",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getPmax(); },puMW } },
	{ "pmin",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getPmin(); },puMW } },
	{ "qmax",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getQmax(); },puMW } },
	{ "qmin",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getQmin(); },puMW } },
	{ "freq",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getFreq(nullptr,cLocalSolverMode); },puHz } },
	{ "angle",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridDynGenerator *>(obj)->getAngle(nullptr,cLocalSolverMode); },rad } },
};

static const std::map<std::string, fstateobjectPair> linkFunctions
{
	{ "real",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealPower(1); }, puMW } },
	{ "reactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "linkreal",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealPower(1); },puMW } },
	{ "linkreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getReactivePower(1); }, puMW } },
	{ "z",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getTotalImpedance(1); },puOhm } },
	{ "y",{ FUNCTION_SIGNATURE_OBJ_ONLY{return 1.0 / (static_cast<gridLink *>(obj)->getTotalImpedance(1)); }, puOhm } },
	{ "r",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealImpedance(1); },puOhm } },
	{ "x",{ FUNCTION_SIGNATURE_OBJ_ONLY{return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(1)); }, puOhm } },
	{ "current",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getCurrent(1)); }, puA } },
	{ "realcurrent",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getRealCurrent(1)); }, puA } },
	{ "imagcurrent",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getImagCurrent(1)); }, puA } },
	{ "voltage",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getVoltage(1)); }, puV } },
	{ "absangle",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getAbsAngle(1)); }, rad } },
	//functions for to side
	{ "real2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealPower(2); }, puMW } },
	{ "reactive2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "linkreal2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealPower(2); },puMW } },
	{ "linkreactive2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getReactivePower(2); }, puMW } },
	{ "z2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getTotalImpedance(2); },puOhm } },
	{ "y2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return 1.0 / (static_cast<gridLink *>(obj)->getTotalImpedance(2)); }, puOhm } },
	{ "r2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<gridLink *>(obj)->getRealImpedance(2); },puOhm } },
	{ "x2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return 1.0 / (static_cast<gridLink *>(obj)->getImagImpedance(2)); }, puOhm } },
	{ "current2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getCurrent(2)); }, puA } },
	{ "realcurrent2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getRealCurrent(2)); }, puA } },
	{ "imagcurrent2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getImagCurrent(2)); }, puA } },
	{ "voltage2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getVoltage(2)); }, puV } },
	{ "absangle2",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getAbsAngle(2)); }, rad } },

	//non numbered fields
	{ "angle",{ FUNCTION_SIGNATURE{return static_cast<gridLink *> (obj)->getAngle(sD->state, sMode);}, rad } },
	{ "loss",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getLoss()); }, puMW } },
	{ "lossreactive",{ FUNCTION_SIGNATURE_OBJ_ONLY{return (static_cast<gridLink *>(obj)->getReactiveLoss()); }, puMW } },
	{ "attached",{ FUNCTION_SIGNATURE_OBJ_ONLY{return static_cast<double> (((!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch1_open_flag)) || (!static_cast<gridLink *>(obj)->checkFlag(gridLink::switch2_open_flag))) && (static_cast<gridLink *>(obj)->enabled)); }, defUnit } },
};

/* *INDENT-ON* */

void stateGrabber::busLoadInfo (const std::string &fld)
{
	auto fnd = stringTranslate.find(fld);
	std::string nfstr = (fnd == stringTranslate.end()) ? field : fnd->second;

	auto funcfind = busFunctions.find(nfstr);
	if (funcfind != busFunctions.end())
	{
		fptr=funcfind->second.first;
		inputUnits = funcfind->second.second;
		loaded = true;
		auto jacfind = busJacFunctions.find(nfstr);
		if (jacfind != busJacFunctions.end())
		{
			jacIfptr = jacfind->second;
			jacCapable = true;
		}
	}
	else
	{
		funcfind = objectFunctions.find(nfstr);
		if (funcfind != objectFunctions.end())
		{
			fptr= funcfind->second.first;
		}
		else
		{
			fptr = nullptr;
			loaded = false;
		}
	}
  
}

void stateGrabber::linkLoadInfo (const std::string &fld)
{
  std::string fieldStr;
  int num = trailingStringInt (fld, fieldStr, 1);
  if (fieldStr == "angle")
    {
      fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
          return static_cast<gridLink *> (obj)->getAngle (sD->state, sMode);
        };
    }
  else if ((fieldStr == "impedance") || (fieldStr == "z"))
    {
      if (num == 1)
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (obj)->getTotalImpedance (1);
            };
        }
      else
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (obj)->getTotalImpedance (2);
            };
        }

    }
  else if ((fieldStr == "admittance") || (fieldStr == "y"))
    {
      if (num == 1)
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return 1.0 / static_cast<gridLink *> (obj)->getTotalImpedance (1);
            };
        }
      else
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return 1.0 / static_cast<gridLink *> (obj)->getTotalImpedance (2);
            };
        }

    }
  else if ((fieldStr == "current") || (fieldStr == "i"))
    {
      if (num == 1)
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (obj)->getCurrent (1);
            };
        }
      else
        {
          fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              static_cast<gridLink *> (obj)->updateLocalCache (sD, sMode);
              return static_cast<gridLink *> (obj)->getCurrent (2);
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
      fptr = [num ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
          return static_cast<gridRelay *> (obj)->getOutput (sD, sMode, static_cast<index_t> (num));
        };
    }
  else if ((fieldStr == "block") || (fieldStr == "b"))
    {
      if (dynamic_cast<sensor *> (cobj))
        {
          fptr = [ num ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              return static_cast<sensor *> (obj)->getBlockOutput (sD, sMode, num);
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
          fptr = [ num ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              return static_cast<sensor *> (obj)->getInput (sD, sMode, num);
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
      fptr = [ num ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
          return (static_cast<gridRelay *> (obj))->getCondition (num)->getVal (1, sD, sMode);
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
              fptr = [ outIndex ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
                  return static_cast<sensor *> (obj)->getOutput (sD, sMode, outIndex);
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
              fptr = [ = ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
                  return static_cast<gridDynGenerator *> (obj)->getFreq (sD, sMode);
                };
            }
          else if (fld == "angle")
            {
              fptr = [ = ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
                  return static_cast<gridDynGenerator *> (obj)->getAngle (sD, sMode);
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


void stateGrabber::secondaryLoadInfo (const std::string &fld)
{
  if ((fld == "realpower") || (fld == "power") || (fld == "p"))
    {
      fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
          return static_cast<gridSecondary *> (obj)->getRealPower (kNullVec, sD, sMode);
        };
      jacCapable = true;
      jacIfptr = [ ](gridCoreObject *obj, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) {
          matrixDataSparse<double> b;
          static_cast<gridSecondary *> (obj)->outputPartialDerivatives (kNullVec, sD, b, sMode);
          ad.copyTranslateRow (b, 0, 0);
        };
    }
  else if ((fld == "reactivepower") || (fld == "reactive") || (fld == "q"))
    {
      fptr = [ ](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
          return static_cast<gridSecondary *> (obj)->getReactivePower (kNullVec, sD, sMode);
        };
      jacCapable = true;
      jacIfptr = [  ](gridCoreObject *obj, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) {
          matrixDataSparse<double> b;
          static_cast<gridSecondary *> (obj)->outputPartialDerivatives (kNullVec, sD, b, sMode);
          ad.copyTranslateRow (b, 1, 0);
        };
    }
  else
    {
      offset = static_cast<gridSecondary *> (cobj)->findIndex (fld, cLocalbSolverMode);
      if (offset != kInvalidLocation)
        {
          prevIndex = 1;
          fptr = [ =](gridCoreObject *obj, const stateData *sD, const solverMode &sMode) {
              if (sMode.offsetIndex != prevIndex)
                {
                  offset = static_cast<gridSecondary *> (obj)->findIndex (field, sMode);
                  prevIndex = sMode.offsetIndex;
                }
              return (offset != kNullLocation) ? sD->state[offset] : -kBigNum;
            };
          jacCapable = true;
          jacIfptr = [ =](gridCoreObject *,const stateData *, matrixData<double> &ad, const solverMode &) {
              ad.assignCheckCol (0, offset, 1.0);
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
      double val = fptr (cobj,sD, sMode);
      val = std::fma (val, gain, bias);
      return val;
    }
  else
    {
      return kNullVal;
    }

}

void stateGrabber::updateObject (gridCoreObject *obj, object_update_mode mode)
{
	if (mode == object_update_mode::direct)
	{
		cobj = obj;
	}
	else if (mode == object_update_mode::match)
	{
		cobj=findMatchingObject(cobj, obj);
	}
}

gridCoreObject * stateGrabber::getObject() const
{
	return cobj;
}

void stateGrabber::getObjects(std::vector<gridCoreObject *> &objects) const
{
	objects.push_back(getObject());
}

void stateGrabber::outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{
  if (!jacCapable)
    {
      return;
    }
  if (gain != 1.0)
    {
      matrixDataScale<double> bd(ad,gain); 
      jacIfptr (cobj,sD, bd, sMode);
    }
  else
    {
      jacIfptr (cobj,sD, ad, sMode);
    }

}

customStateGrabber::customStateGrabber(gridCoreObject *obj) :stateGrabber(obj)
{

}

void customStateGrabber::setGrabberFunction (objStateGrabberFunction nfptr)
{
  fptr = nfptr;
  loaded = true;
}


void customStateGrabber::setGrabberJacFunction(objJacFunction nJfptr)
{
	jacIfptr = nJfptr;
	jacCapable = (jacIfptr)?true:false;
}

std::shared_ptr<stateGrabber> customStateGrabber::clone (std::shared_ptr<stateGrabber > ggb) const
{
	auto cgb = cloneBase<customStateGrabber, stateGrabber>(this, ggb);
  if (cgb == nullptr)
    {
	  return ggb;
    }
  return cgb;
}

stateFunctionGrabber::stateFunctionGrabber (std::shared_ptr<stateGrabber> ggb, std::string func): function_name(func)
{
  if (ggb)
    {
      bgrabber = ggb;
    }
  opptr = get1ArgFunction (function_name);
  jacCapable = (bgrabber->jacCapable);
  loaded = bgrabber->loaded;
}


int stateFunctionGrabber::updateField (std::string fld)
{
	if (!fld.empty())
	{
		if (isFunctionName(fld, function_type::arg))
		{
			function_name = fld;
			opptr = get1ArgFunction(function_name);
		}
		else
		{
			return NOT_LOADED;
		}
	}
	
	return LOADED;
}



std::shared_ptr<stateGrabber> stateFunctionGrabber::clone (std::shared_ptr<stateGrabber> ggb) const
{
	auto fgb = cloneBase<stateFunctionGrabber, stateGrabber>(this, ggb);
  if (fgb == nullptr)
    {
	  if (bgrabber)
	  {
		  return bgrabber->clone(ggb);
	  }
	  else
	  {
		  return ggb;
	  }
    }
  if (bgrabber)
  {
	  fgb->bgrabber = bgrabber->clone(fgb->bgrabber);
  }
  fgb->function_name = function_name;
  fgb->opptr = opptr;
  return fgb;
}

double stateFunctionGrabber::grabData (const stateData *sD, const solverMode &sMode)
{
  double val = opptr (bgrabber->grabData(sD, sMode));
  val = std::fma (val, gain, bias);
  return val;
}


void stateFunctionGrabber::updateObject (gridCoreObject *obj, object_update_mode mode)
{
  if (bgrabber)
    {
      bgrabber->updateObject (obj,mode);
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

void stateFunctionGrabber::outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{

  if (!jacCapable)
    {
      return;
    }

  double temp = bgrabber->grabData (sD, sMode);
  double t1 = opptr (temp);
  double t2 = opptr (temp + 1e-7);
  double dodI = (t2 - t1) / 1e-7;

  matrixDataScale<double> d1(ad, dodI * gain);
  bgrabber->outputPartialDerivatives(sD, d1, sMode);
}

stateOpGrabber::stateOpGrabber (std::shared_ptr<stateGrabber> ggb1, std::shared_ptr<stateGrabber> ggb2, std::string op): op_name(op)
{
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


int stateOpGrabber::updateField (std::string opName)
{
	if (!opName.empty())
	{
		if (isFunctionName(opName, function_type::arg2))
		{
			op_name = opName;
			opptr = get2ArgFunction(op_name);
		}
		else
		{
			return NOT_LOADED;
		}
	}

	return LOADED;
}

std::shared_ptr<stateGrabber> stateOpGrabber::clone (std::shared_ptr<stateGrabber> ggb) const
{
	auto ogb = cloneBase<stateOpGrabber, stateGrabber>(this, ggb);
  if (ogb == nullptr)
    {
	  if (bgrabber1)
	  {
		  bgrabber1->clone(ggb); 
	  }
	  return ggb;
    }
  if (bgrabber1)
  {
	  ogb->bgrabber1 = bgrabber1->clone(ogb->bgrabber1);
 }
  if (bgrabber2)
  {
	  ogb->bgrabber2 = bgrabber2->clone(ogb->bgrabber2);
  }
  
  ogb->op_name = op_name;
  ogb->opptr = opptr;
  return ogb;
}

double stateOpGrabber::grabData (const stateData *sD, const solverMode &sMode)
{
  double grabber1Data = bgrabber1->grabData (sD, sMode);
  double grabber2Data = bgrabber2->grabData (sD, sMode);
  double val = opptr (grabber1Data, grabber2Data);
  val = std::fma (val, gain, bias);
  return val;
}



void stateOpGrabber::updateObject (gridCoreObject *obj, object_update_mode mode)
{
  if (bgrabber1)
    {
      bgrabber1->updateObject (obj,mode);
    }
  if (bgrabber2)
    {
      bgrabber2->updateObject (obj,mode);
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
  else if (bgrabber2)
  {
	  return bgrabber2->getObject();
  }
  else
    {
      return nullptr;
    }
}

void stateOpGrabber::outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{

  if (!jacCapable)
    {
      return;
    }
 

  double grabber1Data = bgrabber1->grabData (sD, sMode);
  double grabber2Data = bgrabber2->grabData (sD, sMode);

  double t1 = opptr (grabber1Data, grabber2Data);
  double t2 = opptr (grabber1Data + 1e-7, grabber2Data);
  
  double dodI = (t2 - t1) / 1e-7;

  matrixDataScale<double> d1(ad, dodI * gain);
  bgrabber1->outputPartialDerivatives(sD, d1, sMode);

  double t3 = opptr(grabber1Data, grabber2Data + 1e-7);
  dodI = (t3 - t1) / 1e-7;
  d1.setScale(dodI * gain);
  bgrabber2->outputPartialDerivatives(sD, d1, sMode);

}
