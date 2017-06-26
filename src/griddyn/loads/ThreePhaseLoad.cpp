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

#include "ThreePhaseLoad.h"

#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/matrixData.hpp"

#include <cmath>
#include <complex>
#include <iostream>

namespace griddyn
{
namespace loads
{
using namespace gridUnits;

/** multiplier constants for representation change*/
static constexpr std::complex<double> alpha0 = std::complex<double> (1.0, 0);
static const std::complex<double> alpha = std::polar (1.0, -2.0 * kPI / 3.0);
static const std::complex<double> alpha2 = alpha * alpha;

ThreePhaseLoad::ThreePhaseLoad (const std::string &objName) : Load (objName) {}
ThreePhaseLoad::ThreePhaseLoad (double rP, double rQ, const std::string &objName) : Load (rP, rQ, objName)
{
    Pa = Pb = Pc = rP / 3.0;

    Qa = Pb = Pc = rQ / 3.0;
}

void ThreePhaseLoad::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    Load::pFlowObjectInitializeA (time0, flags);
}

coreObject *ThreePhaseLoad::clone (coreObject *obj) const
{
    auto nobj = cloneBase<ThreePhaseLoad, Load> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }
    nobj->Pa = Pa;
    nobj->Pb = Pb;
    nobj->Pc = Pc;
    nobj->Qa = Qa;
    nobj->Qb = Qb;
    nobj->Qc = Qc;

    return nobj;
}

void ThreePhaseLoad::setLoad (double level, units_t unitType)
{
    setP (unitConversion (level, unitType, puMW, systemBasePower));
    Pa = Pb = Pc = getP () / 3.0;
}

void ThreePhaseLoad::setLoad (double Plevel, double Qlevel, units_t unitType)
{
    setP (unitConversion (Plevel, unitType, puMW, systemBasePower));
    setQ (unitConversion (Qlevel, unitType, puMW, systemBasePower));
    Pa = Pb = Pc = getP () / 3.0;
    Qa = Qb = Qc = getQ () / 3.0;
}

static const stringVec locNumStrings{"pa", "pb", "pc", "qa", "qb", "qc"};

static const stringVec locStrStrings{

};

static const stringVec flagStrings{"use_abs_angle"};

void ThreePhaseLoad::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    getParamString<ThreePhaseLoad, Load> (this, pstr, locNumStrings, locStrStrings, flagStrings, pstype);
}

void ThreePhaseLoad::setFlag (const std::string &flag, bool val)
{
    if ((flag == "ignore_phase") || (flag == "ignorevoltagephase"))
    {
        opFlags.set (use_abs_angle, !val);
    }
    else if (flag == "use_abs_angle")
    {
        opFlags.set (use_abs_angle, val);
    }
    else
    {
        Load::setFlag (flag, val);
    }
}

// set properties
void ThreePhaseLoad::set (const std::string &param, const std::string &val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        Load::set (param, val);
    }
}

double ThreePhaseLoad::getBaseAngle () const { return (opFlags[use_abs_angle]) ? bus->getAngle () : 0.0; }
// simple template class for selecting the an appropriate value based on Phase
template <class X>
X phaseSelector (char phase, X valA, X valB, X valC, X def)
{
    switch (phase)
    {
    case 'a':
    case '1':
        return valA;
    case 'b':
    case '2':
        return valB;
    case 'c':
    case '3':
        return valC;
    default:
        return def;
    }
}

