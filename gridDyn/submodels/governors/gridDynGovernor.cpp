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

#include "submodels/otherGovernors.h"
#include "generators/gridDynGenerator.h"
#include "core/gridDynExceptions.h"
#include "objectFactoryTemplates.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "matrixDataSparse.h"


//Create the component factories for the various governors
static typeFactory<gridDynGovernor> gfgov1 ("governor", stringVec {"simple","fast"});

static childTypeFactory<gridDynGovernorIeeeSimple, gridDynGovernor> gfgovsi ("governor", stringVec { "basic", "ieeesimple" }, "basic");

static childTypeFactory<gridDynGovernorReheat, gridDynGovernor> gfgovrh ("governor", stringVec {"reheat"});
static childTypeFactory<gridDynGovernorHydro, gridDynGovernor> gfgov2 ("governor", stringVec {"ieeehydro", "hydro"});

static childTypeFactory<gridDynGovernorSteamNR, gridDynGovernor> gfgov3 ("governor", stringVec {"ieeesteamnr", "steamnr"});

static childTypeFactory<gridDynGovernorSteamTCSR, gridDynGovernor> gfgov4 ("governor", stringVec {"ieeesteamtcsr", "steamtcsr"});

static childTypeFactory<gridDynGovernorTgov1, gridDynGovernor> gfgov5 ("governor", stringVec {"tgov1"});

using namespace gridUnits;

gridDynGovernor::gridDynGovernor (const std::string &objName) : gridSubModel (objName), dbb ("deadbad"),cb (T1,"filter"), delay (T3,"outFilter")
{
  // default values
  cb.set ("bias", -1.0);
  dbb.set ("k", -K);
  dbb.setParent (this);
  cb.setParent (this);
  delay.setParent (this);
  // since they are members vs dynamic we set the blocks to own themselves
  dbb.setOwner (nullptr, &dbb);
  cb.setOwner (nullptr, &cb);
  delay.setOwner (nullptr, &delay);
}

coreObject *gridDynGovernor::clone (coreObject *obj) const
{
  gridDynGovernor *gov = cloneBase<gridDynGovernor, gridSubModel> (this, obj);
  if (!gov)
    {
      return obj;
    }
  gov->K = K;
  gov->T1 = T1;
  gov->T2 = T2;
  gov->T3 = T3;
  gov->Pmax = Pmax;
  gov->Pmin = Pmin;
  gov->Pset = Pset;
  gov->Wref = Wref;
  gov->deadbandHigh = deadbandHigh;
  gov->deadbandLow = deadbandLow;
  cb.clone (&(gov->cb));
  dbb.clone (&(gov->dbb));

  delay.clone (&(gov->delay));
  return gov;
}

// destructor
gridDynGovernor::~gridDynGovernor ()
{

}

void gridDynGovernor::objectInitializeA (gridDyn_time time0, unsigned long flags)
{
  prevTime = time0;
  if (Wref < 0)
    {
      Wref = m_baseFreq;
    }
  if (!opFlags[ignore_throttle])
    {
      subObjectList.push_back (&delay);    //delay block comes first to set the first state as the output
    }
  if (!opFlags[ignore_filter])
    {
      subObjectList.push_back (&cb);
    }
  if (!opFlags[ignore_deadband])
    {
      subObjectList.push_back (&dbb);
    }
  gridSubModel::objectInitializeA (time0, flags);

}
// initial conditions
static IOdata kNullVec;

void gridDynGovernor::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet)
{
  if (outputSet.empty ())
    {
      fieldSet[0] = args[govOmegaInLocation];
      cb.initializeB (fieldSet, kNullVec, fieldSet);
      dbb.initializeB (fieldSet, kNullVec, fieldSet);
      double omP = fieldSet[0];

      fieldSet[0] = Pset + omP;
      delay.initializeB (fieldSet, kNullVec, fieldSet);
      fieldSet[0] = Pset + omP;
    }
  else
    {

      double P = outputSet[0];
      fieldSet[0] = args[govOmegaInLocation];
      cb.initializeB (fieldSet, kNullVec, fieldSet);
      dbb.initializeB (fieldSet, kNullVec, fieldSet);
      double omP = fieldSet[0];

      fieldSet[0] = P;
      delay.initializeB (kNullVec, fieldSet, fieldSet);
      fieldSet.resize (2);
      fieldSet[1] = Pset = P - omP;
    }


}

