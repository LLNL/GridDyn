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

// headers
#include "vectorOps.hpp"
#include "objectFactoryTemplates.h"
#include "linkModels/gridLink.h"
#include "linkModels/subsystem.h"
#include "gridCoreTemplates.h"
#include "gridBus.h"
#include "relays/gridRelay.h"
#include "objectInterpreter.h"
#include "stringOps.h"
#include <cmath>
#include <complex>

using namespace gridUnits;


static typeFactory<subsystem> gf ("link", std::vector<std::string>{ "subsystem", "simple" });

subsystem::subsystem (const std::string &objName) : gridLink (objName)
{
  resize (2);
  cterm[0] = 1;
  cterm[1] = 2;
  subarea.setParent (this);
  subarea.setOwner (nullptr, this);       //set the owner to this so nothing deletes it

  subObjectList.push_back (&subarea);       //add the subarea to the subobject list to take advantage of the code in gridPrimary.
}

subsystem::subsystem (count_t terminals, const std::string &objName) : gridLink (objName)
{
  // default values

  resize (terminals);
  if (terminals == 2)
    {
      cterm[0] = 1;
      cterm[1] = 2;
    }
  subarea.setParent (this);
  subarea.setOwner (nullptr,this);  //set the owner to this so nothing deletes it

  subObjectList.push_back (&subarea);  //add the subarea to the subobject list to take advantage of the code in gridPrimary.
}

gridCoreObject *subsystem::clone (gridCoreObject *obj) const
{
  subsystem *sub = cloneBase<subsystem, gridLink> (this, obj);
  if (!(sub))
    {
      return obj;
    }
  subarea.clone (&(sub->subarea));

  sub->resize (m_terminals);
  sub->cterm = cterm;

  //TODO:: find and copy the terminalLink information appropriately

  return sub;
}

// destructor
subsystem::~subsystem ()
{


}

int subsystem::add (gridCoreObject *obj)
{
  return subarea.add (obj);
}



// --------------- remove components ---------------

int subsystem::remove (gridCoreObject *obj)
{
  return subarea.remove (obj);
}



gridBus *subsystem::getBus (index_t num) const
{
  return subarea.getBus (num);
}

gridLink *subsystem::getLink (index_t num) const
{
  return subarea.getLink (num);
}


gridRelay *subsystem::getRelay (index_t num) const
{
  return subarea.getRelay (num);
}

gridArea *subsystem::getArea (index_t num) const
{
  return (num == 0) ? const_cast<gridArea *> (&subarea) : nullptr;
}

gridCoreObject *subsystem::find (const std::string &objname) const
{
  return subarea.find (objname);
}

gridCoreObject *subsystem::getSubObject (const std::string &typeName, index_t num) const
{
  return subarea.getSubObject (typeName, num);
}

void subsystem::setAll (const std::string &type, std::string param, double val, gridUnits::units_t unitType)
{
  subarea.setAll (type, param, val, unitType);

}

gridCoreObject *subsystem::findByUserID (const std::string &typeName, index_t searchID) const
{
  return subarea.findByUserID (typeName, searchID);
}



// reset the bus parameters
void subsystem::reset (reset_levels level)
{
  subarea.reset (level);
}

// initializeB states
void subsystem::pFlowObjectInitializeA (double time0, unsigned long flags)
{

  //make sure the buses are set to the right terminal
  for (index_t ii = 0; ii < m_terminals; ++ii)
    {
      if (terminalLink[ii])
        {
          terminalLink[ii]->updateBus (terminalBus[ii], cterm[ii]);
        }
    }

  return subarea.pFlowInitializeA (time0,flags);

}


void subsystem::setTime (double time)
{

  subarea.setTime (time);
}

void subsystem::updateLocalCache ()
{
  subarea.updateLocalCache ();
}

void subsystem::updateLocalCache (const stateData *sD, const solverMode &sMode)
{
  subarea.updateLocalCache (sD,sMode);
}

change_code subsystem::powerFlowAdjust (unsigned long flags, check_level_t level)
{
  return subarea.powerFlowAdjust (flags, level);
}

