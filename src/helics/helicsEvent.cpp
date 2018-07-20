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

#include "helicsCoordinator.h"
#include "helicsEvent.h"
#include "helics/application_api/Subscriptions.hpp"

namespace griddyn
{
namespace helicsLib
{
helicsEvent::helicsEvent (const std::string &newName) : reversibleEvent (newName) {}


helicsEvent::helicsEvent (helicsEventType type) : eventType (type) {}

helicsEvent::helicsEvent (const EventInfo &gdEI, coreObject *rootObject) : reversibleEvent (gdEI, rootObject)
{
    findCoordinator ();
}

std::unique_ptr<Event> helicsEvent::clone () const
{
    std::unique_ptr<Event> evnt = std::make_unique<helicsEvent> ();
    cloneTo (evnt.get ());
    return evnt;
}

void helicsEvent::cloneTo (Event *evnt) const
{
    reversibleEvent::cloneTo (evnt);
    auto fe = dynamic_cast<helicsEvent *> (evnt);
    if (fe == nullptr)
    {
        return;
    }
    // gp->valueref = valueref;
}

void helicsEvent::set (const std::string &param, double val)
{
    if (param=="delta")
    {
        // valueref = static_cast<unsigned int>(val);
    }
    else
    {
        reversibleEvent::set (param, val);
    }
}

void helicsEvent::set (const std::string &param, const std::string &val)
{
    if (param == "datatype")
    {
        if (val == "string")
        {
            eventType = helicsEventType::string_parameter;
            stringEvent = true;
        }
        else if ((val == "real") || (val == "number"))
        {
            eventType = helicsEventType::parameter;
            stringEvent = false;
        }
    }
	else if (param == "key")
	{
        key = val;
	}
    else
    {
        reversibleEvent::set (param, val);
    }
}

void helicsEvent::updateEvent (const EventInfo &gdEI, coreObject *rootObject)
{
    reversibleEvent::updateEvent (gdEI, rootObject);
    findCoordinator ();
}

bool helicsEvent::setTarget (coreObject *gdo, const std::string &var)
{
    auto ret = reversibleEvent::setTarget (gdo, var);
    if (ret)
    {
        findCoordinator ();
    }
    return ret;
}

coreObject *helicsEvent::getOwner () const { return coord; }

void helicsEvent::initialize()
{
    if (coord!=nullptr)
    {
        findCoordinator ();
		if (coord == nullptr)
		{
            return;
		}
    }
    auto sub = coord->getSubscriptionPointer (subid);
	if (sub == nullptr)
	{
        return;
	}
	if (eventType == helicsEventType::string_parameter)
	{
		sub->registerCallback<std::string>([this](const std::string &update, helics::Time /*tval*/) { updateStringValue (update);
            trigger ();
		});
	}
	else
	{
        sub->registerCallback<double> ([this](double update, helics::Time /*tval*/) {
            setValue (update);
            trigger ();
        });
	}
    
}

void helicsEvent::updateObject (coreObject *gco, object_update_mode mode)
{
    reversibleEvent::updateObject (gco, mode);
    findCoordinator ();
}

void helicsEvent::findCoordinator ()
{
    if (m_obj)
    {
        auto rto = m_obj->getRoot ();
        if (rto)
        {
            auto helicsCont = rto->find ("helics");
            if (dynamic_cast<helicsCoordinator *> (helicsCont))
            {
                if (!isSameObject (helicsCont, coord))
                {
                    coord = static_cast<helicsCoordinator *> (helicsCont);
                    subid=coord->addSubscription (key);
                   
                }
            }
        }
    }
}

}  // namespace helicsLib
}  // namespace griddyn