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


#include "submodels/otherGenModels.h"
#include "generators/gridDynGenerator.h"
#include "core/gridDynExceptions.h"

#include "gridBus.h"
#include "matrixData.h"
#include "gridCoreTemplates.h"
#include "vectorOps.hpp"

#include <cmath>
#include <complex>


gridDynGenModelInverter::gridDynGenModelInverter (const std::string &objName) : gridDynGenModel (objName)
{

}

gridDynGenModelInverter::~gridDynGenModelInverter ()
{
}

gridCoreObject *gridDynGenModelInverter::clone (gridCoreObject *obj) const
{
  gridDynGenModelInverter *gd = cloneBase<gridDynGenModelInverter, gridDynGenModel> (this, obj);
  if (!(gd))
    {
      return obj;
    }

  gd->minAngle = minAngle;
  gd->maxAngle = maxAngle;

  return gd;
}

void gridDynGenModelInverter::objectInitializeA (double /*time0*/, unsigned long /*flags*/)
{

  offsets.local->local.algSize = 1;
  offsets.local->local.jacSize = 4;
  offsets.local->local.algRoots = 1;

}
// initial conditions
void gridDynGenModelInverter::objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet)
{
  double *gm = m_state.data ();
  double V = args[voltageInLocation];
  std::complex<double> outCurrent (outputSet[PoutLocation] / V, -outputSet[QoutLocation] / V);
  auto Z = std::complex<double> (Rs, Xd);
  auto Em = Z * outCurrent + V;


  gm[0] = std::arg (Em);

  inputSet[genModelEftInLocation] = std::abs (Em) - 1.0;

  double loss = 0;
  if (Rs != 0)
    {
      double cosA = cos (gm[0]);
      double Vloss1 = V * V * g;
      double Vloss2 = 2.0 * V * g * std::abs (Em) * cosA;
      double Vloss3 = std::abs (Em) * std::abs (Em) * g;
      loss = Vloss1 + Vloss2 + Vloss3;
    }

  inputSet[genModelPmechInLocation] = outputSet[PoutLocation] + loss;             //Pmt

  bus = static_cast<gridBus *> (parent->find ("bus"));
}

void gridDynGenModelInverter::algebraicUpdate (const IOdata &args, const stateData *, double update[], const solverMode &sMode, double /*alpha*/)
{

  auto offset = offsets.getAlgOffset (sMode);


  //double angle = std::atan2(g, b);

  double Pmt = args[genModelPmechInLocation];
  if (opFlags[at_angle_limits])
    {
      if (Pmt > 0)
        {
          update[offset] = maxAngle;
        }
      else
        {
          update[offset] = minAngle;
        }
    }
  else
    {
      //Get the exciter field
      double V = args[voltageInLocation];
      double Eft = args[genModelEftInLocation] + 1.0;
      if (Rs != 0.0)
        {
          double R = std::hypot (2.0 * g, b);
          double gamma = std::atan ( b / (2.0 * g)) - kPI / 2;
          double Vloss1 = V * V * g;
          double Vloss3 = Eft * Eft * g;
		  double powerRatio = (Pmt - Vloss1 - Vloss3) / (Eft * V * R);
		  if (std::abs(powerRatio) >= 1.0)
		  {
			  update[offset] = (powerRatio >= 1.0) ? kPI / 2.0 : -kPI / 2.0;
		  }
		  else
		  {
			  update[offset] = std::asin(powerRatio)+gamma;
		  }
        }
      else
        {
		  double powerRatio = Pmt / (Eft * V * b);
		  if (std::abs(powerRatio) >=1.0)
			{
			  update[offset] = (powerRatio >= 1.0) ? kPI / 2.0 : -kPI / 2.0;
			}
		  else
		  {
			  update[offset] = std::asin(powerRatio);
		  }
          
        }
    }


}

// residual


