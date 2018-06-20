/*
* LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "motorLoad5.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"
#include "../gridBus.h"
#include "utilities/matrixData.hpp"

#include <cmath>
#include <iostream>
namespace griddyn
{
namespace loads
{
using namespace gridUnits;

// setup the load object factories
static typeFactory<motorLoad> mlf1 ("load", stringVec{"motor", "motor1"});

static typeFactory<motorLoad3> mlf3 ("load", stringVec{"motor3", "motorIII", "m3"});

static typeFactory<motorLoad5> mlf5 ("load", stringVec{"motor5", "motorIV", "m5"});

static const double cSmallDiff = 1e-7;
motorLoad::motorLoad (const std::string &objName) : Load (objName)
{
    // default values
    opFlags.set (has_dyn_states);
}

coreObject *motorLoad::clone (coreObject *obj) const
{
    auto ld = cloneBase<motorLoad, Load> (this, obj);
    if (ld == nullptr)
    {
        return obj;
    }
    ld->r = r;
    ld->x = x;
    ld->Pmot = Pmot;
    ld->r1 = r1;
    ld->x1 = x1;
    ld->xm = xm;
    ld->H = H;
    ld->a = a;
    ld->b = b;
    ld->c = c;
    ld->Vcontrol = Vcontrol;
    ld->mBase = mBase;
    return ld;
}

void motorLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    m_state.resize (1);
    if (opFlags[init_transient])
    {
        if (init_slip >= 0)
        {
            m_state[0] = init_slip;
        }
        else
        {
            m_state[0] = 1.0;
        }
    }
    else if (init_slip > 0)
    {
        m_state[0] = init_slip;
    }
    else
    {
        m_state[0] = 0.03;
    }
    scale = mBase / systemBasePower;
    return Load::pFlowObjectInitializeA (time0, flags);
}

void motorLoad::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    opFlags.set (has_roots);
    return Load::dynObjectInitializeA (time0, flags);
}

void motorLoad::dynObjectInitializeB (const IOdata & /*inputs*/,
                                      const IOdata & /*desiredOutput*/,
                                      IOdata & /*fieldSet*/)
{
    m_dstate_dt[0] = 0;

    if (opFlags[init_transient])
    {
        Pmot = mechPower (m_state[0]);
    }
    else
    {
    }
}

stateSizes motorLoad::LocalStateSizes(const solverMode &sMode) const
{
	stateSizes SS;
	if (isDynamic(sMode))
	{
		if (!isAlgebraicOnly(sMode))
		{
			SS.diffSize = 1;
		}
	}
	else if (!opFlags[init_transient])
	{
		SS.algSize = 1;
	}
	return SS;
}

count_t motorLoad::LocalJacobianCount(const solverMode &sMode) const
{
	count_t localJacSize = 0;
	if (isDynamic(sMode))
	{
		if (!isAlgebraicOnly(sMode))
		{
			localJacSize = 4;
		}
	}
	else if (!opFlags[init_transient])
	{
		localJacSize = 4;
	}
	return localJacSize;
}

std::pair<count_t, count_t> motorLoad::LocalRootCount(const solverMode &/*sMode*/) const
{
	count_t algRoots = 0;
	count_t diffRoots = 0;
	if ((opFlags[stalled]) && (opFlags[resettable]))
	{
		algRoots = 1;
	}
	else
	{
		diffRoots = 1;
	}
	return std::make_pair(algRoots, diffRoots);
}

// set properties
void motorLoad::set (const std::string &param, const std::string &val)
{
    if (param.empty())
    {
    }
    else
    {
        Load::set (param, val);
    }
}

