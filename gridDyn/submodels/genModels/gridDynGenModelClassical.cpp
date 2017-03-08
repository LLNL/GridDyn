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

#include "submodels/gridDynGenModel.h"

#include "generators/gridDynGenerator.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.h"
#include "core/objectFactory.h"
#include "gridBus.h"
#include  "utilities/matrixData.h"

#include "utilities/vectorOps.hpp"

#include <cmath>
#include <complex>





gridDynGenModelClassical::gridDynGenModelClassical (const std::string &objName) : gridDynGenModel (objName)
{
  // default values
  opFlags.set (internal_frequency_calculation);
  Xd = 0.85;
}


coreObject *gridDynGenModelClassical::clone (coreObject *obj) const
{
  gridDynGenModelClassical *gd = cloneBase<gridDynGenModelClassical, gridDynGenModel> (this, obj);
  if (!(gd))
    {
      return obj;
    }
  gd->H = H;
  gd->D = D;
  gd->mp_Kw = mp_Kw;              //!<speed gain for a simple pss
  return gd;
}

void gridDynGenModelClassical::dynObjectInitializeA (coreTime /*time0*/, unsigned long /*flags*/)
{
  offsets.local().local.diffSize = 2;
  offsets.local().local.algSize = 2;
  offsets.local().local.jacSize = 18;

}

// initial conditions
void gridDynGenModelClassical::dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet)
{
  computeInitialAngleAndCurrent (inputs, desiredOutput, Rs, Xd);
  double *gm = m_state.data ();
  double Eft = Vq + Rs * gm[1] - Xd * gm[0];
  // record Pm = Pset
  //this should be close to P from above
  //preset the inputs that should be initialized
  inputSet[2] = Eft;
  inputSet[3] = Eft * gm[1];
}

void gridDynGenModelClassical::computeInitialAngleAndCurrent (const IOdata &inputs, const IOdata &desiredOutput, double R1, double X1)
{
  double *gm = m_state.data ();
  double V = inputs[voltageInLocation];
  double theta = inputs[angleInLocation];
  std::complex<double> SS (desiredOutput[0], -desiredOutput[1]);
  std::complex<double> VV = std::polar (V, theta);
  std::complex<double> II = SS / conj (VV);
  gm[2] = std::arg (VV + std::complex<double> (R1, X1) * II);

  gm[3] = 1.0;
  double angle = gm[2] - theta;

  // Id and Iq
  Vq = V * cos (angle);
  Vd = -V*sin (angle);

  std::complex<double> Idq = II * std::polar (1.0, -(gm[2] - kPI / 2));

  gm[0] = -Idq.real ();
  gm[1] = Idq.imag ();
}

void gridDynGenModelClassical::updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode)
{
  if (sD.updateRequired(seqId))
    {
      Lp Loc = offsets.getLocations (sD, sMode, this);
      double V = inputs[voltageInLocation];
      double angle = Loc.diffStateLoc[0] - inputs[angleInLocation];
      Vq = V * cos (angle);
      Vd = -V*sin (angle);
	  seqId = sD.seqID;
    }
}

void gridDynGenModelClassical::algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double /*alpha*/)
{

  auto offset = offsets.getAlgOffset (sMode);
  updateLocalCache (inputs, sD, sMode);
  solve2x2 (Rs, (Xd), -(Xd), Rs, -Vd, inputs[genModelEftInLocation] - Vq, update[offset], update[offset + 1]);
  m_output = -(update[offset + 1] * Vq + update[offset] * Vd);

}

// residual


void gridDynGenModelClassical::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{

  Lp Loc = offsets.getLocations (sD, resid, sMode, this);


  updateLocalCache (inputs, sD, sMode);

  const double *gm = Loc.algStateLoc;
  const double *gmd = Loc.diffStateLoc;
  const double *gmp = Loc.dstateLoc;

  double *rva = Loc.destLoc;
  double *rvd = Loc.destDiffLoc;


  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];

  /*
  rva[0] = Vd + Rs * gm[0] + (Xqp)* gm[1] - gmd[2];
  rva[1] = Vq + Rs * gm[1] - (Xdp)* gm[0] - gmd[3];

  if (isAlgebraicOnly(sMode))
  {
          return;
  }
  // delta
  rvd[0] = m_baseFreq * (gmd[1] - 1.0) - gmp[0];
  // Edp and Eqp
  rvd[2] = (-gmd[2] - (Xq - Xqp) * gm[1]) / Tqop - gmp[2];
  rvd[3] = (-gmd[3] + (Xd - Xdp) * gm[0] + Eft) / Tdop - gmp[3];

  // omega
  double Pe = gmd[2] * gm[0] + gmd[3] * gm[1] + (Xdp - Xqp) * gm[0] * gm[1];
  rvd[1] = 0.5  * (Pmt - Pe - D * (gmd[1] - 1.0)) / H - gmp[1];
  */
  // Id and Iq



  if (hasAlgebraic (sMode))
    {
      rva[0] = Vd + Rs * gm[0] + Xd * gm[1];
      rva[1] = Vq + Rs * gm[1] - Xd * gm[0] - Eft - mp_Kw * (gmd[1] - 1.0);
    }


  if (hasDifferential (sMode))
    {
      // delta
      rvd[0] = m_baseFreq * (gmd[1] - 1.0) - gmp[0];

      // omega
      double Pe = (Eft + mp_Kw * (gmd[1] - 1.0)) * gm[1];
      rvd[1] = 0.5 * (Pmt - Pe - D * (gmd[1] - 1.0)) / H - gmp[1];
    }

}

