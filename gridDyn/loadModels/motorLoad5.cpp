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

#include "loadModels/motorLoad.h"
#include "gridBus.h"
#include "objectFactory.h"
#include "vectorOps.hpp"
#include "matrixData.h"
#include "gridCoreTemplates.h"

#include <iostream>

using namespace gridUnits;

//setup the load object factories



motorLoad5::motorLoad5 (const std::string &objName) : motorLoad3 (objName)
{
  H = 4;
}

gridCoreObject *motorLoad5::clone (gridCoreObject *obj) const
{
  motorLoad5 *ld = cloneBase<motorLoad5, motorLoad3> (this, obj);
  if (ld == nullptr)
    {
      return obj;
    }

  ld->T0pp = T0pp;
  ld->xpp = xpp;
  ld->r2 = r2;
  ld->x2 = x2;
  return ld;
}


void motorLoad5::pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags)
{
  //setup the parameters
  x0 = x + xm;
  xp = x + x1 * xm / (x1 + xm);
  T0p = (x1 + xm) / (m_baseFreq * r1);

  T0pp = (x2 + x1 * xm / (x1 + xm)) / (m_baseFreq * r2);
  xpp = x + x1 * x2 * xm / (x1 * x2 + x1 * xm + x2 * xm);

  scale = mBase / systemBasePower;
  m_state.resize (7, 0);
  if (opFlags.test (init_transient))
    {
      m_state[2] = init_slip;
    }
  else if (P > -kHalfBigNum)
    {
      m_state[2] = computeSlip (P);
    }
  else
    {
      m_state[2] = 1.0;
      opFlags.set (init_transient);
    }

  Vpqmin = -1.0;
  Vpqmax = kBigNum;
  gridLoad::pFlowObjectInitializeA (time0, flags);
  converge ();

  loadSizes (cLocalSolverMode, false);
  setOffset (0, cLocalSolverMode);

}

void motorLoad5::converge ()
{
  double V = bus->getVoltage ();
  double theta = bus->getAngle ();
  double slip = m_state[2];
  double Qtest = qPower (V, m_state[2]);
  double im, ir;
  double Vr, Vm;
  double er, em;
  double emp, erp;

  Vr = -V *Vcontrol* sin (theta);
  Vm = V * Vcontrol * cos (theta);
  solve2x2 (Vr, Vm, Vm, -Vr, P, Qtest, ir, im);
  double err = 10;
  double slipp = slip;
  int ccnt = 0;
  double fbs = slip * m_baseFreq;

  double perr = 10;
  double dslip = 0;
  while (err > 1e-6)
    {
      erp = Vr - r * ir + xp * im;
      emp = Vm - r * im - xp * ir;
      solve2x2 (fbs, -1.0 / T0pp, 1.0 / T0pp, fbs, fbs * erp - erp / T0pp + (xp - xpp) / T0pp * ir, fbs * emp - emp / T0pp + (xp - xpp) / T0pp * im, er, em);

      slipp = (er + (x0 - xp) * im) / T0p / m_baseFreq / em;
      dslip = slipp - slip;
      if (P > 0)
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
      m_state[5] = erp;
      m_state[6] = emp;
      if (++ccnt > 50)
        {
          break;
        }

      perr = err;

      fbs = slip * m_baseFreq;
      ir = (-fbs * er * T0p - em) / (-(x0 - xp));
      im = (mechPower (slip) - erp * ir) / emp;

    }

}

void motorLoad5::dynObjectInitializeA (gridDyn_time /*time0*/, unsigned long /*flags*/)
{

}

void motorLoad5::dynObjectInitializeB (const IOdata &args, const IOdata & /*outputSet*/)
{
  if (opFlags.test (init_transient))
    {
      derivative (args, nullptr, m_dstate_dt.data (), cLocalSolverMode);
    }

}



void motorLoad5::loadSizes (const solverMode &sMode, bool /*dynOnly*/)
{
  auto so = offsets.getOffsets (sMode);

  so->reset ();
  if (isDynamic (sMode))
    {
      so->total.algSize = 2;
      so->total.jacSize = 8;
      if ((opFlags.test (stalled)) && (opFlags.test (resettable)))
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
          so->total.diffSize = 5;
          so->total.jacSize += 27;

        }
    }
  else
    {
      so->total.algSize = 7;
      if (opFlags.test (init_transient))
        {
          so->total.jacSize = 31;
        }
      else
        {
          so->total.jacSize = 35;
        }
    }
  so->rjLoaded = true;
  so->stateLoaded = true;
}

// set properties
void motorLoad5::set (const std::string &param,  const std::string &val)
{

  if (param == "#")
    {

    }
  else
    {
      gridLoad::set (param, val);
    }

}

