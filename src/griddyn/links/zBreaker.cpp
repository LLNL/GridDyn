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

// headers
#include "links/zBreaker.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBus.h"
#include "utilities/stringOps.h"

#include "core/objectFactoryTemplates.hpp"

namespace griddyn
{
namespace links
{
using namespace gridUnits;

static typeFactory<zBreaker> glf ("link", stringVec{"zbreaker", "zline", "busbreaker"});

zBreaker::zBreaker (const std::string &objName) : Link (objName) { opFlags.set (network_connected); }
coreObject *zBreaker::clone (coreObject *obj) const
{
    auto lnk = cloneBase<zBreaker, Link> (this, obj);
    if (lnk == nullptr)
    {
        return obj;
    }
    return lnk;
}
// parameter set functions

void zBreaker::set (const std::string &param, const std::string &val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        Link::set (param, val);
    }
}

void zBreaker::set (const std::string &param, double val, units_t unitType)
{
    if (param[0] == '#')
    {
    }
    else
    {
        Link::set (param, val, unitType);
    }
}

void zBreaker::switchChange (int /*switchNum*/) { coordinateMergeStatus (); }
void zBreaker::pFlowObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/) { coordinateMergeStatus (); }
void zBreaker::dynObjectInitializeA (coreTime /*time0*/, std::uint32_t /*flags*/) { coordinateMergeStatus (); }
void zBreaker::switchMode (index_t /*num*/, bool mode)
{
    // TODO:PT: this shouldn't cause enable disable, I need to replace this with some of the checks for enabled
    // disable
    if (mode == opFlags[switch1_open_flag])
    {
        return;
    }

    opFlags.flip (switch1_open_flag);
    opFlags.flip (switch2_open_flag);
    if (opFlags[pFlow_initialized])
    {
        if (linkInfo.v1 < 0.2)
        {
            alert (this, POTENTIAL_FAULT_CHANGE);
        }
        coordinateMergeStatus ();
    }

    /*if (opFlags[switch2_open_flag])
    {
    enable();
    opFlags.reset(switch2_open_flag);
    }
    else
    {
    disable();
    opFlags.set(switch2_open_flag);
    }*/
}

void zBreaker::updateLocalCache ()
{
    if (!isEnabled ())
    {
        return;
    }
    linkInfo.v1 = B1->getVoltage ();
    linkInfo.v2 = linkInfo.v1;
}
void zBreaker::updateLocalCache (const IOdata & /*inputs*/, const stateData &sD, const solverMode & /*sMode*/)
{
    if (!isEnabled ())
    {
        return;
    }
    if (!sD.updateRequired (linkInfo.seqID))
    {
        return;
    }
    std::memset (&linkInfo, 0, sizeof (linkI));
    linkInfo.seqID = sD.seqID;
    linkInfo.v1 = B1->getVoltage ();
    linkInfo.v2 = linkInfo.v1;
}

double zBreaker::quickupdateP () { return 0; }
void zBreaker::coordinateMergeStatus ()
{
    if (isConnected ())
    {
        if (!merged)
        {
            merge ();
        }
    }
    else if (merged)
    {
        unmerge ();
    }
}
void zBreaker::merge ()
{
    B1->mergeBus (B2);
    merged = true;
}

void zBreaker::unmerge ()
{
    B1->unmergeBus (B2);
    merged = false;
}


int zBreaker::fixRealPower (double /*power*/,
	id_type_t /*measureTerminal*/,
	id_type_t /*fixedTerminal*/,
                            gridUnits::units_t /*unitType*/)
{
    return 1;
}
int zBreaker::fixPower (double /*rPower*/,
                        double /*qPower*/,
	id_type_t /*measureTerminal*/,
	id_type_t /*fixedTerminal*/,
                        gridUnits::units_t /*unitType*/)
{
    return 1;
}

}  // namespace links
}  // namespace griddyn