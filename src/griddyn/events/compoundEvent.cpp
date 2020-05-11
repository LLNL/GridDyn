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

#include "compoundEvent.h"

#include "core/coreExceptions.h"
#include "core/objectInterpreter.h"
#include "gmlc/utilities/stringOps.h"
#include <sstream>

namespace griddyn {
namespace events {
    compoundEvent::compoundEvent(coreTime time0): Event(time0) {}

    compoundEvent::compoundEvent(const std::string& eventName): Event(eventName) {}

    compoundEvent::compoundEvent(const EventInfo& gdEI, coreObject* rootObject):
        Event(gdEI, rootObject)
    {
    }

    std::unique_ptr<Event> compoundEvent::clone() const
    {
        std::unique_ptr<Event> upE = std::make_unique<compoundEvent>(getName());
        cloneTo(upE.get());
        return upE;
    }

    void compoundEvent::cloneTo(Event* gE) const
    {
        Event::cloneTo(gE);
        auto nE = dynamic_cast<compoundEvent*>(gE);
        if (nE == nullptr) {
            return;
        }
        nE->fields = fields;
        nE->values = values;
        nE->units = units;
        nE->targetObjects = targetObjects;
    }

    void compoundEvent::updateObject(coreObject* gco, object_update_mode mode)
    {
        // TODO:  more thinking on exception safety
        if (mode == object_update_mode::direct) {
            setTarget(gco);
        } else if (mode == object_update_mode::match) {
            for (auto& obj : targetObjects) {
                if (obj != nullptr) {
                    auto tempobj = findMatchingObject(obj, gco);
                    if (tempobj == nullptr) {
                        throw(objectUpdateFailException());
                    }
                    obj = tempobj;
                }
            }
        }
    }

    coreObject* compoundEvent::getObject() const { return targetObjects[0]; }

    void compoundEvent::getObjects(std::vector<coreObject*>& objects) const
    {
        for (auto& obj : targetObjects) {
            objects.push_back(obj);
        }
    }

    void compoundEvent::setValue(double val, units::unit newUnits)
    {
        // TODO(pt):: THIS has issues
        for (auto& vv : values) {
            vv = val;
        }
        for (auto& uu : units) {
            uu = newUnits;
        }
    }

    void compoundEvent::set(const std::string& param, double val)
    {
        if (param[0] == '#') {
        } else {
            Event::set(param, val);
        }
    }

    void compoundEvent::set(const std::string& param, const std::string& val)
    {
        if (param[0] == '#') {
        } else {
            Event::set(param, val);
        }
    }

    void compoundEvent::setValue(const std::vector<double>& val) { values = val; }

    std::string compoundEvent::to_string()
    {
        // @time1[,time2,time3,... |+ period] >[rootobj::obj:]field(units) = val1,[val2,val3,...]
        std::stringstream ss;
        ss << '@' << triggerTime << " | ";

        ss << fullObjectName(targetObjects[0]) << ':' << fields[0];
        if (units[0] != units::defunit) {
            ss << '(' << units::to_string(units[0]) << ')';
        }
        ss << " = " << values[0];
        for (index_t kk = 1; kk < static_cast<index_t>(values.size()); ++kk) {
            ss << "; " << fullObjectName(targetObjects[kk]) << ':' << fields[kk];
            if (units[kk] != units::defunit) {
                ss << '(' << units::to_string(units[kk]) << ')';
            }
            ss << " = " << values[kk];
        }

        return ss.str();
    }
    change_code compoundEvent::trigger()
    {
        try {
            m_obj->set(field, value, unitType);
            return change_code::parameter_change;
        }
        catch (const std::invalid_argument&) {
            return change_code::execution_failure;
        }
    }

    change_code compoundEvent::trigger(coreTime time)
    {
        change_code ret = change_code::not_triggered;
        if (time >= triggerTime) {
            try {
                m_obj->set(field, value, unitType);
                ret = change_code::parameter_change;
            }
            catch (const std::invalid_argument&) {
                ret = change_code::execution_failure;
            }
        }
        return ret;
    }

    bool compoundEvent::setTarget(coreObject* gdo, const std::string& var)
    {
        if (!var.empty()) {
            field = var;
        }
        if (gdo != nullptr) {
            m_obj = gdo;
        }

        if (m_obj != nullptr) {
            setName(m_obj->getName());
            armed = true;
        } else {
            armed = false;
        }
        return armed;
    }
}  // namespace events
}  // namespace griddyn