// residual
void gridDynGovernor::residual (const IOdata &args, const stateData &sD, double resid[],  const solverMode &sMode)
{
  cb.residElements (args[govOmegaInLocation], 0,sD, resid, sMode);
  dbb.residElements (cb.getBlockOutput (sD,sMode), 0,sD, resid, sMode);
  delay.residElements (dbb.getBlockOutput (sD,sMode) + args[govpSetInLocation], 0,sD, resid,sMode);

}

void gridDynGovernor::timestep (gridDyn_time ttime,  const IOdata &args,const solverMode &)
{
  double out = cb.step (ttime, args[govOmegaInLocation]);

  out = dbb.step (ttime, out);
  delay.step (ttime, out + args[govpSetInLocation]);

}

void gridDynGovernor::derivative (const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode)
{
  IOdata i {
    args[govOmegaInLocation]
  };                                                         //deadband doesn't have any derivatives
  cb.derivative (i, sD, deriv, sMode);
  i[0] = dbb.getOutput (i, sD, sMode) + args[govpSetInLocation];       //gain from deadband +Pset
  delay.derivative (i, sD, deriv, sMode);
}


void gridDynGovernor::jacobianElements (const IOdata &args, const stateData &sD, matrixData<double> &ad,  const IOlocs &argLocs, const solverMode &sMode)
{
  cb.jacElements  (args[govOmegaInLocation], 0,sD, ad, argLocs[govOmegaInLocation], sMode);


  matrixDataSparse<double> kp;
  index_t wloc = cb.getOutputLoc(sMode);
  double out = cb.getOutput (kNullVec, sD, sMode);
  dbb.jacElements (out, 0,sD, ad, wloc, sMode);

  out = dbb.getOutput (kNullVec, sD, sMode);
  wloc = dbb.getOutputLoc(sMode);
  delay.jacElements (out + args[govpSetInLocation], 0,sD, kp, 0, sMode);

  if (argLocs[govpSetInLocation] != kNullLocation)
    {
      for (index_t pp = 0; pp < kp.size (); ++pp)
        {
          if (kp.colIndex (pp) == 0)
            {
              ad.assign (kp.rowIndex (pp), wloc, kp.val (pp));
            }
          else
            {
              ad.assign (kp.rowIndex (pp), kp.colIndex (pp), kp.val (pp));
            }
        }
    }
  else
    {
      for (index_t pp = 0; pp < kp.size (); ++pp)
        {
          if (kp.colIndex (pp) == 0)
            {
              ad.assign (kp.rowIndex (pp), wloc, kp.val (pp));
              ad.assign (kp.rowIndex (pp), argLocs[govpSetInLocation], kp.val (pp));
            }
          else
            {
              ad.assign (kp.rowIndex (pp), kp.colIndex (pp), kp.val (pp));
            }
        }

    }


  /*
  copyReplicate(matrixDataSparse *a2, index_t origCol, std::vector<index_t> newIndices, std::vector<double> mult)
  auto res = a2->dVec.begin();
        auto term = a2->dVec.end();

        while (res != term)
        {
                if (std::get<adCol>(*res) == origCol)
                {
                        for (index_t nn = 0; nn<newIndices.size(); ++nn)
                        {
                                //dVec.push_back(cLoc(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]));
                                dVec.emplace_back(std::get<adRow>(*res), newIndices[nn], std::get<adVal>(*res)*mult[nn]);
                        }
                }
                else
                {
                        dVec.push_back(*res);
                }
                ++res;
        }
        */
}


void gridDynGovernor::rootTest (const IOdata & /*args*/, const stateData &sD, double root[],  const solverMode &sMode)
{
  IOdata i {
    cb.getOutput (kNullVec, sD, sMode)
  };
  if (dbb.checkFlag (has_roots))
    {
      dbb.rootTest (i, sD, root, sMode);
    }
  //cb should not have roots
  if (delay.checkFlag (has_roots))
    {
      delay.rootTest (i, sD, root, sMode);
    }
}

index_t gridDynGovernor::findIndex (const std::string &field, const solverMode &sMode) const
{

  index_t ret = kInvalidLocation;
  if (field == "pm")
    {
      ret = delay.getOutputLoc (sMode,0);
    }
  else if (field == "dbo")
    {
      ret = dbb.getOutputLoc(sMode, 0);
    }
  else if (field == "w")
    {
      ret = cb.getOutputLoc(sMode, 0);
    }
  return ret;
}