void subsystem::pFlowCheck (std::vector<violation> &Violation_vector)
{

  subarea.pFlowCheck (Violation_vector);
}

// initializeB states for dynamic solution
void subsystem::dynObjectInitializeA (double time0, unsigned long flags)
{
  return subarea.dynInitializeA (time0, flags);
}


void subsystem::converge (double ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode mode, double tol)
{
  subarea.converge (ttime, state, dstate_dt, sMode, mode, tol);
}

void subsystem::resize (count_t count)
{
  m_terminals = count;
  terminalBus.resize (count);
  terminalLink.resize (count);
  Pout.resize (count,0);
  Qout.resize (count,0);
  cterm.resize (count);
}

// set properties
int subsystem::set (const std::string &param,  const std::string &val)

{
  int out = PARAMETER_FOUND;
  std::string iparam;
  int num = trailingStringInt (param, iparam, -1);
  if (iparam == "bus")
    {
      gridBus *bus = dynamic_cast<gridBus *> (locateObject (val, parent));
      out = INVALID_PARAMETER_VALUE;
      if (bus)
        {
          if (num > static_cast<int> (m_terminals))
            {
              resize (num);
            }
          if (num <= 0)
            {
              num = 1;
              while (terminalBus[num - 1])
                {
                  ++num;
                  if (num > static_cast<int> (m_terminals))
                    {
                      resize (num);
                      break;
                    }
                }
            }
          updateBus (bus, num);
          out = PARAMETER_FOUND;

        }
    }
  else if (param == "from")
    {
      gridBus *bus = dynamic_cast<gridBus *> (locateObject (val, parent));
      out = INVALID_PARAMETER_VALUE;
      if (bus)
        {
          updateBus (bus, 1);
          out = PARAMETER_FOUND;
        }
    }
  else if (param == "to")
    {
      gridBus *bus = dynamic_cast<gridBus *> (locateObject (val, parent));
      out = INVALID_PARAMETER_VALUE;
      if (bus)
        {
          updateBus (bus, 2);
          out = PARAMETER_FOUND;
        }
    }
  else if (iparam == "connection")
    {
      auto pos1 = val.find_first_of (":,");
      index_t term1 = kNullLocation;
      if (pos1 != std::string::npos)
        {
          term1 = indexRead (val.substr (pos1 + 1),0);
        }
      gridLink *lnk = dynamic_cast<gridLink *> (locateObject (val, this,false));
      out = INVALID_PARAMETER_VALUE;
      if (lnk)
        {
          if (num > static_cast<int> (m_terminals))
            {
              resize (num);
            }
          if (num == 0)
            {
              num = 1;
              while (terminalLink[num - 1])
                {
                  ++num;
                  if (num > static_cast<int> (m_terminals))
                    {
                      resize (num);
                      break;
                    }
                }
            }

          terminalLink[num] = lnk;
          if (term1 >= 1)
            {
              if (term1 <= lnk->terminalCount ())
                {
                  cterm[num] = term1;
                }
            }
          else
            {
              for (count_t pp = 1; pp <= lnk->terminalCount (); ++pp)
                {
                  if (lnk->getBus (pp) == nullptr)
                    {
                      cterm[num] = pp;
                      break;
                    }
                }
            }
          if (cterm[num] > 0)
            {
              out = PARAMETER_FOUND;
            }

        }
    }
  else
    {
      out = gridPrimary::set (param, val);
      if (out != PARAMETER_FOUND)
        {
          out = subarea.set (param, val);
        }
    }
  return out;
}

int subsystem::set (const std::string &param, double val, units_t unitType)
{
  int out = PARAMETER_FOUND;

  if (param == "basepower")
    {
      systemBasePower = unitConversion (val, unitType, MW);

      subarea.set (param, systemBasePower);
    }
  else if ((param == "basefrequency") || (param == "basefreq"))
    {
      //the defunit in this case should be Hz since that is what everyone assumes but we
      //need it in rps NOTE: we only do this assumed conversion for an area/simulation
      if (unitType == defUnit)
        {
          unitType = Hz;
        }
      m_baseFreq = unitConversionFreq (val, unitType, rps);
      subarea.set (param, systemBasePower);
    }
  else if (param == "terminals")
    {
      resize (static_cast<count_t> (val));
    }
  else
    {
      out = gridPrimary::set (param, val, unitType);  //skipping gridLink set function
      if (out != PARAMETER_FOUND)
        {
          out = subarea.set (param, val, unitType);
        }
    }
  return out;
}


