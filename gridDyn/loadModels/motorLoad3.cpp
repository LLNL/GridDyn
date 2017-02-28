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

#include "loadModels/motorLoad.h"
#include "gridBus.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "core/coreObjectTemplates.h"
#include <iostream>

using namespace gridUnits;

//setup the load object factories



motorLoad3::motorLoad3 (const std::string &objName) : motorLoad (objName)
{

}

coreObject *motorLoad3::clone (coreObject *obj) const
{
  motorLoad3 *ld = cloneBase<motorLoad3, motorLoad> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  return ld;
}


void motorLoad3::pFlowObjectInitializeA (coreTime time0, unsigned long flags)
{
//setup the parameters
  x0 = x + xm;
  xp = x + x1 * xm / (x1 + xm);
  T0p = (x1 + xm) / (m_baseFreq * r1);
  scale = mBase / systemBasePower;
  m_state.resize (5,0);
  if (opFlags[init_transient])
    {
      m_state[2] = init_slip;
    }
  else if (Pmot > -kHalfBigNum)
    {
      m_state[2] = computeSlip (Pmot / scale);
    }
  else
    {
      m_state[2] = 1.0;
      opFlags.set (init_transient);
    }

  gridLoad::pFlowObjectInitializeA (time0, flags);
  converge ();

  loadSizes (cLocalSolverMode,false);
  setOffset (0,cLocalSolverMode);

}

void motorLoad3::converge ()
{
  double V = bus->getVoltage ();
  double theta = bus->getAngle ();
  double slip = m_state[2];
  double Qtest = qPower (V, m_state[2]);
  double im,ir;
  double er,em;

  double Vr = -V *Vcontrol* sin (theta);
  double Vm = V * Vcontrol * cos (theta);
  solve2x2 (Vr,Vm,Vm,-Vr,Pmot / scale,Qtest,ir,im);
  double err = 10;
  double slipp = slip;
  int ccnt = 0;
  double perr = 10;
  double dslip = 0;
  while (err > 1e-6)
    {
      er = Vr - r * ir + xp * im;
      em = Vm - r * im - xp * ir;
      slipp = (er + (x0 - xp) * im) / T0p / m_baseFreq / em;
      dslip = slipp - slip;
      if (Pmot > 0)
        {
          if (slipp < 0)
            {
              slip = slip / 2.0;
            }
          else
            {
              slip = slipp;
            }
        }

      err = std::abs (dslip);
      if (err > perr)
        {
          break;
        }
      //just archiving the states in case we need to break;
      m_state[0] = ir;
      m_state[1] = im;
      m_state[2] = slip;
      m_state[3] = er;
      m_state[4] = em;
      if (++ccnt > 50)
        {
          break;
        }

      perr = err;
      ir = (-m_baseFreq * slip * er * T0p - em) / (-(x0 - xp));
      im = (mechPower (slip) - er * ir) / em;
    }

}

void motorLoad3::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{

}

void motorLoad3::dynObjectInitializeB (const IOdata &inputs, const IOdata & /*desiredOutput*/, IOdata &/*fieldSet*/)
{
  if (opFlags[init_transient])
    {
      derivative (inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
    }

}



void motorLoad3::loadSizes (const solverMode &sMode, bool /*dynOnly*/)
{
  auto so = offsets.getOffsets (sMode);
  so->reset ();
  if (isDynamic (sMode))
    {
      so->total.algSize = 2;
      so->total.jacSize = 8;
      if ((opFlags[stalled]) && (opFlags[resettable]))
        {
          so->total.algRoots = 1;
          opFlags.set (has_alg_roots);
        }
      else
        {
          so->total.diffRoots = 1;
          opFlags.reset (has_alg_roots);
        }

      if (!isAlgebraicOnly (sMode))
        {
          so->total.diffSize = 3;
          so->total.jacSize += 15;

        }
    }
  else
    {
      so->total.algSize = 5;
      if (opFlags[init_transient])
        {
          so->total.jacSize = 19;
        }
      else
        {
          so->total.jacSize = 23;
        }
    }
  so->rjLoaded = true;
  so->stateLoaded = true;

}

// set properties
void motorLoad3::set (const std::string &param,  const std::string &val)
{

  if (param[0] == '#')
    {

    }
  else
    {
      motorLoad::set (param, val);
    }

}

void motorLoad3::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "rs")
    {
      r = val;
    }
  else
    {
      motorLoad::set (param, val, unitType);
    }


}