void motorLoad::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    bool slipCheck = false;

    if (param.size () == 1)
    {
        switch (param[0])
        {
        case 'h':
            H = val;
            break;
        case 'p':
            Pmot = unitConversion (val, unitType, puMW, systemBasePower, localBaseVoltage);
            if (mBase < 0)
            {
                mBase = Pmot * systemBasePower;
                scale = Pmot;
            }
            alpha = Pmot / scale;
            a = alpha - b - c;
            slipCheck = true;
            break;
        case 'b':
            b = val;
            alpha = a + b + c;
            beta = -b - 2 * c;
            slipCheck = true;
            break;
        case 'a':
            a = val;
            alpha = a + b + c;
            slipCheck = true;
            break;
        case 'c':
            c = val;
            gamma = c;
            slipCheck = true;
            break;
        default:
            throw (unrecognizedParameter (param));
        }
    }
    else
    {
        if (param == "r1")
        {
            r1 = val;
        }
        else if (param == "x1")
        {
            x1 = val;
        }
        else if (param == "xm")
        {
            xm = val;
        }

        else if (param == "alpha")
        {
            alpha = val;
            a = alpha - b - c;
            slipCheck = true;
        }
        else if (param == "beta")
        {
            beta = val;
            b = -beta - 2 * c;
            a = alpha - b - c;
            slipCheck = true;
        }
        else if (param == "gamma")
        {
            gamma = val;
            c = gamma;
            slipCheck = true;
        }
        else if ((param == "base") || (param == "mbase") || (param == "rating"))
        {
            mBase = unitConversion (val, unitType, MVAR, systemBasePower, localBaseVoltage);
        }
        else if (param == "Vcontrol")
        {
            Vcontrol = unitConversion (val, unitType, puV, systemBasePower, localBaseVoltage);
            slipCheck = true;
        }
        else
        {
            Load::set (param, val, unitType);
        }
    }

    if (slipCheck)
    {
        if (opFlags[stalled])
        {
            rootCheck (bus->getOutputs (noInputs, emptyStateData, cLocalSolverMode), emptyStateData,
                       cLocalSolverMode, check_level_t::reversable_only);
        }
    }
}

void motorLoad::setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode)
{
    if (isDynamic (sMode))
    {
        if (isAlgebraicOnly (sMode))
        {
            return;
        }

        auto offset = offsets.getDiffOffset (sMode);
        m_state[0] = state[offset];
        m_dstate_dt[0] = dstate_dt[offset];
    }
    else if (!opFlags[init_transient])
    {
        auto offset = offsets.getAlgOffset (sMode);
        m_state[0] = state[offset];
    }
    prevTime = time;
}

void motorLoad::guessState (coreTime /*time*/, double state[], double dstate_dt[], const solverMode &sMode)
{
    if (isDynamic (sMode))
    {
        if (hasDifferential (sMode))
        {
            auto offset = offsets.getDiffOffset (sMode);
            state[offset] = m_state[0];
            dstate_dt[offset] = m_dstate_dt[0];
        }
    }
    else if (!opFlags[init_transient])
    {
        auto offset = offsets.getAlgOffset (sMode);
        state[offset] = m_state[0];
    }
}

// residual
void motorLoad::residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode)
{
    if (isDynamic (sMode))
    {
        if (hasDifferential (sMode))
        {
            derivative (inputs, sD, resid, sMode);
            auto offset = offsets.getDiffOffset (sMode);
            resid[offset] -= sD.dstate_dt[offset];
        }
    }
    else if (!opFlags[init_transient])
    {
        auto offset = offsets.getAlgOffset (sMode);
        double slip = sD.state[offset];
        resid[offset] = mechPower (slip) - rPower (inputs[voltageInLocation], slip);
        // printf("slip=%f mpower=%f, rPower=%f\n", slip, mechPower(slip), rPower(inputs[voltageInLocation],
        // slip));
    }
}

void motorLoad::getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const
{
    if (isDynamic (sMode))
    {
        if (isAlgebraicOnly (sMode))
        {
            return;
        }

        auto offset = offsets.getDiffOffset (sMode);
        stNames[offset] = prefix + getName () + ":slip";
    }
    else if (!opFlags[init_transient])
    {
        auto offset = offsets.getAlgOffset (sMode);
        stNames[offset] = prefix + getName () + ":slip";
    }
    else
    {
        return;
    }
}
void motorLoad::timestep (coreTime time, const IOdata &inputs, const solverMode & /*sMode*/)
{
    double dt = time - prevTime;
    motorLoad::derivative (inputs, emptyStateData, m_dstate_dt.data (), cLocalSolverMode);
    m_state[0] += dt * m_dstate_dt[0];
}

void motorLoad::derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode)
{
    auto offset = offsets.getDiffOffset (sMode);
    double slip = (!sD.empty ()) ? sD.state[offset] : m_state[0];
    double V = inputs[voltageInLocation];

    deriv[offset] = (opFlags[stalled]) ? 0 : (0.5 / H * (mechPower (slip) - rPower (V * Vcontrol, slip)));
}