double ThreePhaseLoad::get (const std::string &param, units_t unitType) const
{
    if (param.length () == 2)
    {
        switch (param[0])
        {
        case 'p':
            return unitConversion (phaseSelector (param[1], Pa, Pb, Pc, kNullVal), puMW, unitType,
                                   systemBasePower);
        case 'q':
            return unitConversion (phaseSelector (param[1], Qa, Qb, Qc, kNullVal), puMW, unitType,
                                   systemBasePower);
        case 'v':
        {
            double V = bus->getVoltage ();
            return unitConversion (V, puV, unitType, systemBasePower, baseVoltage);
        }
        case 'a':
        {
            double A = getBaseAngle ();
            double phaseAngle = phaseSelector (param[1], A, A + 2.0 * kPI / 3.0, A + 4.0 * kPI / 3.0, kNullVal);
            return unitConversion (phaseAngle, rad, unitType, systemBasePower, baseVoltage);
        }
        default:
            break;
        }
    }
    else if (param.length () == 3)
    {
        if (param.compare (0, 2, "vi") == 0)  // get the real part of the voltage
        {
            auto Vc = std::polar (bus->getVoltage (), getBaseAngle ());
            Vc = Vc * phaseSelector (param[2], alpha0, alpha, alpha2, alpha0);
            return unitConversion (Vc.real (), puV, unitType, systemBasePower, baseVoltage);
        }
        if (param.compare (0, 2, "vj") == 0)  // get the reactive part of the voltage
        {
            auto Vc = std::polar (bus->getVoltage (), getBaseAngle ());
            Vc = Vc * phaseSelector (param[2], alpha0, alpha, alpha2, alpha0);
            return unitConversion (Vc.imag (), puV, unitType, systemBasePower, baseVoltage);
        }
    }
    else if (param.compare (0, 4, "imag") == 0)
    {
        switch (param[4])
        {
        case 'a':
        {
            auto va = std::polar (bus->getVoltage (), getBaseAngle ());
            auto sa = std::complex<double> (Pa, Qa);
            auto ia = sa / va;
            return unitConversion (std::abs (ia) / multiplier, puA, unitType, systemBasePower, baseVoltage);
        }
        case 'b':
        {
            auto vb = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha;
            auto sb = std::complex<double> (Pb, Qb);
            auto ib = sb / vb;

            return unitConversion (std::abs (ib) / multiplier, puA, unitType, systemBasePower, baseVoltage);
        }
        case 'c':
        {
            auto vc = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha2;
            auto sc = std::complex<double> (Pc, Qc);
            auto ic = sc / vc;

            return unitConversion (std::abs (ic) / multiplier, puA, unitType, systemBasePower, baseVoltage);
        }
        }
    }
    else if (param.compare (0, 6, "iangle") == 0)
    {
        switch (param[6])
        {
        case 'a':
        {
            auto va = std::polar (bus->getVoltage (), getBaseAngle ());
            auto sa = std::complex<double> (Pa, Qa);
            auto ia = sa / va;
            return unitConversion (std::arg (ia), rad, unitType);
        }
        case 'b':
        {
            auto vb = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha;
            auto sb = std::complex<double> (Pb, Qb);
            auto ib = sb / vb;
            return unitConversion (std::arg (ib), rad, unitType);
        }
        case 'c':
        {
            auto vc = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha2;
            auto sc = std::complex<double> (Pc, Qc);
            auto ic = sc / vc;
            return unitConversion (std::arg (ic), rad, unitType);
        }
        }
    }
    else if (param == "multiplier")
    {
        return multiplier;
    }
    return Load::get (param, unitType);
}