void motorLoad3::setState (coreTime ttime, const double state[], const double dstate_dt[], const solverMode &sMode)
{
  gridObject::setState (ttime,state,dstate_dt,sMode);
}

void motorLoad3::guess (coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode)
{
  gridObject::guess (ttime,state,dstate_dt,sMode);
}

// residual
void motorLoad3::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations (sD, resid, sMode, this);


      double V = inputs[voltageInLocation];
      double theta = inputs[angleInLocation];
      const double *gm = Loc.algStateLoc;
      const double *gmd = Loc.diffStateLoc;
      const double *gmp = Loc.dstateLoc;

      double *rva = Loc.destLoc;
      double *rvd = Loc.destDiffLoc;
      double Vr, Vm;

      Vr = -V *Vcontrol* sin (theta);
      Vm = V * Vcontrol * cos (theta);


      //ir
      rva[0] = Vm - gmd[2] - r * gm[1] - xp * gm[0];
      //im
      rva[1] = Vr - gmd[1] - r * gm[0] + xp * gm[1];

      if (isAlgebraicOnly (sMode))
        {
          return;
        }
      derivative (inputs, sD, resid, sMode);
      //Get the exciter field

      // delta
      rvd[0] -= gmp[0];
      rvd[1] -= gmp[1];
      rvd[2] -= gmp[2];
      // printf("t=%f:motor state a1=%f a2=%f, d1=%f, d2=%f,d3=%f\n", sD.time, gm[0], gm[1], gmd[0], gmd[1], gmd[2]);
      // printf("t=%f:motor resid a1=%e a2=%e, d1=%e, d2=%e,d3=%e\n",sD.time,rva[0],rva[1],rvd[0],rvd[1],rvd[2]);
      // printf("t=%f, V=%f, ir=%f, im=%f, r1=%e, r2=%e\n",sD.time,V,gm[0],gm[1],rva[0],rva[1]);
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      const double V = inputs[voltageInLocation];
      double theta = inputs[angleInLocation];

      const double *gm = sD.state + offset;
      double *rv = resid + offset;
      double Vr, Vm;

      Vr = -V *Vcontrol* sin (theta);
      Vm = V * Vcontrol * cos (theta);

      //ir
      rv[0] = Vm - gm[4] - r * gm[1] - xp * gm[0];
      //im
      rv[1] = Vr - gm[3] - r * gm[0] + xp * gm[1];

      double slip = gm[2];
      // printf("angle=%f, slip=%f\n",theta,slip);
      // slip
      if (opFlags[init_transient])
        {
          rv[2] = slip - m_state[2];
        }
      else
        {

          double Te = gm[3] * gm[0] + gm[4] * gm[1];
          rv[2] = (mechPower (slip) - Te) / (2 * H);
        }
      // Erp and Emp
      rv[3] = m_baseFreq * slip * gm[4] - (gm[3] + (x0 - xp) * gm[1]) / T0p;
      rv[4] = -m_baseFreq * slip * gm[3] - (gm[4] - (x0 - xp) * gm[0]) / T0p;

    }

}

void motorLoad3::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  std::string prefix2 = prefix + getName();
  if (isDynamic (sMode))
    {
      if (isAlgebraicOnly (sMode))
        {
          return;
        }
      auto offsetA = offsets.getAlgOffset (sMode);
      auto offsetD = offsets.getDiffOffset (sMode);
      stNames[offsetA] = prefix2 + ":ir";
      stNames[offsetA + 1] = prefix2 + ":im";
      stNames[offsetD] = prefix2 + ":slip";
      stNames[offsetD + 1] = prefix2 + ":erp";
      stNames[offsetD + 2] = prefix2 + ":emp";
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      stNames[offset] = prefix2 + ":ir";
      stNames[offset + 1] = prefix2 + ":im";
      stNames[offset + 2] = prefix2 + ":slip";
      stNames[offset + 3] = prefix2 + ":erp";
      stNames[offset + 4] = prefix2 + ":emp";
    }

}


