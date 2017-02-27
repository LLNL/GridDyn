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

#include "loadModels/compositeLoad.h"
#include "core/coreObjectTemplates.h"
#include "gridBus.h"
#include "core/objectFactoryTemplates.h"
#include "core/coreExceptions.h"
#include "stringConversion.h"

#include <cmath>

static typeFactory<compositeLoad> glfld ("load", stringVec {"composite","cluster","group"});

using namespace stringOps;
compositeLoad::compositeLoad (const std::string &objName) : zipLoad (objName)
{
  add (new zipLoad (getName() + "sub"));

}

coreObject * compositeLoad::clone (coreObject *obj) const
{
  compositeLoad *nobj = cloneBase<compositeLoad, zipLoad> (this, obj);
  if (!(nobj))
    {
      return obj;
    }
  nobj->consumeSimpleLoad = consumeSimpleLoad;
  nobj->fraction = fraction;

  for (auto &ld : subLoads)
    {
      nobj->add (ld->clone (nullptr));
    }

  return nobj;
}

void compositeLoad::add (zipLoad *ld)
{
  if (ld->locIndex != kNullLocation)
    {
      if (subLoads.size () <= ld->locIndex)
        {
          subLoads.resize (ld->locIndex + 1,nullptr);
          fraction.resize (ld->locIndex + 1,-1);
        }
      subLoads[ld->locIndex] = ld;
	  addSubObject(ld);
      
    }
  else
    {
      subLoads.push_back (ld);
      fraction.push_back (-1.0);
      ld->locIndex = static_cast<index_t> (subLoads.size ()) - 1;
	  addSubObject(ld);
    }
}

void compositeLoad::add (coreObject *obj)
{
  zipLoad *ld = dynamic_cast<zipLoad *> (obj);
  if (ld)
    {
      return add (ld);
    }
  else
    {
	  throw(unrecognizedObjectException(this));
    }
}

void compositeLoad::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{
	//TODO::Need to rethink this objects
  zipLoad::pFlowInitializeA (time0,flags);
  if (consumeSimpleLoad)
    {

      int nLoads = bus->getInt ("loadcount");


      if (nLoads == 0)
        {
          return;
        }
      zipLoad *sLoad = nullptr;
      zipLoad *testLoad = nullptr;
      double mxP = 0;
      for (int kk = 0; kk < nLoads; ++kk)
        {
          testLoad = static_cast<zipLoad *> (getParent()->getSubObject ("load",kk));
          if (testLoad->getID () == getID ())
            {
              continue;
            }
          if (std::abs (testLoad->getRealPower ()) > mxP)
            {
              mxP = std::abs (testLoad->getRealPower ());
              sLoad = testLoad;
            }
        }
      if (!sLoad)
        {
          return;
        }
      //do a first pass of loading
      double rem = 1.0;
      setP(((compositeLoad *)sLoad)->getP());              //so this composite load function has direct access to the zipLoad variables seems a little odd to need to do this but seems to be a requirement in C++
      setQ(((compositeLoad *)sLoad)->getQ());
      setIp(((compositeLoad *)sLoad)->getIp());
      setIq(((compositeLoad *)sLoad)->getIq());
      setYp(((compositeLoad *)sLoad)->getYp());
      setYq(((compositeLoad *)sLoad)->getYq());

      for (size_t nn = 0; nn < subLoads.size (); ++nn)
        {
          if (fraction[nn] > 0)
            {
              subLoads[nn]->set ("p", getP() * fraction[nn]);
              subLoads[nn]->set ("q", getQ() * fraction[nn]);
              subLoads[nn]->set ("ip", getIp() * fraction[nn]);
              subLoads[nn]->set ("iq", getIq() * fraction[nn]);
              subLoads[nn]->set ("yp", getYp() * fraction[nn]);
              subLoads[nn]->set ("yq", getYq() * fraction[nn]);
              rem -= fraction[nn];
            }
        }
      double remnegcnt = 0;
      for (auto &sL : fraction)
        {
          if (sL < 0)
            {
              remnegcnt += 1.0;
            }
        }
      if (remnegcnt > 0)
        {
          mxP = rem / remnegcnt;
          for (size_t nn = 0; nn < subLoads.size (); ++nn)
            {
              if (fraction[nn] < 0)
                {
                  subLoads[nn]->set ("p", getP() * mxP);
                  subLoads[nn]->set ("q", getQ() * mxP);
                  subLoads[nn]->set ("ip", getIp() * mxP);
                  subLoads[nn]->set ("iq", getIq() * mxP);
                  subLoads[nn]->set ("yp", getYp() * mxP);

                  subLoads[nn]->set ("yq", getYq() * mxP);
                  fraction[nn] = mxP;
                }
            }
        }
      testLoad->disable ();
    }

  for (auto &ld : subLoads)
    {
      ld->pFlowInitializeA (time0, flags);
    }
}

void compositeLoad::pFlowObjectInitializeB ()
{
  for (auto &ld:subLoads)
    {
      ld->pFlowInitializeB ();
    }
}

void compositeLoad::dynObjectInitializeA (coreTime time, unsigned long flags)
{
  for (auto &ld : subLoads)
    {
      ld->dynInitializeA (time,flags);
    }
}