void gridDynGenModelClassical::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, deriv, sMode, this);
  double *dv = Loc.destDiffLoc;
  //Get the exciter field
  double Eft = inputs[genModelEftInLocation];
  double Pmt = inputs[genModelPmechInLocation];

  double omega = Loc.diffStateLoc[1] - 1.0;
  // Id and Iq

  // delta
  dv[0] = m_baseFreq * omega;
  // Edp and Eqp

  // omega
  double Pe = (Eft + mp_Kw * omega) * Loc.algStateLoc[1];
  dv[1] = 0.5  * (Pmt - Pe - D * omega) / H;

}

double gridDynGenModelClassical::getFreq (const stateData &sD, const solverMode &sMode, index_t *FreqOffset) const
{
  double omega = 1.0;

  if (isLocal (sMode))
    {
      omega = m_state[3];
      if (FreqOffset)
        {
          *FreqOffset = kNullLocation;
        }
      return omega;
    }

  if (!sD.empty())
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);

      omega = Loc.diffStateLoc[1];

      if (FreqOffset)
        {
          *FreqOffset = Loc.diffOffset + 1;
          if (isAlgebraicOnly (sMode))
            {
              *FreqOffset = kNullLocation;
            }
        }
    }
  else if (FreqOffset)
    {
      *FreqOffset = offsets.getDiffOffset (sMode) + 1;
    }
  return omega;

}

double gridDynGenModelClassical::getAngle (const stateData &sD, const solverMode &sMode, index_t *angleOffset) const
{
  double angle = kNullVal;

  if (isLocal (sMode))
    {
      angle = m_state[2];
      if (angleOffset)
        {
          *angleOffset = kNullLocation;
        }
      return angle;
    }

  if (!sD.empty())
    {
      Lp Loc = offsets.getLocations  (sD, sMode, this);

      angle = Loc.diffStateLoc[0];

      if (angleOffset)
        {
          *angleOffset = Loc.diffOffset;
          if (isAlgebraicOnly (sMode))
            {
              *angleOffset = kNullLocation;
            }
        }
    }
  else if (angleOffset)
    {
      *angleOffset = offsets.getDiffOffset (sMode);
    }
  return angle;

}

IOdata gridDynGenModelClassical::getOutputs (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  IOdata out (2);
  out[PoutLocation] = -(Loc.algStateLoc[1] * Vq + Loc.algStateLoc[0] * Vd);
  out[QoutLocation] = -(Loc.algStateLoc[1] * Vd - Loc.algStateLoc[0] * Vq);
  return out;

}

double gridDynGenModelClassical::getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t numOut) const
{
  Lp Loc = offsets.getLocations (sD, sMode, this);
  double Vqtemp = Vq;
  double Vdtemp = Vd;
  if ((sD.empty()) || (sD.seqID != seqId) || (sD.seqID == 0))
    {
      double V = inputs[voltageInLocation];
      double angle = Loc.diffStateLoc[0] - inputs[angleInLocation];
      Vqtemp = V * cos (angle);
      Vdtemp = -V*sin (angle);
    }

  if (numOut == PoutLocation)
    {
      return -(Loc.algStateLoc[1] * Vqtemp + Loc.algStateLoc[0] * Vdtemp);
    }
  else if (numOut == QoutLocation)
    {
      return -(Loc.algStateLoc[1] * Vdtemp - Loc.algStateLoc[0] * Vqtemp);
    }
  return kNullVal;

}


void gridDynGenModelClassical::ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);

  double V = inputs[voltageInLocation];
  updateLocalCache (inputs, sD, sMode);

  const double *gm = Loc.algStateLoc;

  if (inputLocs[angleInLocation] != kNullLocation)
    {
      ad.assign (PoutLocation, inputLocs[angleInLocation], gm[1] * Vd - gm[0] * Vq);
      ad.assign (QoutLocation, inputLocs[angleInLocation], -gm[1] * Vq - gm[0] * Vd);
    }
  if (inputLocs[voltageInLocation] != kNullLocation)
    {
      ad.assign (PoutLocation, inputLocs[voltageInLocation], -gm[1] * Vq / V - gm[0] * Vd / V);
      ad.assign (QoutLocation, inputLocs[voltageInLocation], -gm[1] * Vd / V + gm[0] * Vq / V);
    }

}