void motorLoad3::timestep (coreTime ttime, const IOdata &inputs, const solverMode &)
{
  stateData sD(ttime,m_state.data());
  derivative (inputs, sD, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[2] += dt * m_dstate_dt[2];
  m_state[3] += dt * m_dstate_dt[3];
  m_state[4] += dt * m_dstate_dt[4];
  prevTime = ttime;
  updateCurrents (inputs, sD, cLocalSolverMode);
}

void motorLoad3::updateCurrents (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{


  Lp Loc = offsets.getLocations (sD, const_cast<double *> (sD.state), sMode, this);
  double V = inputs[voltageInLocation];
  double theta = inputs[angleInLocation];

  double vr, vm;
  vr = -V*Vcontrol*sin (theta);
  vm = V * Vcontrol * cos (theta);

  solve2x2 (r, -xp, xp, r, vr - Loc.diffStateLoc[1], vm - Loc.diffStateLoc[2], Loc.destLoc[0], Loc.destLoc[1]);

}

void motorLoad3::derivative (const IOdata & /*inputs*/, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, deriv, sMode, this);
  const double *ast = Loc.algStateLoc;
  const double *dst = Loc.diffStateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double slip = dst[0];

  //if (sD.time>=1.0)
  // {
  //  mechPower(slip);
  //}

  // slip
  if (opFlags[stalled])
    {
      dv[0] = 0;
    }
  else
    {
      double Te = dst[1] * ast[0] + dst[2] * ast[1];
      dv[0] = (mechPower (slip) - Te) / (2 * H);
    }
  // printf("t=%f, slip=%f mp=%f, te=%f, dslip=%e\n", sD.time, slip,mechPower(slip), Te,dv[0] );
  // Edp and Eqp
  dv[1] = m_baseFreq * slip * dst[2] - (dst[1] + (x0 - xp) * ast[1]) / T0p;
  dv[2] = -m_baseFreq * slip * dst[1] - (dst[2] - (x0 - xp) * ast[0]) / T0p;

}


void motorLoad3::jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  index_t refAlg,refDiff;
  const double *gm,*dst;
  double cj = sD.cj;
  if  (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);

      refAlg = Loc.algOffset;
      refDiff = Loc.diffOffset;
      gm = Loc.algStateLoc;
      dst = Loc.diffStateLoc;
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      refAlg = offset;
      refDiff = offset + 2;
      gm = sD.state + offset;
      dst = sD.state + offset + 2;
      cj = 0;
    }

  double V = inputs[voltageInLocation];
  double theta = inputs[angleInLocation];
  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];

  double Vr = -V *Vcontrol* sin (theta);
  double Vm = V * Vcontrol * cos (theta);



  //ir
  // rva[0] = Vm - gmd[2] - r*gm[1] - xp*gm[0];
  //im
  //rva[1] = Vr - gmd[1] - r*gm[0] + xp*gm[1];

  // P
  if (TLoc != kNullLocation)
    {
      ad.assign (refAlg, TLoc, Vr);
      ad.assign (refAlg + 1, TLoc, -Vm);
    }
  // Q
  if (VLoc != kNullLocation)
    {
      ad.assign (refAlg, VLoc, Vm / V);
      ad.assign (refAlg + 1, VLoc, Vr / V);
    }

  ad.assign (refAlg, refAlg, -xp);
  ad.assign (refAlg, refAlg + 1, -r);

  ad.assign (refAlg + 1, refAlg, -r);
  ad.assign (refAlg + 1, refAlg + 1, xp);
  if ((isDynamic (sMode))&&(isAlgebraicOnly (sMode)))
    {
      return;
    }
  // Ir Differential

  ad.assign (refAlg, refDiff + 2, -1.0);
  // Im Differential
  ad.assign (refAlg + 1, refDiff + 1, -1.0);


  double slip = dst[0];
  if ((isDynamic (sMode))||(!opFlags[init_transient]))
    {
      /*
      // slip
      double Te = dst[1] * ast[0] + dst[2] * ast[1];
      dv[0] = (mechPower(slip) - Te) / (2 * H);

      */
      // slip
      if (opFlags[stalled])
        {
          ad.assign (refDiff,refDiff,-cj);
        }
      else
        {
          ad.assign (refDiff, refDiff, dmechds (slip) / (2.0 * H) - cj);
          ad.assign (refDiff, refDiff + 1, -gm[0] / (2.0 * H));
          ad.assign (refDiff, refDiff + 2, -gm[1] / (2.0 * H));
          ad.assign (refDiff, refAlg, -dst[1] / (2.0 * H));
          ad.assign (refDiff, refAlg + 1, -dst[2] / (2.0 * H));
        }
    }
  else
    {
      ad.assign (refDiff,refDiff,1.0);
    }
  // omega

  // Edp and Eqp
  //dv[1] = m_baseFreq*slip*dst[2] - (dst[1] + (x0 - xp)*ast[1]) / T0p;
  //dv[2] = -m_baseFreq*slip*dst[1] - (dst[2] - (x0 - xp)*ast[0]) / T0p;


  ad.assign (refDiff + 1, refAlg + 1,-(x0 - xp) / T0p );
  ad.assign (refDiff + 1, refDiff, m_baseFreq * dst[2]);
  ad.assign (refDiff + 1, refDiff + 1, -1.0 / T0p - cj);
  ad.assign (refDiff + 1, refDiff + 2, m_baseFreq * slip);

  ad.assign (refDiff + 2, refAlg, (x0 - xp) / T0p);
  ad.assign (refDiff + 2, refDiff, -m_baseFreq * dst[1]);
  ad.assign (refDiff + 2, refDiff + 1, -m_baseFreq * slip);
  ad.assign (refDiff + 2, refDiff + 2, -1.0 / T0p - cj);

}