void compositeLoad::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet)
{
  IOdata out = desiredOutput;

  for (auto &ld : subLoads)
    {
      ld->dynInitializeB (inputs,out,fieldSet);

    }
}


void compositeLoad::set (const std::string &param,  const std::string &val)
{

  std::string iparam;
  int num = trailingStringInt (param, iparam,-1);
  double frac = -1.0;
  if (iparam == "subload")
    {
      zipLoad *Ld;
      auto strSplit = splitline(val);
	  trim(strSplit);
      auto load_factory = coreObjectFactory::instance ()->getFactory ("load");
      size_t nn = 0;
      while (nn < strSplit.size ())
        {
          if (load_factory->isValidType (strSplit[nn]))
            {

              Ld = static_cast<zipLoad *> (load_factory->makeObject (strSplit[nn]));
              Ld->locIndex = num;
              add (Ld);
              ++nn;
              try
                {
                  if (nn < strSplit.size ())
                    {
                      frac = std::stod (strSplit[nn]);
                      ++nn;
                    }
                  else
                    {
                      frac = -1.0;
                    }
                }
              catch (std::invalid_argument)
                {
                  frac = -1.0;
                }
              if (num > 0)
                {
                  if (static_cast<int> (fraction.size ()) < num)
                    {
                      fraction.resize (num, -1);
                    }
                  fraction[num] = frac;
                }
              else
                {
                  fraction.push_back (frac);
                }
            }
        }
    }
  else if (param == "fraction")
    {
      auto fval = str2vector<double> (val,-1.0);
      for (size_t nn = 0; nn < fval.size (); ++nn)
        {
          if (nn + 1 >= fraction.size ())
            {
              LOG_WARNING ("fraction specification count exceeds load count");
              break;
            }
          fraction[nn + 1] = fval[nn];
        }
      if (opFlags[pFlow_initialized])
        {
          for (size_t nn = 0; nn < subLoads.size (); ++nn)
            {
              subLoads[nn]->set ("p", getP() * fraction[nn]);
              subLoads[nn]->set ("q", getQ() * fraction[nn]);
              subLoads[nn]->set ("ir", getIp() * fraction[nn]);
              subLoads[nn]->set ("iq", getIq() * fraction[nn]);
              subLoads[nn]->set ("yp", getYp() * fraction[nn]);
              subLoads[nn]->set ("Yq", getYq() * fraction[nn]);
            }
        }
    }
  else
    {
      zipLoad::set (param,val);
    }
  
}
void compositeLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  
  std::string iparam;
  int num = trailingStringInt (param, iparam, -1);
  bool reallocate = true;
  if (param == "consume")
    {
      consumeSimpleLoad = (val > 0);
      reallocate = false;
    }
  else if (iparam == "fraction")
    {
      if (num <= 0)
        {
          num = 1;
        }
      if (num >= static_cast<int> (fraction.size ()))
        {
          LOG_WARNING ("fraction specification count exceeds load count");
		  throw(invalidParameterValue());
        }
      else
        {
          fraction[num] = val;
        }
    }
  else
    {
      zipLoad::set (param, val,unitType);
    }
  if ((reallocate)&&(opFlags[pFlow_initialized]))
    {
      for (size_t nn = 0; nn < subLoads.size (); ++nn)
        {
          subLoads[nn]->set ("p", getP() * fraction[nn]);
          subLoads[nn]->set ("q", getQ() * fraction[nn]);
          subLoads[nn]->set ("ir", getIp() * fraction[nn]);
          subLoads[nn]->set ("iq", getIq() * fraction[nn]);
          subLoads[nn]->set ("yp", getYp() * fraction[nn]);
          subLoads[nn]->set ("yq", getYq() * fraction[nn]);
        }
    }

}

void compositeLoad::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->residual (inputs, sD,resid,sMode);
        }
    }
}

void compositeLoad::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->diffSize (sMode) > 0)
        {
          ld->derivative (inputs, sD, deriv, sMode);
        }
    }
}

void compositeLoad::outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->outputPartialDerivatives (inputs, sD, ad, sMode);
        }
    }
}

void compositeLoad::ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  for  (auto &ld : subLoads)
    {
      ld->ioPartialDerivatives (inputs, sD, ad, inputLocs,sMode);
    }
}

void compositeLoad::jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  for  (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->jacobianElements (inputs, sD, ad,inputLocs,sMode);
        }
    }
}

void compositeLoad::timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      ld->timestep (ttime,inputs, sMode);
    }
}

double compositeLoad::getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getRealPower (inputs, sD, sMode);
        }
    }
  return rp;
}

double compositeLoad::getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getReactivePower (inputs, sD, sMode);
        }
    }
  return rp;
}

double compositeLoad::getRealPower (double V) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getRealPower (V);
        }
    }
  return rp;
}

double compositeLoad::getReactivePower (double V) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getReactivePower (V);
        }
    }
  return rp;
}

double compositeLoad::getRealPower () const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getRealPower ();
        }
    }
  return rp;
}

double compositeLoad::getReactivePower () const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->isConnected())
        {
          rp += ld->getReactivePower ();
        }
    }
  return rp;
}
