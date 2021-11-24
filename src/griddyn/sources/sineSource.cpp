/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sineSource.h"

#include "core/coreObjectTemplates.hpp"
#include <cmath>

namespace griddyn {
namespace sources {
    /*
enum pulse_type_t{ square = 0, triangle = 1, gaussian = 2, biexponential = 3, exponential = 4 };
pulse_type_t ptype;
protected:
double period;
double duty_cylce;
double A;
double nextCycleTime;*/

    sineSource::sineSource(const std::string& objName, double startVal):
        pulseSource(objName, startVal)
    {
    }
    coreObject* sineSource::clone(coreObject* obj) const
    {
        auto nobj = cloneBase<sineSource, pulseSource>(this, obj);
        if (nobj == nullptr) {
            return obj;
        }
        nobj->Amp = Amp;
        nobj->frequency = frequency;
        nobj->phase = phase;
        nobj->lastCycle = lastCycle;
        nobj->sinePeriod = sinePeriod;
        nobj->dfdt = dfdt;
        nobj->dAdt = dAdt;
        return nobj;
    }

    void sineSource::pFlowObjectInitializeA(coreTime time0, std::uint32_t flags)
    {
        if (frequency <= 0.0) {
            lastCycle = negTime;
        } else {
            lastCycle = time0 - phase / (frequency * 2.0 * kPI);
        }
        pulseSource::pFlowObjectInitializeA(time0, flags);
        updateOutput(time0);
    }

    double sineSource::computeOutput(coreTime time) const
    {
        std::cout << "sineSource::computeOutput start" << std::endl;
        auto dt = time - prevTime;
        std::cout << "dt = " << dt << std::endl;
        if (dt == timeZero) {
            return m_output;
        }
        // account for the frequency shift
        double Nfrequency = frequency + dfdt * dt;
        double NAmp = Amp + dAdt * dt;
        // compute the sine wave component
        auto tdiff = time - lastCycle;
        double addComponent = NAmp * sin(2.0 * kPI * (Nfrequency * tdiff) + phase);
        double mult = 1.0;

        std::cout << "  Nfrequency   = " << Nfrequency   << std::endl;
        std::cout << "  NAmp         = " << NAmp         << std::endl;
        std::cout << "  time         = " << time         << std::endl;
        std::cout << "  lastCycle    = " << lastCycle    << std::endl;
        std::cout << "  tdiff        = " << tdiff        << std::endl;
        std::cout << "  addComponent = " << addComponent << std::endl;
        std::cout << "  mult         = " << mult         << std::endl;

        if (opFlags[pulsed_flag]) {
            auto tdiff2 = time - cycleTime;
            std::cout << "  tdiff2         = " << tdiff2         << std::endl;
            if (tdiff2 > period) {
                tdiff2 = tdiff2 % period;
            }
            std::cout << "  tdiff2         = " << tdiff2         << std::endl;
            mult = pulseCalc(tdiff2);
            std::cout << "  mult           = " << mult           << std::endl;
        }

        auto tmp = baseValue + (mult * addComponent);
        std::cout << "  return = " << tmp << std::endl;
        std::cout << "sineSource::computeOutput end" << std::endl;
        return tmp;
    }

    void sineSource::updateOutput(coreTime time)
    {
        std::cout << "sineSource::updateOutput start" << std::endl;
        auto dt = time - prevTime;
        if (dt == timeZero) {
            std::cout << "sineSource::updateOutput end -- dt timeZero" << std::endl;
            return;
        }

        // HACK to skip update
        return;

        std::cout << "  time         = " << time      << std::endl;
        std::cout << "  prevTime     = " << prevTime  << std::endl;
        std::cout << "  dt           = " << dt        << std::endl;

        auto tdiff = time - lastCycle;

        std::cout << "  lastCycle    = " << lastCycle << std::endl;
        std::cout << "  tdiff        = " << tdiff     << std::endl;

        // account for the frequency shift
        frequency += dfdt * (time - prevTime);
        Amp += dAdt * (time - prevTime);
        // compute the sine wave component
        double addComponent = Amp * sin(2.0 * kPI * (frequency * tdiff) + phase);
        double mult = 1.0;
        while (tdiff > sinePeriod) {
            tdiff -= sinePeriod;
            lastCycle += sinePeriod;
        }
        if (opFlags[pulsed_flag]) {
            auto tdiff2 = time - cycleTime;
            while (tdiff2 > period) {
                cycleTime += period;
                tdiff2 -= period;
            }
            mult = pulseCalc(tdiff2);
        }

        std::cout << "  frequency    = " << frequency    << std::endl;
        std::cout << "  Amp          = " << Amp          << std::endl;
        std::cout << "  addComponent = " << addComponent << std::endl;
        std::cout << "  mult         = " << mult         << std::endl;
        std::cout << "  tdiff        = " << tdiff        << std::endl;
        std::cout << "  lastCycle    = " << lastCycle    << std::endl;

        m_output = baseValue + (mult * addComponent);
        m_tempOut = m_output;
        prevTime = time;

        std::cout << "  m_output    = " << m_output << std::endl;
        std::cout << "  m_tempOut   = " << m_output << std::endl;
        std::cout << "  prevTime    = " << prevTime << std::endl;

        std::cout << "sineSource::updateOutput end -- dt not timeZero" << std::endl;
    }

    void sineSource::set(const std::string& param, const std::string& val)
    {
        pulseSource::set(param, val);
    }
    void sineSource::set(const std::string& param, double val, units::unit unitType)
    {
        if ((param == "a") || (param == "amplitude") || (param == "amp")) {
            Amp = val;
        } else if ((param == "frequency") || (param == "freq") || (param == "f")) {
            frequency = val;
            sinePeriod = 1.0 / frequency;
        } else if ((param == "period") || (param == "sineperiod")) {
            sinePeriod = val;
            frequency = 1.0 / val;
        } else if (param == "pulseperiod") {
            pulseSource::set("period", val, unitType);
        } else if (param == "pulseamplitude") {
            pulseSource::set("amplitude", val, unitType);
        } else if (param == "phase") {
            phase = val;
        } else if (param == "dfdt") {
            dfdt = val;
        } else if (param == "dadt") {
            dAdt = val;
        } else if (param == "pulsed") {
            if (val > 0.0) {
                if (!(opFlags[pulsed_flag])) {
                    cycleTime = prevTime;
                }
                opFlags.set(pulsed_flag);
            } else {
                opFlags.reset(pulsed_flag);
            }
        } else {
            pulseSource::set(param, val, unitType);
        }
    }
}  // namespace sources
}  // namespace griddyn