double subsystem::get (const std::string &param, units_t unitType) const
{
  double val = subarea.get (param,unitType);
  if (val == kNullVal)
    {
      val = gridPrimary::get (param, unitType);
    }
  return val;
}



double subsystem::timestep (const double ttime, const solverMode &sMode)
{

  subarea.timestep (ttime, sMode);
  prevTime = ttime;
  return 0;
}

count_t subsystem::getBusVector (std::vector<gridBus *> &busVector, index_t start)
{
  return subarea.getBusVector (busVector, start);
}



//single value return functions
double subsystem::getLoss () const
{
  return subarea.getLoss ();
}


// -------------------- Power Flow --------------------


// pass the solution
void subsystem::setState (const double ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{

  subarea.setState (ttime, state, dstate_dt, sMode);
  prevTime = ttime;
  updateLocalCache ();
  //next do any internal area states
}

void subsystem::getVoltageStates (double vStates[], const solverMode &sMode)

{
  subarea.getVoltageStates (vStates, sMode);
}

bool subsystem::switchTest () const
{
  bool sm = false;
  for (size_t kk = 0; kk < terminalLink.size (); ++kk)
    {
      sm |= terminalLink[kk]->switchTest (cterm[kk]);
    }
  return sm;
}

bool subsystem::switchTest (index_t num) const
{
  if (num <= m_terminals)
    {
      return terminalLink[num - 1]->switchTest (cterm[num - 1]);
    }
  else
    {
      return false;
    }
}
void subsystem::switchMode (index_t num, bool mode)
{
  if (num <= m_terminals)
    {
      terminalLink[num - 1]->switchMode (cterm[num - 1],mode);
    }
}
//is connected
bool subsystem::isConnected () const
{
  bool con = true;
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      con &= (terminalLink[kk]->isConnected ());
    }
  return con;
}

int subsystem::fixRealPower (double power, index_t mterminal, index_t /*fixedterminal*/, gridUnits::units_t unitType)
{
  if (mterminal <= m_terminals)
    {
      return terminalLink[mterminal - 1]->fixRealPower (power, cterm[mterminal - 1],unitType);
    }
  return 0;
}

int subsystem::fixPower (double rPower, double qPower, index_t mterminal, index_t /*fixedterminal*/, gridUnits::units_t unitType)
{
  if (mterminal <= m_terminals)
    {
      return terminalLink[mterminal - 1]->fixPower (rPower,qPower, cterm[mterminal - 1], unitType);
    }
  return 0;
}

void subsystem::followNetwork (int network, std::queue<gridBus *> &stk)
{
  terminalLink[0]->followNetwork (network,stk);
}

int subsystem::updateBus (gridBus *bus, index_t busnumber)
{
  if (busnumber <= m_terminals)
    {
      int ret = terminalLink[busnumber - 1]->updateBus (bus,cterm[busnumber - 1]);
      if (ret != OBJECT_ADD_FAILURE)
        {
          terminalBus[busnumber - 1] = bus;
        }
      return ret;
    }
  else
    {
      if (opFlags.test (direct_connection))
        {
          return gridLink::updateBus (bus, busnumber);
        }
      else
        {
          return OBJECT_ADD_FAILURE;
        }

    }
}


double subsystem::quickupdateP ()
{
  return 0;
}

double subsystem::remainingCapacity () const
{
  return terminalLink[0]->remainingCapacity ();
}

double subsystem::getAngle () const
{
  const double t1 = terminalBus[0]->getAngle ();
  double t2 = terminalBus[m_terminals - 1]->getAngle ();
  return t1 - t2;
}

