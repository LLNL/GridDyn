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

#include "loadModels/compositeLoad.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "objectFactoryTemplates.h"
#include "core/gridDynExceptions.h"
#include "stringConversion.h"

#include <cmath>

static typeFactory<compositeLoad> glfld ("load", stringVec {"composite","cluster","group"});


compositeLoad::compositeLoad (const std::string &objName) : gridLoad (objName)
{
  add (new gridLoad (name + "sub"));

}

coreObject * compositeLoad::clone (coreObject *obj) const
{
  compositeLoad *nobj = cloneBase<compositeLoad, gridLoad> (this, obj);
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

void compositeLoad::add (gridLoad *ld)
{
  if (ld->locIndex != kNullLocation)
    {
      if (subLoads.size () <= ld->locIndex)
        {
          subLoads.resize (ld->locIndex + 1,nullptr);
          fraction.resize (ld->locIndex + 1,-1);
        }
      subLoads[ld->locIndex] = ld;
      ld->setParent (this);
      subObjectList.push_back (ld);//don't care what order subObjectList is in
    }
  else
    {
      subLoads.push_back (ld);
      subObjectList.push_back (ld);
      fraction.push_back (-1.0);
      ld->setParent (this);
      ld->locIndex = static_cast<index_t> (subLoads.size ()) - 1;
    }
}

void compositeLoad::add (coreObject *obj)
{
  gridLoad *ld = dynamic_cast<gridLoad *> (obj);
  if (ld)
    {
      return add (ld);
    }
  else
    {
	  throw(invalidObjectException(this));
    }
}

void compositeLoad::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  gridLoad::pFlowInitializeA (time0,flags);
  if (consumeSimpleLoad)
    {

      int nLoads = bus->getInt ("loadcount");


      if (nLoads == 0)
        {
          return;
        }
      gridLoad *sLoad = nullptr;
      gridLoad *testLoad = nullptr;
      double mxP = 0;
      for (int kk = 0; kk < nLoads; ++kk)
        {
          testLoad = static_cast<gridLoad *> (parent->getSubObject ("load",kk));
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
      setP(((compositeLoad *)sLoad)->getP());              //so this composite load function has direct access to the gridLoad variables seems a little odd to need to do this but seems to be a requirement in C++
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

void compositeLoad::dynObjectInitializeA (gridDyn_time time, unsigned long flags)
{
  for (auto &ld : subLoads)
    {
      ld->dynInitializeA (time,flags);
    }
}

void compositeLoad::dynObjectInitializeB (const IOdata &args, const IOdata &outputSet)
{
  IOdata out = outputSet;

  for (auto &ld : subLoads)
    {
      ld->dynInitializeB (args,out);

    }
}


void compositeLoad::set (const std::string &param,  const std::string &val)
{

  std::string iparam;
  int num = trailingStringInt (param, iparam,-1);
  double frac = -1.0;
  if (iparam == "subload")
    {
      gridLoad *Ld;
      auto strSplit = splitlineTrim (val);
      auto load_factory = coreObjectFactory::instance ()->getFactory ("load");
      size_t nn = 0;
      while (nn < strSplit.size ())
        {
          if (load_factory->isValidType (strSplit[nn]))
            {

              Ld = static_cast<gridLoad *> (load_factory->makeObject (strSplit[nn]));
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
      gridLoad::set (param,val);
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
      gridLoad::set (param, val,unitType);
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

void compositeLoad::residual (const IOdata &args, const stateData &sD, double resid[], const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->residual (args, sD,resid,sMode);
        }
    }
}

void compositeLoad::derivative (const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->diffSize (sMode) > 0)
        {
          ld->derivative (args, sD, deriv, sMode);
        }
    }
}

void compositeLoad::outputPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->outputPartialDerivatives (args, sD, ad, sMode);
        }
    }
}

void compositeLoad::ioPartialDerivatives (const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
  for  (auto &ld : subLoads)
    {
      ld->ioPartialDerivatives (args, sD, ad, argLocs,sMode);
    }
}

void compositeLoad::jacobianElements (const IOdata &args, const stateData &sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
  for  (auto &ld : subLoads)
    {
      if (ld->stateSize (sMode) > 0)
        {
          ld->jacobianElements (args, sD, ad,argLocs,sMode);
        }
    }
}

void compositeLoad::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &sMode)
{
  for (auto &ld : subLoads)
    {
      ld->timestep (ttime,args, sMode);
    }
}

double compositeLoad::getRealPower (const IOdata &args, const stateData &sD, const solverMode &sMode) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->enabled)
        {
          rp += ld->getRealPower (args, sD, sMode);
        }
    }
  return rp;
}

double compositeLoad::getReactivePower (const IOdata &args, const stateData &sD, const solverMode &sMode) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->enabled)
        {
          rp += ld->getReactivePower (args, sD, sMode);
        }
    }
  return rp;
}

double compositeLoad::getRealPower (double V) const
{
  double rp = 0;
  for (auto &ld : subLoads)
    {
      if (ld->enabled)
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
      if (ld->enabled)
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
      if (ld->enabled)
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
      if (ld->enabled)
        {
          rp += ld->getReactivePower ();
        }
    }
  return rp;
}