void motorLoad::jacobianElements (const IOdata &inputs,
                                  const stateData &sD,
                                  matrixData<double> &md,
                                  const IOlocs &inputLocs,
                                  const solverMode &sMode)
{
    if (isDynamic (sMode))
    {
        if (hasDifferential (sMode))
        {
            auto offset = offsets.getDiffOffset (sMode);
            double slip = sD.state[offset];
            double V = inputs[voltageInLocation];
            if (opFlags[stalled])
            {
                md.assign (offset, offset, -sD.cj);
            }
            else
            {
                md.assignCheck (offset, inputLocs[voltageInLocation],
                                -(1.0 / H) * (V * Vcontrol * Vcontrol * r1 * slip /
                                              (r1 * r1 + slip * slip * (x + x1) * (x + x1))));
                // this is a really ugly looking derivative so I am computing it numerically
                double test1 = 0.5 / H * (mechPower (slip) - rPower (V, slip));
                double test2 =
                  0.5 / H * (mechPower (slip + cSmallDiff) - rPower (V * Vcontrol, slip + cSmallDiff));
                md.assign (offset, offset, (test2 - test1) / cSmallDiff - sD.cj);
            }
        }
    }
    else if (!opFlags[init_transient])
    {
        int offset = offsets.getAlgOffset (sMode);
        double slip = sD.state[offset];
        double V = inputs[voltageInLocation];

        double t1 = rPower (V * Vcontrol, slip);
        double t3 = rPower (V * Vcontrol, slip + cSmallDiff);
        md.assign (offset, offset, dmechds (slip) - (t3 - t1) / cSmallDiff);

        md.assignCheck (offset, inputLocs[voltageInLocation], -2 * t1 / V);
    }
}

void motorLoad::outputPartialDerivatives (const IOdata &inputs,
                                          const stateData &sD,
                                          matrixData<double> &md,
                                          const solverMode &sMode)
{
    if (isDynamic (sMode))
    {
        if (isAlgebraicOnly (sMode))
        {
            return;
        }

        auto offset = offsets.getDiffOffset (sMode);
        double slip = sD.state[offset];
        double V = inputs[voltageInLocation];

        md.assign (PoutLocation, offset,
                   scale * (rPower (V * Vcontrol, slip + cSmallDiff) - rPower (V * Vcontrol, slip)) / cSmallDiff);

        md.assign (QoutLocation, offset,
                   scale * (qPower (V * Vcontrol, slip + cSmallDiff) - qPower (V * Vcontrol, slip)) / cSmallDiff);
    }
    else if (!opFlags[init_transient])
    {
        auto offset = offsets.getAlgOffset (sMode);
        double slip = sD.state[offset];
        double V = inputs[voltageInLocation];
        md.assign (QoutLocation, offset,
                   scale * (qPower (V * Vcontrol, slip + cSmallDiff) - qPower (V * Vcontrol, slip)) / cSmallDiff);
        md.assign (PoutLocation, offset,
                   scale * (rPower (V * Vcontrol, slip + cSmallDiff) - rPower (V * Vcontrol, slip)) / cSmallDiff);
    }
}

count_t motorLoad::outputDependencyCount (index_t /*num*/, const solverMode & /*sMode*/) const { return 1; }
void motorLoad::ioPartialDerivatives (const IOdata &inputs,
                                      const stateData &sD,
                                      matrixData<double> &md,
                                      const IOlocs &inputLocs,
                                      const solverMode &sMode)
{
    if (inputLocs[voltageInLocation] != kNullLocation)
    {
        double slip = m_state[0];
        double V = inputs[voltageInLocation];
        if (isDynamic (sMode))
        {
            auto Loc = offsets.getLocations (sD, sMode, this);
            slip = Loc.diffStateLoc[0];
        }
        else if (!opFlags[init_transient])
        {
            slip = sD.state[offsets.getAlgOffset (sMode)];
        }
        double temp = V * slip / (r1 * r1 + slip * slip * (x + x1) * (x + x1));
        md.assign (PoutLocation, inputLocs[voltageInLocation], scale * (2 * r1 * temp));
        md.assign (QoutLocation, inputLocs[voltageInLocation], scale * (2 * V / xm + 2 * slip * (x + x1) * temp));
    }
}

index_t motorLoad::findIndex (const std::string &field, const solverMode &sMode) const
{
    index_t ret = kInvalidLocation;
    if (field == "slip")
    {
        ret = offsets.getDiffOffset (sMode);
    }
    return ret;
}

void motorLoad::rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
    auto Loc = offsets.getLocations (sD, sMode, this);
    double slip = Loc.diffStateLoc[0];
    auto ro = offsets.getRootOffset (sMode);
    if (opFlags[stalled])
    {
        roots[ro] = rPower (inputs[voltageInLocation] * Vcontrol, 1.0) - mechPower (1.0);
    }
    else
    {
        roots[ro] = 1.0 - slip;
    }
}