void gridDynGovernor::setFlag (const std::string &flag, bool val)
{
	try
	{
		gridSubModel::setFlag(flag, val);
	}
	catch (const unrecognizedParameter &)
	{
		dbb.setFlag(flag, val);
	}
  
}
// set parameters
void gridDynGovernor::set (const std::string &param,  const std::string &val)
{
	try
	{
		gridSubModel::set(param, val);
	}
	catch (const unrecognizedParameter &)
	{
      dbb.set (param, val);
    }

}

void gridDynGovernor::set (const std::string &param, double val, units_t unitType)
{


  if (param == "k")
    {
      K = val;
      dbb.set (param, -K);
    }
  else if (param == "r")
    {
      K = 1 / val;
      dbb.set ("k", -K);
    }
  else if (param == "t1")
    {
      T1 = val;
      cb.set ("t1", val);
    }
  else if (param == "t2")
    {
      T2 = val;
      cb.set ("t2", val);
    }
  else if (param == "t3")
    {
      T3 = val;
      delay.set ("t1", val);
    }
  else if ((param == "omegaref") || (param == "wref"))
    {
      Wref = unitConversion (val,unitType,rps);
      //TODO:PT deal with changing reference frequency
    }
  else if (param == "pmax")
    {
      Pmax = unitConversion (val, unitType, puMW, systemBasePower);
      delay.set ("max", Pmax);
    }
  else if (param == "pmin")
    {
      Pmin = unitConversion (val, unitType, puMW, systemBasePower);
      delay.set ("min", Pmin);
    }
  else if (param == "deadband")
    {
      deadbandHigh = unitConversionFreq (val, unitType, puHz, m_baseFreq);
      if (deadbandHigh > 1.0)
        {
          deadbandHigh = deadbandHigh - 1.0;
        }
      deadbandLow = -deadbandHigh;
      dbb.set ("deadband", deadbandHigh);
    }
  else if (param == "deadbandhigh")
    {
      deadbandHigh = unitConversionFreq (val, unitType, puHz, m_baseFreq);
      if (deadbandHigh > 1.0)
        {
          deadbandHigh = deadbandHigh - 1.0;
        }
      dbb.set ("deadbandhigh", deadbandHigh);
    }
  else if (param == "deadbandlow")
    {
      deadbandLow = -unitConversionFreq (val, unitType, puHz,m_baseFreq);
      if (deadbandLow > 0.95)
        {
          deadbandLow = deadbandLow - 1.0;
        }
      if (deadbandLow > 0)
        {
          deadbandLow = -deadbandLow;
        }
      dbb.set ("deadbandhigh", deadbandLow);
    }
  else
    {
      gridSubModel::set (param, val, unitType);
    }

}

double gridDynGovernor::get (const std::string &param, gridUnits::units_t unitType) const
{
  double out = kNullVal;
  if (param == "k")
    {
      out = K;
    }
  else if (param == "r")
    {
      out = 1.0 / K;
    }
  else if (param == "t1")
    {
      out = T1;
    }
  else if (param == "t2")
    {
      out = T2;
    }
  else if (param == "t3")
    {
      out = T3;
    }
  else if ((param == "omegaref") || (param == "wref"))
    {
      out = unitConversion (Wref, rps, unitType);
      //TODO:PT deal with changing reference frequency
    }
  else if (param == "pmax")
    {
      out = unitConversion (Pmax, puMW,unitType, puMW, systemBasePower);
    }
  else if (param == "pmin")
    {
      out = unitConversion (Pmin, puMW, unitType, puMW, systemBasePower);
    }
  else if (param == "deadband")
    {

      out = unitConversionFreq (deadbandHigh,puHz, unitType, m_baseFreq);
    }
  else if (param == "deadbandhigh")
    {
      out = unitConversionFreq (deadbandHigh, puHz, unitType, m_baseFreq);
    }
  else if (param == "deadbandlow")
    {
      out = unitConversionFreq (deadbandHigh, puHz, unitType, m_baseFreq);
    }
  else
    {
      out = gridSubModel::get (param,unitType);
    }
  return out;
}