void motorLoad5::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param == "r2")
    {
      r2 = val;
    }
  else if (param == "x2")
    {
      x2 = val;
    }
  else
    {
      motorLoad::set (param, val, unitType);
    }

}


// residual
void motorLoad5::residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{
  if (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations (sD, resid, sMode, this);


      double V = args[voltageInLocation];
      double theta = args[angleInLocation];
      const double *gm = Loc.algStateLoc;
      const double *gmd = Loc.diffStateLoc;
      const double *gmp = Loc.dstateLoc;

      double *rva = Loc.destLoc;
      double *rvd = Loc.destDiffLoc;
      double Vr, Vm;

      Vr = -V *Vcontrol* sin (theta);
      Vm = V * Vcontrol * cos (theta);


      //ir
      rva[irA] = Vm - gmd[emppD] - r * gm[imA] - xpp * gm[irA];
      //im
      rva[imA] = Vr - gmd[erppD] - r * gm[irA] + xpp * gm[imA];

      if (isAlgebraicOnly (sMode))
        {
          return;
        }
      derivative (args, sD, resid, sMode);
      //Get the exciter field

      // delta
      rvd[0] -= gmp[0];
      rvd[1] -= gmp[1];
      rvd[2] -= gmp[2];
      rvd[3] -= gmp[3];
      rvd[4] -= gmp[4];

    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      double V = args[voltageInLocation];
      double theta = args[angleInLocation];

      const double *gm = sD->state + offset;
      double *rv = resid + offset;
      double Vr, Vm;

      Vr = -V *Vcontrol* sin (theta);
      Vm = V * Vcontrol * cos (theta);

      //ir
      rv[irA] = Vm - gm[emppA] - r * gm[imA] - xpp * gm[irA];
      //im
      rv[imA] = Vr - gm[erppA] - r * gm[irA] + xpp * gm[imA];

      double slip = gm[slipA];
      // printf("angle=%f, slip=%f\n", theta, slip);
      // slip
      if (opFlags.test (init_transient))
        {
          rv[slipA] = slip - m_state[slipA];
        }
      else
        {

          double Te = gm[erppA] * gm[irA] + gm[emppA] * gm[imA];
          rv[slipA] = (mechPower (slip) - Te) / (2 * H);
        }
      // Erp and Emp
      rv[erpA] = m_baseFreq * slip * gm[empA] - (gm[erpA] + (x0 - xp) * gm[imA]) / T0p;
      rv[empA] = -m_baseFreq * slip * gm[erpA] - (gm[empA] - (x0 - xp) * gm[irA]) / T0p;
      rv[erppA] = -m_baseFreq * slip * (gm[empA] - gm[emppA]) - (gm[erpA] - gm[emppA] - (xp - xpp) * gm[imA]) / T0pp;
      rv[emppA] = m_baseFreq * slip * (gm[erpA] - gm[erppA])  - (gm[empA] - gm[erppA] + (xp - xpp) * gm[irA]) / T0pp;

    }

}

void motorLoad5::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
  std::string prefix2 = prefix + name;
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
      stNames[offsetD + 3] = prefix2 + ":erpp";
      stNames[offsetD + 4] = prefix2 + ":empp";
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      stNames[offset] = prefix2 + ":ir";
      stNames[offset + 1] = prefix2 + ":im";
      stNames[offset + 2] = prefix2 + ":slip";
      stNames[offset + 3] = prefix2 + ":erp";
      stNames[offset + 4] = prefix2 + ":emp";
      stNames[offset + 5] = prefix2 + ":erpp";
      stNames[offset + 6] = prefix2 + ":empp";
    }

}
void motorLoad5::timestep (gridDyn_time ttime, const IOdata &args, const solverMode &)
{
  stateData sD(ttime, m_state.data());

  derivative (args, &sD, m_dstate_dt.data (), cLocalSolverMode);
  double dt = ttime - prevTime;
  m_state[2] += dt * m_dstate_dt[2];
  m_state[3] += dt * m_dstate_dt[3];
  m_state[4] += dt * m_dstate_dt[4];
  m_state[5] += dt * m_dstate_dt[5];
  m_state[6] += dt * m_dstate_dt[6];
  prevTime = ttime;
  updateCurrents (args, &sD, cLocalSolverMode);
}

void motorLoad5::updateCurrents (const IOdata &args, const stateData *sD, const solverMode &sMode)
{

  Lp Loc = offsets.getLocations (sD, const_cast<double *> (sD->state), sMode, this);
  double V = args[voltageInLocation];
  double theta = args[angleInLocation];

  double vr, vm;
  vr = -V*Vcontrol*sin (theta);
  vm = V * Vcontrol * cos (theta);

  solve2x2 (r, -xpp, xpp, r, vr - Loc.diffStateLoc[3], vm - Loc.diffStateLoc[4], Loc.destLoc[0], Loc.destLoc[1]);

}

