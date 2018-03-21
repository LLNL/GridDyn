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

#include "longLine.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "../primary/acBus.h"

#include <cmath>

namespace griddyn
{
namespace links
{
longLine::longLine (const std::string &objName) : subsystem (objName) {}
coreObject *longLine::clone (coreObject *obj) const
{
    auto line = cloneBase<longLine, Link> (this, obj);
    if (line == nullptr)
    {
        return obj;
    }
    line->segmentationLength = segmentationLength;
    if (opFlags[pFlow_initialized])
    {
        subsystem::clone (line);
    }
    return line;
}
// add components
void longLine::add (coreObject * /*obj*/) { throw (unrecognizedObjectException (this)); }
// remove components
void longLine::remove (coreObject * /*obj*/) {}
void longLine::pFlowObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    generateIntermediateLinks ();
    return subsystem::pFlowObjectInitializeA (time0, flags);
}

void longLine::set (const std::string &param, const std::string &val) { return Link::set (param, val); }
void longLine::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (param.length () == 1)
    {
        switch (param[0])
        {
        case 'r':
            r = val;
            break;
        case 'x':
            x = val;
            break;
        case 'b':
            mp_B = val;
            break;
        case 'g':
            mp_G = val;
            break;

        default:
            throw (unrecognizedParameter (param));
        }
        return;
    }

    if ((param == "segmentationlength") || (param == "segmentlength"))
    {
        segmentationLength = gridUnits::unitConversionDistance (val, unitType, gridUnits::km);
    }
    else if (param == "length")
    {
        length = unitConversionDistance (val, unitType, gridUnits::km);
    }
    else if (param == "fault")
    {
        if (opFlags[pFlow_initialized])
        {
            fault = val;
            if (fault > 1.0)
            {
                fault = -1;
            }
            if (fault >= 0)
            {
                auto nb = getInt ("linkcount");
                double fs = 1.0 / static_cast<double> (nb);
                int kk = 0;
                double cm = fs;
                while (cm < fault)
                {
                    ++kk;
                    cm += fs;
                }
                double newfaultVal = fault - cm + fs;

                if (faultLink >= 0)  // if there is an existing fault move it
                {
                    getLink (faultLink)->set ("fault", -1.0);
                }
                getLink (kk)->set ("fault", newfaultVal);
                faultLink = kk;
            }
            else
            {
                if (faultLink >= 0)
                {
                    getLink (faultLink)->set ("fault", -1.0);
                    faultLink = -1;
                }
            }
        }
        else
        {
            fault = val;
        }
    }
    else
    {
        Link::set (param, val, unitType);  // bypass subsystem set function
    }
}

double longLine::get (const std::string &param, gridUnits::units_t unitType) const
{
    double val = kNullVal;
    if (param == "segmentationlength")
    {
        val = segmentationLength;
    }
    else
    {
        val = subsystem::get (param, unitType);
    }
    return val;
}

void longLine::generateIntermediateLinks ()
{
    int numLinks = std::ceil (length / segmentationLength);

    double sr = r / static_cast<double> (numLinks);
    double sx = x / static_cast<double> (numLinks);
    double sB = mp_B / static_cast<double> (numLinks);
    double sG = mp_G / static_cast<double> (numLinks);

    int clinks = getInt ("linkCount");
    Link *link;

    if (clinks == 0)
    {
        link = new acLine (sr, sx);
        link->set ("b", sB);
        if (sG != 0)
        {
            link->set ("g", sG);
        }
        subsystem::add (link);
        clinks = 1;
        terminalLink[0] = link;
    }
    else
    {
        for (int pp = 0; pp < clinks; ++pp)
        {
            link = subarea.getLink (pp);
            link->set ("r", sr);
            link->set ("x", sx);
            link->set ("b", sB);
            if (sG != 0)
            {
                link->set ("g", sG);
            }
        }
    }
    for (int pp = clinks; pp < numLinks; ++pp)
    {
        gridBus *bus = new acBus ("ibus" + std::to_string (pp));

        subsystem::add (bus);

        link = new acLine (sr, sx);
        link->set ("b", sB);
        if (sG != 0)
        {
            link->set ("g", sG);
        }
        subsystem::add (link);
        link->updateBus (bus, 1);

        subarea.getLink (pp - 1)->updateBus (bus, 2);
        if (pp == numLinks - 1)
        {
            terminalLink[1] = link;
        }
    }
}

}  // namespace links
}  // namespace griddyn