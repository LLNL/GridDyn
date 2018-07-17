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

#include "fmiEvent.h"
#include "fmiCoordinator.h"

namespace griddyn
{
namespace fmi
{
fmiEvent::fmiEvent (const std::string &newName, fmiEventType type) : reversibleEvent (newName), eventType (type) {}

fmiEvent::fmiEvent (fmiEventType type) : eventType (type) {}

fmiEvent::fmiEvent (const EventInfo &gdEI, coreObject *rootObject) : reversibleEvent (gdEI, rootObject)
{
    findCoordinator ();
}

std::unique_ptr<Event> fmiEvent::clone () const
{
    std::unique_ptr<Event> evnt = std::make_unique<fmiEvent> ();
    cloneTo (evnt.get ());
    return evnt;
}

void fmiEvent::cloneTo (Event *evnt) const
{
    reversibleEvent::cloneTo (evnt);
    auto fe = dynamic_cast<fmiEvent *> (evnt);
    if (fe == nullptr)
    {
        return;
    }
    // gp->valueref = valueref;
}

void fmiEvent::set (const std::string &param, double val)
{
    if ((param == "vr") || (param == "valuereference"))
    {
        // valueref = static_cast<unsigned int>(val);
    }
    else
    {
        reversibleEvent::set (param, val);
    }
}

void fmiEvent::set (const std::string &param, const std::string &val)
{
    if (param == "datatype")
    {
        if (val == "string")
        {
            eventType = fmiEventType::string_parameter;
            stringEvent = true;
        }
        else if ((val == "real") || (val == "number"))
        {
            eventType = fmiEventType::parameter;
            stringEvent = false;
        }
    }
    else
    {
        reversibleEvent::set (param, val);
    }
}

void fmiEvent::updateEvent (const EventInfo &gdEI, coreObject *rootObject)
{
    reversibleEvent::updateEvent (gdEI, rootObject);
    findCoordinator ();
}

bool fmiEvent::setTarget (coreObject *gdo, const std::string &var)
{
    auto ret = reversibleEvent::setTarget (gdo, var);
    if (ret)
    {
        findCoordinator ();
    }
    return ret;
}

coreObject *fmiEvent::getOwner () const { return coord; }

void fmiEvent::updateObject (coreObject *gco, object_update_mode mode)
{
    reversibleEvent::updateObject (gco, mode);
    findCoordinator ();
}

void fmiEvent::findCoordinator ()
{
    if (m_obj != nullptr)
    {
        auto rto = m_obj->getRoot ();
        if (rto != nullptr)
        {
            auto fmiCont = rto->find ("fmiCoordinator");
            if (dynamic_cast<fmiCoordinator *> (fmiCont) != nullptr)
            {
                if (!isSameObject (fmiCont, coord))
                {
                    coord = static_cast<fmiCoordinator *> (fmiCont);
                    if (eventType == fmiEventType::input)
                    {
                        coord->registerInput (getName (), this);
                    }
                    else
                    {
                        coord->registerParameter (getName (), this);
                    }
                }
            }
        }
    }
}

}  // namespace fmi
}  // namespace griddyn