void gridDynGenModelClassical::jacobianElements (const IOdata &inputs, const stateData &sD,
                                                 matrixData<double> &ad,
                                                 const IOlocs &inputLocs, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations (sD, sMode, this);

  updateLocalCache (inputs, sD, sMode);

  const double *gm = Loc.algStateLoc;
  auto VLoc = inputLocs[voltageInLocation];
  auto TLoc = inputLocs[angleInLocation];
  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  //rva[0] = Vd + Rs * gm[0] + Xd * gm[1];
  //rva[1] = Vq + Rs * gm[1] - Xd* gm[0] - Eft-mp_Kp*(gmd[1]-1.0);
  if (hasAlgebraic (sMode))
    {
      if (TLoc != kNullLocation)
        {
          ad.assign (refAlg, TLoc, Vq);
          ad.assign (refAlg + 1, TLoc, -Vd);
        }

      // Q
      if (VLoc != kNullLocation)
        {

          ad.assign (refAlg, VLoc, Vd / inputs[voltageInLocation]);
          ad.assign (refAlg + 1, VLoc, Vq / inputs[voltageInLocation]);
        }

      ad.assign (refAlg, refAlg, Rs);
      ad.assign (refAlg, refAlg + 1, (Xd));



      ad.assign (refAlg + 1, refAlg, -(Xd));
      ad.assign (refAlg + 1, refAlg + 1, Rs);
      ad.assignCheckCol (refAlg + 1, inputLocs[genModelEftInLocation], -1.0);

      if (isAlgebraicOnly (sMode))
        {
          return;
        }
      ad.assign (refAlg, refDiff, -Vq);

      ad.assign (refAlg + 1, refDiff + 1, -mp_Kw);

      // Iq Differential
      ad.assign (refAlg + 1, refDiff, Vd);
    }
  // Id and Iq
  /*
  rv[0] = Vd + Rs*gm[0] + (Xdp - Xl)*gm[1];
  rv[1] = Vq + Rs*gm[1] - (Xdp - Xl)*gm[0];
  */
  // Id Differential

  //ad.assignCheckCol (refAlg + 1, inputLocs[genModelEftInLocation], -1.0);
  // delta
  ad.assign (refDiff, refDiff, -sD.cj);
  ad.assign (refDiff, refDiff + 1, m_baseFreq);
  // omega

  double Eft = inputs[genModelEftInLocation];
  //double Pe = (Eft+mp_Kp*(gmd[1] - 1.0))*gm[1];

  double kVal = -0.5 / H;


  //ad.assign (refDiff + 1, refAlg, -0.5  * (Xd - Xdp) * gm[1] / H);
  if (hasAlgebraic (sMode))
    {
      ad.assign (refDiff + 1, refAlg + 1, kVal * (Eft + mp_Kw * (Loc.diffStateLoc[1] - 1.0)));
    }
  ad.assign (refDiff + 1, refDiff + 1, kVal * (D + mp_Kw * gm[1]) - sD.cj);

  ad.assignCheckCol (refDiff + 1, inputLocs[genModelPmechInLocation], -kVal);               // governor: Pm
  ad.assignCheckCol (refDiff + 1, inputLocs[genModelEftInLocation], kVal * gm[1]);               // exciter: Ef

}

void gridDynGenModelClassical::outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const solverMode &sMode)
{
  Lp Loc = offsets.getLocations  (sD, sMode, this);
  auto refAlg = Loc.algOffset;
  auto refDiff = Loc.diffOffset;


  const double *gm = Loc.algStateLoc;

  updateLocalCache (inputs, sD, sMode);
  if (hasAlgebraic (sMode))
    {
      //output P
      ad.assign (PoutLocation, refAlg, -Vd);
      ad.assign (PoutLocation, refAlg + 1, -Vq);


      //output Q
      ad.assign (QoutLocation, refAlg, Vq);
      ad.assign (QoutLocation, refAlg + 1, -Vd);
    }

  if (hasDifferential (sMode))
    {
      ad.assign (PoutLocation, refDiff, -gm[1] * Vd + gm[0] * Vq);
      ad.assign (QoutLocation, refDiff, gm[1] * Vq + gm[0] * Vd);
    }

}

count_t gridDynGenModelClassical::outputDependencyCount(index_t /*num*/, const solverMode & /*sMode*/) const
{
	return 3;
}

static const stringVec genModelClassicStateNames {
  "id","iq","delta","freq"
};

stringVec gridDynGenModelClassical::localStateNames () const
{
  return genModelClassicStateNames;
}


// set parameters
void gridDynGenModelClassical::set (const std::string &param, const std::string &val)
{
  coreObject::set (param, val);
}

void gridDynGenModelClassical::set (const std::string &param, double val, gridUnits::units_t unitType)
{

  if (param.length () == 1)
    {
      switch (param[0])
        {
        case 'x':
          Xd = val;
          break;
        case 'h':
          H = val;
          break;
        case 'r':
          Rs = val;
          break;
        case 'm':
          H = val / 2.0;
          break;
        case 'd':
          D = gridUnits::unitConversionFreq (val, unitType, gridUnits::puHz, m_baseFreq);
          break;

        default:
			throw(unrecognizedParameter());

        }
	  return;
    }

 if (param == "kw")
    {
      mp_Kw = val;
    }
  else
    {
      gridDynGenModel::set (param, val, unitType);
    }

}