void gridDynGenModelInverter::residual(const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode)
{

	if (!hasAlgebraic(sMode))
	{
		return;
	}
	Lp Loc = offsets.getLocations(sD, resid, sMode, this);

	double angle = *Loc.algStateLoc;
	//printf("time=%f, angle=%f\n", sD->time, angle);
	double Pmt = args[genModelPmechInLocation];
	if (opFlags[at_angle_limits])
	{
		if (Pmt > 0)
		{
			Loc.destLoc[0] = maxAngle - angle;
		}
		else
		{
			Loc.destLoc[0] = minAngle - angle;
		}
	}
	else
	{
		//Get the exciter field
		double Eft = args[genModelEftInLocation] + 1.0;

		double V = args[voltageInLocation];

		double PnoR = Eft*V*b*sin(angle);
		if (Rs != 0.0)
		{
			double cosA = cos(angle);
			double Vloss1 = V*V*g;
			double Vloss2 = 2.0*V*g*Eft*cosA;
			double Vloss3 = Eft*Eft*g;
			double loss = Vloss1 + Vloss2 + Vloss3;
			Loc.destLoc[0] = Pmt - PnoR - loss;
		}
		else
		{
			Loc.destLoc[0] = Pmt - PnoR;
		}
	}


}


double gridDynGenModelInverter::getFreq(const stateData *sD, const solverMode &sMode, index_t *FreqOffset) const
{
	//there is no inertia in this gen model so it can't compute a frequency and must use the bus frequency
	if (FreqOffset)
	{
		*FreqOffset = bus->getOutputLoc(sMode, frequencyInLocation);
		
	}
	return bus->getFreq(sD, sMode);

}

double gridDynGenModelInverter::getAngle(const stateData *sD, const solverMode &sMode, index_t *AngleOffset) const
{
	//there is no inertia in this gen model so it can't compute a frequency and must use the bus frequency
	auto offset = offsets.getAlgOffset(sMode);
	if (AngleOffset)
	{
		*AngleOffset = offset;
	}

	return (sD) ? sD->state[offset] : m_state[0];

}

IOdata gridDynGenModelInverter::getOutputs(const IOdata &args, const stateData *sD, const solverMode &sMode)
{
	Lp Loc = offsets.getLocations(sD, sMode, this);

	IOdata out(2);
	double V = args[voltageInLocation];
	double Eft = args[genModelEftInLocation] + 1.0;
	double cAng = cos(Loc.algStateLoc[0]);
	double sAng = sin(Loc.algStateLoc[0]);

	out[PoutLocation] = realPowerCompute(V, Eft, cAng, sAng);
	out[QoutLocation] = reactivePowerCompute(V, Eft, cAng, sAng);

	return out;

}

double gridDynGenModelInverter::realPowerCompute(double V, double Ef, double cosA, double sinA) const
{
	return V*V*g - V*g*Ef*cosA - V*Ef*b*sinA;
}

double gridDynGenModelInverter::reactivePowerCompute(double V, double Ef, double cosA, double sinA) const
{
	return V*V*b - V*Ef*b*cosA + V*Ef*g*sinA;
}

double gridDynGenModelInverter::getOutput(const IOdata &args, const stateData *sD, const solverMode &sMode, index_t numOut) const
{
	Lp Loc = offsets.getLocations(sD, sMode, this);
	double cAng = cos(Loc.algStateLoc[0]);
	double sAng = sin(Loc.algStateLoc[0]);
	double V = args[voltageInLocation];
	double Eft = args[genModelEftInLocation] + 1.0;

	if (numOut == PoutLocation)
	{
		return  realPowerCompute(V, Eft, cAng, sAng);
	}
	else if (numOut == QoutLocation)
	{
		return  reactivePowerCompute(V, Eft, cAng, sAng);

	}
	return kNullVal;

}