void ThreePhaseLoad::set (const std::string &param, double val, units_t unitType)
{
    if (param.length () == 2)
    {
        switch (param[0])
        {
        case 'p':
            switch (param[1])
            {
            case 'a':
                setPa (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            case 'b':
                setPb (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            case 'c':
                setPc (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            }
            break;
        case 'q':
            switch (param[1])
            {
            case 'a':
                setQa (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            case 'b':
                setQb (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            case 'c':
                setQc (unitConversion (val * multiplier, unitType, puMW, systemBasePower, baseVoltage));
                break;
            }
            break;
        default:
            Load::set (param, val, unitType);
        }
        return;
    }
    if (param.compare (0, 4, "imag") == 0)
    {
        switch (param[4])
        {
        case 'a':
        {
            auto va = std::polar (bus->getVoltage (), getBaseAngle ());
            auto sa = std::complex<double> (Pa, Qa);
            auto ia = sa / va;

            auto newia =
              std::polar (unitConversion (val, unitType, puA, systemBasePower, baseVoltage) * multiplier,
                          std::arg (ia));
            auto newP = newia * va;
            setPa (newP.real ());
            setQa (newP.imag ());
        }

        break;
        case 'b':
        {
            auto vb = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha;
            auto sb = std::complex<double> (Pb, Qb);
            auto ib = sb / vb;

            auto newib =
              std::polar (unitConversion (val, unitType, puA, systemBasePower, baseVoltage) * multiplier,
                          std::arg (ib));
            auto newP = newib * vb;
            setPb (newP.real ());
            setQb (newP.imag ());
        }
        break;
        case 'c':
        {
            auto vc = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha2;
            auto sc = std::complex<double> (Pc, Qc);
            auto ic = sc / vc;

            auto newic =
              std::polar (unitConversion (val, unitType, puA, systemBasePower, baseVoltage) * multiplier,
                          std::arg (ic));
            auto newP = newic * vc;
            setPc (newP.real ());
            setQc (newP.imag ());
        }
        break;
        }
    }
    else if (param.compare (0, 6, "iangle") == 0)
    {
        switch (param[6])
        {
        case 'a':
        {
            auto va = std::polar (bus->getVoltage (), getBaseAngle ());
            auto sa = std::complex<double> (Pa, Qa);
            auto ia = sa / va;
            auto newia = std::polar (std::abs (ia), unitConversion (val, unitType, rad));
            auto newP = newia * va;
            setPa (newP.real ());
            setQa (newP.imag ());
        }
        break;
        case 'b':
        {
            auto vb = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha;
            auto sb = std::complex<double> (Pb, Qb);
            auto ib = sb / vb;
            auto newib = std::polar (std::abs (ib), unitConversion (val, unitType, rad));
            auto newP = newib * vb;
            setPb (newP.real ());
            setQb (newP.imag ());
        }
        break;
        case 'c':
        {
            auto vc = std::polar (bus->getVoltage (), getBaseAngle ()) * alpha2;
            auto sc = std::complex<double> (Pc, Qc);
            auto ic = sc / vc;
            auto newic = std::polar (std::abs (ic), unitConversion (val, unitType, rad));
            auto newP = newic * vc;
            setPc (newP.real ());
            setQc (newP.imag ());
        }
        break;
        }
    }
    else if (param == "multiplier")
    {
        multiplier = val;
    }
    else
    {
        Load::set (param, val, unitType);
    }
}

IOdata ABCtoPNZ_R (const IOdata &abcR, const IOdata &abcI)
{
    std::complex<double> A (abcR[0], abcI[0]);
    std::complex<double> B (abcR[1], abcI[1]);
    std::complex<double> C (abcR[2], abcI[2]);

    double P = abcR[0] + abcR[1] + abcR[2];

    std::complex<double> N = A + B * alpha + C * alpha2;
    std::complex<double> Z = A + B * alpha2 + C * alpha;

    return {P, N.real (), Z.real ()};
}

IOdata ABCtoPNZ_I (const IOdata &abcR, const IOdata &abcI)
{
    std::complex<double> A (abcR[0], abcI[0]);
    std::complex<double> B (abcR[1], abcI[1]);
    std::complex<double> C (abcR[2], abcI[2]);

    double Q = abcI[0] + abcI[1] + abcI[2];

    std::complex<double> N = A + B * alpha + C * alpha2;
    std::complex<double> Z = A + B * alpha2 + C * alpha;

    return {Q, N.imag (), Z.imag ()};
}

IOdata ThreePhaseLoad::getRealPower3Phase (const IOdata & /*inputs*/,
                                           const stateData & /*sD*/,
                                           const solverMode & /*sMode*/,
                                           phase_type_t type) const
{
    return getRealPower3Phase (type);
}
IOdata ThreePhaseLoad::getReactivePower3Phase (const IOdata & /*inputs*/,
                                               const stateData & /*sD*/,
                                               const solverMode & /*sMode*/,
                                               phase_type_t type) const
{
    return getReactivePower3Phase (type);
}
/** get the 3 phase real output power that based on the given voltage
@param[in] V the bus voltage
@return the real power consumed by the load*/
IOdata ThreePhaseLoad::getRealPower3Phase (const IOdata & /*V*/, phase_type_t type) const
{
    return getRealPower3Phase (type);
}
/** get the 3 phase reactive output power that based on the given voltage
@param[in] V the bus voltage
@return the reactive power consumed by the load*/
IOdata ThreePhaseLoad::getReactivePower3Phase (const IOdata & /*V*/, phase_type_t type) const
{
    return getReactivePower3Phase (type);
}
IOdata ThreePhaseLoad::getRealPower3Phase (phase_type_t type) const
{
    switch (type)
    {
    case phase_type_t::abc:
    default:
        return {Pa, Pb, Pc};
    case phase_type_t::pnz:
        return ABCtoPNZ_R ({Pa, Pb, Pc}, {Qa, Qb, Qc});
    }
}
IOdata ThreePhaseLoad::getReactivePower3Phase (phase_type_t type) const
{
    switch (type)
    {
    case phase_type_t::abc:
    default:
        return {Qa, Qb, Qc};
    case phase_type_t::pnz:
        return ABCtoPNZ_I ({Pa, Pb, Pc}, {Qa, Qb, Qc});
    }
}

void ThreePhaseLoad::setPa (double val)
{
    Pa = val;
    setP (Pa + Pb + Pc);
}
void ThreePhaseLoad::setPb (double val)
{
    Pb = val;
    setP (Pa + Pb + Pc);
}
void ThreePhaseLoad::setPc (double val)
{
    Pc = val;
    setP (Pa + Pb + Pc);
}
void ThreePhaseLoad::setQa (double val)
{
    Qa = val;
    setQ (Qa + Qb + Qc);
}
void ThreePhaseLoad::setQb (double val)
{
    Qb = val;
    setQ (Qa + Qb + Qc);
}
void ThreePhaseLoad::setQc (double val)
{
    Qc = val;
    setQ (Qa + Qb + Qc);
}
}  // namespace loads
}  // namespace griddyn