void motorLoad5::derivative (const IOdata & /*args*/, const stateData *sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, deriv, sMode, this);
  const double *ast = Loc.algStateLoc;
  const double *dst = Loc.diffStateLoc;
  const double *ddt = Loc.dstateLoc;
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double slip = dst[slipD];

  if (Loc.time >= 1.0)
    {
      mechPower (slip);
    }

  // slip
  if (opFlags.test (stalled))
    {
      dv[slipD] = 0;
    }
  else
    {
      double Te = dst[erppD] * ast[irA] + dst[emppD] * ast[imA];
      dv[slipD] = (mechPower (slip) - Te) / (2 * H);
    }
  // printf("t=%f, slip=%f mp=%f, te=%f, dslip=%e\n", sD->time, slip,mechPower(slip), Te,dv[0] );
  // Edp and Eqp
  dv[erpD] = m_baseFreq * slip * dst[empD] - (dst[erpD] + (x0 - xp) * ast[imA]) / T0p;
  dv[empD] = -m_baseFreq * slip * dst[erpD] - (dst[empD] - (x0 - xp) * ast[irA]) / T0p;
  dv[erppD] = -m_baseFreq * slip * (dst[empD] - dst[emppD]) + ddt[erpD] - (dst[erpD] - dst[emppD] - (xp - xpp) * ast[imA]) / T0pp;
  dv[emppD] = m_baseFreq * slip * (dst[erpD] - dst[erppD]) + ddt[empD]  - (dst[empD] - dst[erppD] + (xp - xpp) * ast[irA]) / T0pp;

}


void motorLoad5::jacobianElements (const IOdata &args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
  index_t refAlg, refDiff;
  const double *gm, *dst;
  double cj = sD->cj;
  if  (isDynamic (sMode))
    {
      Lp Loc = offsets.getLocations (sD, sMode, this);

      refAlg = Loc.algOffset;
      refDiff = Loc.diffOffset;
      gm = Loc.algStateLoc;
      dst = Loc.diffStateLoc;
      //auto mx = &(offsets.getOffsets (sMode)->jacSize);
      //auto js = *mx;
    }
  else
    {
      auto offset = offsets.getAlgOffset (sMode);
      refAlg = offset;
      refDiff = offset + 2;
      gm = sD->state + offset;
      dst = sD->state + offset + 2;
      cj = 0;
    }

  double V = args[voltageInLocation];
  double theta = args[angleInLocation];
  auto VLoc = argLocs[voltageInLocation];
  auto TLoc = argLocs[angleInLocation];
  double Vr, Vm;

  Vr = -V *Vcontrol* sin (theta);
  Vm = V * Vcontrol * cos (theta);



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

  ad.assign (refAlg, refAlg, -xpp);
  ad.assign (refAlg, refAlg + 1, -r);

  ad.assign (refAlg + 1, refAlg, -r);
  ad.assign (refAlg + 1, refAlg + 1, xpp);
  if ((isDynamic (sMode)) && (isAlgebraicOnly (sMode)))
    {
      return;
    }
  // Ir Differential

  ad.assign (refAlg, refDiff + 4, -1);
  // Im Differential
  ad.assign (refAlg + 1, refDiff + 3, -1);


  double slip = dst[0];
  if ((isDynamic (sMode)) || (!opFlags.test (init_transient)))
    {
      /*
      // slip
      double Te = dst[1] * ast[0] + dst[2] * ast[1];
      dv[0] = (mechPower(slip) - Te) / (2 * H);

      */
      // slip
      if (opFlags.test (stalled))
        {
          ad.assign (refDiff, refDiff, -cj);
        }
      else
        {
          ad.assign (refDiff, refDiff, dmechds (slip) / (2 * H) - cj);
          ad.assign (refDiff, refDiff + 3, -gm[0] / (2 * H));
          ad.assign (refDiff, refDiff + 4, -gm[1] / (2 * H));
          ad.assign (refDiff, refAlg, -dst[3] / (2 * H));
          ad.assign (refDiff, refAlg + 1, -dst[4] / (2 * H));
        }
    }
  else
    {
      ad.assign (refDiff, refDiff, 1);
    }
  // omega

  // Erp and Emp
  //dv[1] = m_baseFreq*slip*dst[2] - (dst[1] + (x0 - xp)*ast[1]) / T0p;
  //dv[2] = -m_baseFreq*slip*dst[1] - (dst[2] + (x0 - xp)*ast[0]) / T0p;


  ad.assign (refDiff + 1, refAlg + 1, -(x0 - xp) / T0p);
  ad.assign (refDiff + 1, refDiff, m_baseFreq * dst[2]);
  ad.assign (refDiff + 1, refDiff + 1, -1 / T0p - cj);
  ad.assign (refDiff + 1, refDiff + 2, m_baseFreq * slip);

  ad.assign (refDiff + 2, refAlg, (x0 - xp) / T0p);
  ad.assign (refDiff + 2, refDiff, -m_baseFreq * dst[1]);
  ad.assign (refDiff + 2, refDiff + 1, -m_baseFreq * slip);
  ad.assign (refDiff + 2, refDiff + 2, -1 / T0p - cj);

  //Erpp and Empp
  //dv[3] = -m_baseFreq*slip*(dst[2] - dst[4]) + ddt[1] - (dst[1] - dst[4] - (xp - xpp)*ast[1]) / T0pp;
  //dv[4] = m_baseFreq*slip*(dst[1] - dst[3]) + ddt[2] - (dst[2] - dst[3] + (xp - xpp)*ast[0]) / T0pp;
  ad.assign (refDiff + 3, refAlg + 1, (xp - xpp) / T0pp);
  ad.assign (refDiff + 3, refDiff, -m_baseFreq * (dst[2] - dst[4]));
  ad.assign (refDiff + 3, refDiff + 1, -1 / T0pp + cj);
  ad.assign (refDiff + 3, refDiff + 2, -m_baseFreq * slip);
  ad.assign (refDiff + 3, refDiff + 3, -cj);
  ad.assign (refDiff + 3, refDiff + 4, m_baseFreq * slip + 1 / T0pp);

  ad.assign (refDiff + 4, refAlg, -(xp - xpp) / T0pp);
  ad.assign (refDiff + 4, refDiff, m_baseFreq * (dst[1] - dst[3]));
  ad.assign (refDiff + 4, refDiff + 1, m_baseFreq * slip);
  ad.assign (refDiff + 4, refDiff + 2, -1 / T0pp + cj);
  ad.assign (refDiff + 4, refDiff + 3, -m_baseFreq * slip + 1 / T0pp);
  ad.assign (refDiff + 4, refDiff + 4, -cj);

}


