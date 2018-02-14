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

#include "rampSource.h"
#include "core/coreObjectTemplates.hpp"
#include "utilities/vectorOps.hpp"
#include <ctime>

namespace griddyn
{
namespace sources
{
rampSource::rampSource (const std::string &objName, double startVal) : Source (objName, startVal) {}
coreObject *rampSource::clone (coreObject *obj) const
{
    auto ld = cloneBase<rampSource, Source> (this, obj);
    if (ld == nullptr)
    {
        return obj;
    }
    ld->mp_dOdt = mp_dOdt;
    return ld;
}

// set properties
void rampSource::set (const std::string &param, const std::string &val) { Source::set (param, val); }
void rampSource::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "dodt") || (param == "ramp") || (param == "rate"))
    {
        mp_dOdt = val;
    }
    else
    {
        Source::set (param, val, unitType);
    }
}

double rampSource::computeOutput (coreTime time) const
{
    auto tdiff = time - prevTime;
    return m_output + mp_dOdt * tdiff;
}

double rampSource::getDoutdt (const IOdata & /*inputs*/,
                              const stateData & /*sD*/,
                              const solverMode & /*sMode*/,
                              index_t num) const
{
    return (num == 0) ? mp_dOdt : 0.0;
}
}  // namespace sources
}  // namespace griddyn