void gridDynGenModelInverter::ioPartialDerivatives(const IOdata &args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode)
{
	Lp Loc = offsets.getLocations(sD, sMode, this);

	double V = args[voltageInLocation];

	double cAng = cos(Loc.algStateLoc[0]);
	double sAng = sin(Loc.algStateLoc[0]);

	//out[PoutLocation] = V*V*g - V*g*Eft*cAng - V*Eft*b*sAng;
	//out[QoutLocation] = V*V*b - V*Eft*b*cAng + V*Eft*g*sAng;


	if (argLocs[genModelEftInLocation] != kNullLocation)
	{
		ad.assign(PoutLocation, argLocs[genModelEftInLocation], -V*g*cAng - V*b*sAng);
		ad.assign(QoutLocation, argLocs[genModelEftInLocation], -V*b*cAng + V*g*sAng);
	}

	if (argLocs[voltageInLocation] != kNullLocation)
	{
		double Eft = args[genModelEftInLocation] + 1.0;
		ad.assign(PoutLocation, argLocs[voltageInLocation], 2.0*V*g - g*Eft*cAng - Eft*b*sAng);
		ad.assign(QoutLocation, argLocs[voltageInLocation], 2.0*V*b - Eft*b*cAng + V*Eft*g*sAng);
	}

}


void gridDynGenModelInverter::jacobianElements(const IOdata &args, const stateData *sD,
	matrixData<double> &ad,
	const IOlocs &argLocs, const solverMode &sMode)
{
	if (!hasAlgebraic(sMode))
	{
		return;
	}
	Lp Loc = offsets.getLocations(sD, sMode, this);
	auto offset = Loc.algOffset;
	if (opFlags[at_angle_limits])
	{
		ad.assign(offset, offset, -1.0);
	}
	else
	{
		double V = args[voltageInLocation];
		double Eft = args[genModelEftInLocation] + 1.0;
		double cAng = cos(Loc.algStateLoc[0]);
		double sAng = sin(Loc.algStateLoc[0]);

		//rva[0] = Pmt -V*V*g - Eft*Eft*g - 2.0*V * Eft*g*cos(gm[0]) - V * Eft*b*sin(gm[0]);


		ad.assign(offset, offset, 2.0*V*Eft*g*sAng - V*Eft*b*cAng);

		ad.assignCheckCol(offset, argLocs[genModelPmechInLocation], 1.0);
		ad.assignCheckCol(offset, argLocs[genModelEftInLocation], -2.0*Eft*g - 2.0*V*g*cAng - V*b*sAng);
		ad.assignCheckCol(offset, argLocs[voltageInLocation], -2.0*V*g - 2.0*Eft*g*cAng - Eft*b*sAng);
	}



}

void gridDynGenModelInverter::outputPartialDerivatives(const IOdata &args, const stateData *sD, matrixData<double> &ad, const solverMode &sMode)
{
	if (!hasAlgebraic(sMode))
	{
		return;
	}
	Lp Loc = offsets.getLocations(sD, sMode, this);

	double V = args[voltageInLocation];
	double Eft = args[genModelEftInLocation] + 1.0;
	double cAng = cos(Loc.algStateLoc[0]);
	double sAng = sin(Loc.algStateLoc[0]);

	//out[PoutLocation] = V*V*g - V*g*Eft*cAng - V*Eft*b*sAng;
	//out[QoutLocation] = V*V*b - V*Eft*b*cAng + V*Eft*g*sAng;


	ad.assign(PoutLocation, Loc.algOffset, V*g*Eft*sAng - V*Eft*b*cAng);
	ad.assign(QoutLocation, Loc.algOffset, V*Eft*b*sAng + V*Eft*g*cAng);


}

static const stringVec genModelNames{ "angle" };

stringVec gridDynGenModelInverter::localStateNames() const
{
	return genModelNames;
}

// set parameters
void gridDynGenModelInverter::set(const std::string &param, const std::string &val)
{
	return gridSubModel::set(param, val);
}

void gridDynGenModelInverter::set(const std::string &param, double val, gridUnits::units_t unitType)
{

	if (param.length() == 1)
	{
		switch (param[0])
		{
		case 'x':
			Xd = val;
			reCalcImpedences();
			break;
		case 'r':
			Rs = val;
			reCalcImpedences();
			break;
		default:
			throw(unrecognizedParameter());

		}

		return;
	}

	if ((param == "xd") || (param == "xs"))
	{
		Xd = val;
		reCalcImpedences();
	}
	else if (param == "maxangle")
	{
		maxAngle = gridUnits::unitConversionAngle(val, unitType, gridUnits::rad);
	}
	else if (param == "minangle")
	{
		minAngle = gridUnits::unitConversionAngle(val, unitType, gridUnits::rad);
	}
	else if (param == "rs")
	{
		Rs = val;
		reCalcImpedences();
	}
	else
	{
		gridDynGenModel::set(param, val, unitType);
	}

}