void motorLoad3::outputPartialDerivatives (const IOdata &inputs, const stateData &, matrixData<double> &ad, const solverMode &sMode)
{


  auto refAlg = offsets.getAlgOffset (sMode);
  double V = inputs[voltageInLocation];
  double theta = inputs[angleInLocation];

  double vr, vm;
  vr = -V*Vcontrol*sin (theta);
  vm = V * Vcontrol * cos (theta);


  //vr*m_state[0] + vm*m_state[1];

  //output P
  ad.assign (PoutLocation, refAlg, vr * scale);
  ad.assign (PoutLocation, refAlg + 1, vm * scale);

  //vm*m_state[0] - vr*m_state[1];
  //output Q
  ad.assign (QoutLocation, refAlg, vm * scale);
  ad.assign (QoutLocation, refAlg + 1, -vr * scale);


}

count_t motorLoad3::outputDependencyCount(index_t /*num*/, const solverMode & /*sMode*/) const
{
	return 2;
}

void motorLoad3::ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{

  Lp Loc = offsets.getLocations  (sD, sMode, this);

  double V = inputs[voltageInLocation];
  double angle = inputs[angleInLocation];
  double vr, vm;
  vr = -V*Vcontrol*sin (angle);
  vm = V * Vcontrol * cos (angle);

  const double *gm = Loc.algStateLoc;

  double ir = gm[0] * scale;
  double im = gm[1] * scale;

  //P=vr*m_state[0] + vm*m_state[1];

  //Q=vm*m_state[0] - vr*m_state[1];
  ad.assignCheckCol (PoutLocation, inputLocs[angleInLocation], -ir * vm + vr * im);
  ad.assignCheckCol (PoutLocation, inputLocs[voltageInLocation], ir * vr / V + vm * im / V);
  ad.assignCheckCol (QoutLocation, inputLocs[angleInLocation], vr * ir + vm * im);
  ad.assignCheckCol (QoutLocation, inputLocs[voltageInLocation], vm * ir / V - vr * im / V);

}

index_t motorLoad3::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "slip")
    {
      if (isLocal (sMode))
        {
          ret = 2;
        }
      else if (isDynamic (sMode))
        {
          ret = offsets.getDiffOffset (sMode);

        }
      else
        {
          ret = offsets.getAlgOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 2 : ret;
        }
    }
  else if (field == "erp")
    {
      if (isLocal (sMode))
        {
          ret = 3;
        }
      else if (isDynamic (sMode))
        {
          ret = offsets.getDiffOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 1 : ret;
        }
      else
        {
          ret = offsets.getAlgOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 3 : ret;
        }

    }
  else if (field == "emp")
    {
      if (isLocal (sMode))
        {
          ret = 4;
        }
      else if (isDynamic (sMode))
        {
          ret = offsets.getDiffOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 2 : ret;
        }
      else
        {
          ret = offsets.getAlgOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 4 : ret;
        }
    }
  else if (field == "ir")
    {
      ret = offsets.getAlgOffset (sMode);

    }
  else if (field == "im")
    {
      ret = offsets.getAlgOffset (sMode);
      ret = (ret != kNullLocation) ? ret + 1 : ret;
    }
  return ret;
}