double subsystem::getAngle (const double state[], const solverMode &sMode) const
{
  double t1 = terminalBus[0]->getAngle (state, sMode);
  double t2 = terminalBus[m_terminals - 1]->getAngle (state, sMode);
  return t1 - t2;
}

double subsystem::getAbsAngle (index_t busId) const
{
  if ((busId == 1) || (busId == terminalBus[0]->getID ()))
    {
      return terminalBus[m_terminals - 1]->getAngle ();
    }
  else
    {
      return terminalBus[0]->getAngle ();
    }
}

double subsystem::getRealImpedance (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          double vb = terminalBus[kk]->getVoltage ();
          std::complex<double> Z = (vb * vb) / std::complex<double> (Pout[kk], Qout[kk]);
          return std::isnormal (Z.real ()) ? Z.real () : kBigNum;
        }
    }
  return kBigNum;
}

double subsystem::getImagImpedance (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          double vb = terminalBus[kk]->getVoltage ();
          std::complex<double> Z = (vb * vb) / std::complex<double> (Pout[kk], Qout[kk]);
          return std::isnormal (Z.imag ()) ? Z.imag () : kBigNum;
        }
    }
  return kBigNum;
}
double subsystem::getTotalImpedance (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          double vp = terminalBus[kk]->getVoltage ();
          // printf("id1 impedance=%f\n", signn(linkInfo.P1 + linkInfo.Q1)*(linkInfo.v1*linkInfo.v1) / std::hypot(linkInfo.P1, linkInfo.Q1));
          double val = signn (Pout[kk] + Qout[kk]) * (vp * vp) / std::hypot (Pout[kk], Qout[kk]);
          return (std::isnormal (val) ? val : kBigNum);
        }
    }
  return kBigNum;
}

double subsystem::getCurrent (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          return std::hypot (Qout[kk], Pout[kk]) / terminalBus[kk]->getVoltage ();
        }
    }
  return 0;
}
double subsystem::getRealCurrent (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          return Pout[kk] / terminalBus[kk]->getVoltage ();
        }
    }
  return 0;
}
double subsystem::getImagCurrent (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          return Qout[kk] / terminalBus[kk]->getVoltage ();
        }
    }
  return 0;
}

double subsystem::getRealPower (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          return Pout[kk];
        }
    }
  return 0;
}     //function to return the real flow in
double subsystem::getReactivePower (index_t busId) const
{
  if (busId == kNullLocation)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          return Qout[kk];
        }
    }
  return 0;
}      //function to return the reactive power in

double subsystem::getReactiveLoss () const
{
  return std::abs (sum (Qout));
}

double subsystem::getMaxTransfer () const
{
  return 0;
}


// initializeB power flow


//for computing all the Jacobian elements at once
void subsystem::ioPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode)
{
  if  (busId <= 0)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          terminalLink[kk]->ioPartialDerivatives (cterm[kk],sD,ad,argLocs,sMode);
          break;
        }
    }
}

void subsystem::outputPartialDerivatives (index_t busId, const stateData *sD, arrayData<double> *ad, const solverMode &sMode)
{
  if (busId <= 0)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          terminalLink[kk]->outputPartialDerivatives (cterm[kk], sD, ad,  sMode);
          break;
        }
    }
}

IOdata subsystem::getOutputs (const stateData *sD, const solverMode &sMode)
{
  return getOutputs (1, sD, sMode);
}
//virtual void busResidual(index_t busId, const stateData *sD, double *Fp, double *Fq, const solverMode &sMode);
IOdata subsystem::getOutputs (index_t busId, const stateData *sD, const solverMode &sMode)
{
  updateLocalCache (sD, sMode);
  IOdata out {
    Pout[0], Qout[0]
  };

  if (busId <= 0)
    {
      busId = 1;
    }
  for (index_t kk = 0; kk < m_terminals; ++kk)
    {
      if ((busId == kk + 1) || (busId == terminalBus[kk]->getID ()))
        {
          out[PoutLocation] = Pout[kk];
          out[QoutLocation] = Qout[kk];
          break;
        }
    }
  return out;
}