index_t motorLoad5::findIndex (const std::string &field, const solverMode &sMode) const
{
  index_t ret = kInvalidLocation;
  if (field == "erpp")
    {
      if (isLocal (sMode))
        {
          ret = 5;
        }
      else if (isDynamic (sMode))
        {
          ret = offsets.getDiffOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 3 : ret;
        }
      else
        {
          ret = offsets.getAlgOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 5 : ret;
        }

    }
  else if (field == "empp")
    {
      if (isLocal (sMode))
        {
          ret = 6;
        }
      else if (isDynamic (sMode))
        {
          ret = offsets.getDiffOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 4 : ret;
        }
      else
        {
          ret = offsets.getAlgOffset (sMode);
          ret = (ret != kNullLocation) ? ret + 6 : ret;
        }
    }
  else
    {
      ret = motorLoad3::findIndex (field,sMode);

    }

  return ret;
}

void motorLoad5::rootTest (const IOdata & /*args*/, const stateData *sD, double roots[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  auto ro = offsets.getRootOffset (sMode);
  if (opFlags.test (stalled))
    {
      double Te = Loc.diffStateLoc[erppD] * Loc.algStateLoc[irA] + Loc.diffStateLoc[emppD] * Loc.algStateLoc[imA];
      roots[ro] = Te - mechPower (1.0);
    }
  else
    {
      double slip = Loc.diffStateLoc[0];
      roots[ro] = 1.0 - slip;
    }
}

void motorLoad5::rootTrigger (gridDyn_time /*ttime*/, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
  if (!rootMask[offsets.getRootOffset (sMode)])
    {
      return;
    }
  if (opFlags.test (stalled))
    {
      if (args[voltageInLocation] > 0.5)
        {
          opFlags.reset (stalled);
          alert (this, JAC_COUNT_INCREASE);
          m_state[slipA] = 1.0 - 1e-7;
        }
    }
  else
    {
      opFlags.set (stalled);
      alert (this, JAC_COUNT_DECREASE);
      m_state[slipA] = 1.0;
    }
}

change_code motorLoad5::rootCheck (const IOdata & /*args*/, const stateData *sD, const solverMode &sMode, check_level_t /*level*/)
{
  if (opFlags[stalled])
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);
      double Te = Loc.diffStateLoc[erppD] * Loc.algStateLoc[irA] + Loc.diffStateLoc[emppD] * Loc.algStateLoc[imA];
      if (Te - mechPower (1.0) > 0)
        {
          opFlags.reset (stalled);
          alert (this, JAC_COUNT_INCREASE);
          return change_code::jacobian_change;
        }
    }
  return change_code::no_change;
}