void motorLoad3::rootTest (const IOdata & /*inputs*/, const stateData &sD, double roots[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  auto ro = offsets.getRootOffset (sMode);
  if (opFlags[stalled])
    {
      double Te = Loc.diffStateLoc[1] * Loc.algStateLoc[0] + Loc.diffStateLoc[2] * Loc.algStateLoc[1];
      roots[ro] = Te - mechPower (1.0);
     // printf ("[%f]look power =%f\n",sD.time,roots[ro]);
    }
  else
    {
      double slip = Loc.diffStateLoc[0];
      roots[ro] = 1.0 - slip;

    }
}

void motorLoad3::rootTrigger (coreTime /*ttime*/, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode)
{
  if (!rootMask[offsets.getRootOffset (sMode)])
    {
      return;
    }
  if (opFlags[stalled])
    {
      if (inputs[voltageInLocation] > 0.5)
        {
          opFlags.reset (stalled);
          alert (this, JAC_COUNT_INCREASE);
          m_state[2] = 1.0 - 1e-7;
        }
    }
  else
    {
      opFlags.set (stalled);
      alert (this, JAC_COUNT_DECREASE);
		if (inputs[voltageInLocation]<0.25)
		{
			alert(this, POTENTIAL_FAULT_CHANGE);
		}
      m_state[2] = 1.0;
    }
}

change_code motorLoad3::rootCheck (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode, check_level_t /*level*/)
{
  if (opFlags[stalled])
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);
      const double Te = Loc.diffStateLoc[1] * Loc.algStateLoc[0] + Loc.diffStateLoc[2] * Loc.algStateLoc[1];
      if (Te - mechPower (1.0) > 0)
        {
          opFlags.reset (stalled);
          alert (this, JAC_COUNT_INCREASE);
          return change_code::jacobian_change;
        }
    }
  return change_code::no_change;
}



double motorLoad3::getRealPower () const
{
  double v = 1.0,ang = 0.0;
  if (bus)
    {
      v = bus->getVoltage ();
      ang = bus->getAngle ();
    }
  double vr = -v*Vcontrol*sin (ang);
  double vm = v * Vcontrol * cos (ang);
  double Ptemp = vr * m_state[0] + vm * m_state[1];
  return Ptemp * scale;
}

double motorLoad3::getReactivePower () const
{
  double v = 1.0, ang = 0.0;
  if (bus)
    {
      v = bus->getVoltage ();
      ang = bus->getAngle ();
    }
  double vr = -v*Vcontrol*sin (ang);
  double vm = v * Vcontrol * cos (ang);
  double Qtemp = vm * m_state[0] - vr * m_state[1];

  return Qtemp * scale;
}

double motorLoad3::getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  const double V = inputs[voltageInLocation];
  double A = inputs[angleInLocation];

  double Vr = -V *Vcontrol* sin (A);
  double Vm = V * Vcontrol * cos (A);


  auto offset = offsets.getAlgOffset (sMode);
  double im = sD.state[offset + 1];
  double ir = sD.state[offset];
  double Ptemp = Vr * ir + Vm * im;


  return Ptemp * scale;
}

double motorLoad3::getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
  const double V = inputs[voltageInLocation];
  double A = inputs[angleInLocation];

  double Vr = -V *Vcontrol* sin (A);
  double Vm = V * Vcontrol * cos (A);


  auto offset = offsets.getAlgOffset (sMode);
  double im = sD.state[offset + 1];
  double ir = sD.state[offset];
  double Qtemp = Vm * ir - Vr * im;

  return Qtemp * scale;

}

double motorLoad3::getRealPower (double V) const
{
  double  ang = (bus) ? bus->getAngle () : 0.0;

  double vr = -V*Vcontrol*sin (ang);
  double vm = V * Vcontrol * cos (ang);
  double Ptemp = vr * m_state[0] + vm * m_state[1];
  return Ptemp * scale;
}

double motorLoad3::getReactivePower (double V) const
{
  double  ang = (bus) ? bus->getAngle () : 0.0;

  double vr = -V*Vcontrol*sin (ang);
  double vm = V * Vcontrol * cos (ang);
  double Qtemp = vm * m_state[0] - vr * m_state[1];

  return Qtemp * scale;
}