void motorLoad::rootTrigger (coreTime /*time*/,
                             const IOdata &inputs,
                             const std::vector<int> &rootMask,
                             const solverMode &sMode)
{
    if (rootMask[offsets.getRootOffset (sMode)] == 0)
    {
        return;
    }
    if (opFlags[stalled])
    {
        if (inputs[voltageInLocation] > 0.5)
        {
            opFlags.reset (stalled);
            alert (this, JAC_COUNT_INCREASE);
            m_state[0] = 1.0 - 1e-7;
        }
    }
    else
    {
        opFlags.set (stalled);
        alert (this, JAC_COUNT_DECREASE);
        m_state[0] = 1.0;
    }
}

change_code motorLoad::rootCheck (const IOdata &inputs,
                                  const stateData & /*sD*/,
                                  const solverMode & /*sMode*/,
                                  check_level_t /*level*/)
{
    if (opFlags[stalled])
    {
        if (rPower (inputs[voltageInLocation] * Vcontrol, 1.0) - mechPower (1.0) > 0)
        {
            opFlags.reset (stalled);
            alert (this, JAC_COUNT_INCREASE);
            return change_code::jacobian_change;
        }
    }
    return change_code::no_change;
}

double motorLoad::getRealPower () const
{
    const double V = bus->getVoltage ();

    double slip = m_state[0];
    return rPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getReactivePower () const
{
    const double V = bus->getVoltage ();

    double slip = m_state[0];

    return qPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    const double V = inputs[voltageInLocation];

    double Ptemp;
    if (isDynamic (sMode))
    {
        auto Loc = offsets.getLocations (sD, sMode, this);

        double slip = Loc.diffStateLoc[0];
        Ptemp = rPower (V * Vcontrol, slip);
    }
    else if (opFlags[init_transient])
    {
        double slip = m_state[0];
        Ptemp = rPower (V * Vcontrol, slip);
    }
    else
    {
        auto offset = offsets.getAlgOffset (sMode);
        double slip = sD.state[offset];
        Ptemp = rPower (V * Vcontrol, slip);
    }

    return Ptemp * scale;
}

double motorLoad::getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const
{
    double V = inputs[voltageInLocation];
    double Qtemp;
    if (isDynamic (sMode))
    {
        auto Loc = offsets.getLocations (sD, sMode, this);

        double slip = Loc.diffStateLoc[0];
        Qtemp = qPower (V, slip);
    }
    else if (opFlags[init_transient])
    {
        double slip = m_state[0];
        Qtemp = qPower (V * Vcontrol, slip);
    }
    else
    {
        auto offset = offsets.getAlgOffset (sMode);
        double slip = sD.state[offset];
        Qtemp = qPower (V * Vcontrol, slip);
    }
    return Qtemp * scale;
}

double motorLoad::getRealPower (const double V) const
{
    double slip = m_state[0];

    return rPower (V * Vcontrol, slip) * scale;
}

double motorLoad::getReactivePower (double V) const
{
    double slip = m_state[0];
    return qPower (V * Vcontrol, slip) * scale;
}

double motorLoad::mechPower (double slip) const
{
    double Tm = alpha + beta * slip + gamma * slip * slip;
    return Tm;
}

double motorLoad::dmechds (double slip) const
{
    double Tmds = beta + 2 * gamma * slip;
    return Tmds;
}

double motorLoad::computeSlip (double Ptarget) const
{
    if (gamma == 0)
    {
        return (beta == 0) ? 0.05 : (Ptarget - alpha) / beta;
    }

    double out1 = (-beta + std::sqrt (beta * beta - 4.0 * gamma * (alpha - Ptarget))) / (2.0 * gamma);
    double out2 = (-beta - std::sqrt (beta * beta - 4.0 * gamma * (alpha - Ptarget))) / (2.0 * gamma);

    if ((out1 >= 0) && (out1 <= 1.0))
    {
        return out1;
    }
    return out2;
}

double motorLoad::rPower (double vin, double slip) const
{
    double out = r1 * vin * vin * slip / (r1 * r1 + slip * slip * (x + x1) * (x + x1));
    return out;
}
double motorLoad::qPower (double vin, double slip) const
{
    double xs2 = (x + x1) * slip * slip;
    double out = vin * vin * (1.0 / xm + xs2 / (r1 * r1 + xs2 * (x + x1)));
    return out;
}
}  // namespace loads
}  // namespace griddyn