void gridDynGenModelInverter::reCalcImpedences()
{
	double Ys = 1.0 / (Rs*Rs + Xd*Xd);
	b = Xd*Ys;
	g = Rs*Ys;
}


void gridDynGenModelInverter::rootTest(const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode)
{

	if (rootSize(sMode) > 0)
	{
		auto ro = offsets.getRootOffset(sMode);
		auto so = offsets.getAlgOffset(sMode);
		double angle = sD->state[so];
		if (opFlags[at_angle_limits])
		{
			if (args[genModelPmechInLocation] > 0)
			{
				double pmax = -realPowerCompute(args[genModelEftInLocation], args[voltageInLocation], cos(maxAngle), sin(maxAngle));
				roots[ro] = args[genModelPmechInLocation] + 0.0001 - pmax;
			}
			else
			{
				double pmin = -realPowerCompute(args[genModelEftInLocation], args[voltageInLocation], cos(minAngle), sin(minAngle));
				roots[ro] = pmin - args[genModelPmechInLocation] + 0.0001;
			}

		}
		else
		{
			roots[ro] = std::min(angle - minAngle, maxAngle - angle);
		}

	}
}

void gridDynGenModelInverter::rootTrigger(double /*ttime*/, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode)
{
	if (rootSize(sMode) > 0)
	{

		auto ro = offsets.getRootOffset(sMode);
		if (rootMask[ro] > 0)
		{
			if (opFlags[at_angle_limits])
			{
				opFlags.reset(at_angle_limits);
				LOG_DEBUG("reset angle limit");
				algebraicUpdate(args, nullptr, m_state.data(), sMode, 1.0);

			}
			else
			{
				opFlags.set(at_angle_limits);
				LOG_DEBUG("angle at limits");
				if (args[genModelPmechInLocation] > 0)
				{
					m_state[0] = maxAngle;
				}
				else
				{
					m_state[0] = minAngle;
				}

			}
		}
	}
}

change_code gridDynGenModelInverter::rootCheck(const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t /*level*/)
{
	if (rootSize(sMode) > 0)
	{
		Lp Loc = offsets.getLocations(sD, sMode, this);
		double angle = Loc.algStateLoc[0];
		if (opFlags[at_angle_limits])
		{
			if (args[genModelPmechInLocation] > 0)
			{
				double pmax = -realPowerCompute(args[genModelEftInLocation], args[voltageInLocation], cos(maxAngle), sin(maxAngle));
				if (args[genModelPmechInLocation] - pmax < -0.0001)
				{
					opFlags.reset(at_angle_limits);
					LOG_DEBUG("reset angle limit-from root check");
					algebraicUpdate(args, nullptr, m_state.data(), sMode, 1.0);
					return change_code::jacobian_change;
				}
			}
			else
			{
				double pmin = -realPowerCompute(args[genModelEftInLocation], args[voltageInLocation], cos(minAngle), sin(minAngle));
				if (pmin - args[genModelPmechInLocation] < -0.0001)
				{
					opFlags.reset(at_angle_limits);
					LOG_DEBUG("reset angle limit- from root check");
					algebraicUpdate(args, nullptr, m_state.data(), sMode, 1.0);
					return change_code::jacobian_change;
				}
			}
		}
		else
		{
			auto remAngle = std::min(angle - minAngle, maxAngle - angle);
			if (remAngle < 0.0000001)
			{
				opFlags.set(at_angle_limits);
				LOG_DEBUG("angle at limit from check");
				if (args[genModelPmechInLocation] > 0)
				{
					m_state[0] = maxAngle;
				}
				else
				{
					m_state[0] = minAngle;
				}
				return change_code::jacobian_change;
			}
		}

	}
	return change_code::no_change;
}