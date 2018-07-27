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

#include "coreObject.h"
#include "coreExceptions.h"
#include "nullObject.h"
#include "utilities/dataDictionary.h"
#include "utilities/stringOps.h"
#include "utilities/string_viewOps.h"
#include <cassert>
#include <cmath>

namespace griddyn
{
// start at 101 since there are some objects that use low numbers as a check for interface number and the id as
// secondary
std::atomic<id_type_t> coreObject::s_obcnt (101);

coreObject::coreObject (const std::string &objName) : m_refCount (0), m_oid (s_obcnt++), name (objName)
{
    static nullObject nullObject0 (0);
    // not using updateName since in many cases the id has not been set yet
    if (!name.empty () && (name.back () == '#'))
    {
        name.pop_back ();
        appendInteger (name, m_oid);
    }
    parent = &nullObject0;
}

// this constructor is only used for building some select nullObjects
coreObject::coreObject (id_type_t coid) : m_refCount (0), m_oid (coid)
{
    id = coid;
    parent = nullptr;
}
coreObject::~coreObject () = default;

static dataDictionary<id_type_t, std::string> descriptionDictionary;

// inherited copy construction method
coreObject *coreObject::clone (coreObject *obj) const
{
    if (obj == nullptr)
    {
        obj = new coreObject (name);
        descriptionDictionary.copy (m_oid, obj->m_oid);
    }
    obj->enabled = enabled;
    obj->id = id;
    obj->prevTime = prevTime;

    obj->nextUpdateTime = nextUpdateTime;
    obj->lastUpdateTime = lastUpdateTime;
    obj->updatePeriod = updatePeriod;
    obj->updateDelay = updateDelay;

    return obj;
}

void coreObject::updateName ()
{
    if (name.empty ())
    {
        return;
    }
    switch (name.back ())
    {
    case '$':
        name.pop_back ();
        appendInteger (name, id);
        break;
    case '#':
        name.pop_back ();
        appendInteger (name, m_oid);
        break;
    case '@':
        name.pop_back ();
        appendInteger (name, locIndex);
        break;
    default:
        break;
    }
}

void coreObject::add (coreObject *obj)
{
    if (obj != nullptr)
    {
        throw (objectAddFailure (this));
    }
}

void coreObject::remove (coreObject *obj)
{
    if (obj != nullptr)
    {
        throw (objectRemoveFailure (this));
    }
}

void coreObject::addHelper (std::shared_ptr<helperObject> obj)
{
    if (obj)
    {
        throw (objectAddFailure (this));
    }
}

void coreObject::addOwningReference ()
{
    // use relaxed ordering since no one cares about order on the increment operation
    m_refCount.fetch_add (1, std::memory_order_relaxed);
}

static stringVec locNumStrings{"updateperiod", "updaterate", "nextupdatetime", "basepower", "enabled", "id"};
static const stringVec locStrStrings{"name", "description"};

void coreObject::getParameterStrings (stringVec &pstr, paramStringType pstype) const
{
    switch (pstype)
    {
    case paramStringType::all:
        pstr.reserve (pstr.size () + locNumStrings.size () + locStrStrings.size () + 1);
        pstr.insert (pstr.end (), locNumStrings.begin (), locNumStrings.end ());
        pstr.emplace_back ("#");
        pstr.insert (pstr.end (), locStrStrings.begin (), locStrStrings.end ());
        break;
    case paramStringType::localnum:
        pstr = locNumStrings;
        break;
    case paramStringType::localstr:
        pstr = locStrStrings;
        break;
    case paramStringType::numeric:
        pstr.reserve (pstr.size () + locNumStrings.size ());
        pstr.insert (pstr.end (), locNumStrings.begin (), locNumStrings.end ());
        break;
    case paramStringType::str:
        pstr.reserve (pstr.size () + locStrStrings.size ());
        pstr.insert (pstr.end (), locStrStrings.begin (), locStrStrings.end ());
        break;
    case paramStringType::localflags:
        break;
    case paramStringType::flags:
        break;
    }
}

void coreObject::set (const std::string &param, const std::string &val)
{
    if ((param == "name") || (param == "rename") || (param == "id"))
    {
        setName (val);
    }
    else if (param == "description")
    {
        setDescription (val);
    }
    else if ((param.empty ()) || (param.front () == '#'))
    {
        // comment parameter meant to do nothing
    }
    else
    {
        if (val == "true")
        {
            setFlag (param, true);
        }
        else if (val == "false")
        {
            setFlag (param, false);
        }
        else
        {
            auto lower = convertToLowerCase (param);
            if (lower != param)
            {
                set (lower, val);
                LOG_WARNING (std::string ("parameters should be lower case \"") + param + "\" is not");
            }
            else
            {
                LOG_WARNING ("parameter " + param + " not found");
                throw (unrecognizedParameter (param));
            }
        }
    }
}

void coreObject::setDescription (const std::string &description)
{
    descriptionDictionary.update (m_oid, description);
}

std::string coreObject::getDescription () const { return descriptionDictionary.query (m_oid); }
void coreObject::nameUpdate () { parent->alert (this, OBJECT_NAME_CHANGE); }
void coreObject::idUpdate () { parent->alert (this, OBJECT_ID_CHANGE); }
void coreObject::setUpdateTime (double newUpdateTime) { nextUpdateTime = newUpdateTime; }
void coreObject::setParent (coreObject *parentObj)
{
    static nullObject nullObjectEp (0);
    parent = parentObj;
    if (parent == nullptr)
    {
        parent = &nullObjectEp;
    }
}

void coreObject::setFlag (const std::string &flag, bool val)
{
    if ((flag == "enable") || (flag == "status") || (flag == "enabled"))
    {
        if (isEnabled () != val)
        {
            if (val)
            {
                enable ();
            }
            else
            {
                disable ();
            }
        }
    }
    else if (flag == "disabled")
    {
        if (isEnabled () == val)  // looking for opposites to trigger
        {
            if (val)
            {
                disable ();
            }
            else
            {
                enable ();
            }
        }
    }
    else if (flag == "updates")
    {
        enable_updates (val);
    }
    else if (flag == "searchable")
    {
        alert (this, OBJECT_IS_SEARCHABLE);
    }
    else if (flag == "hasupdates")
    {
        alert (this, UPDATE_REQUIRED);
    }
    else if ((flag.empty ()) || (flag.front () == '#'))
    {
        // comment parameter meant to do nothing
    }
    else
    {
        auto lower = convertToLowerCase (flag);
        if (lower != flag)
        {
            setFlag (lower, val);
            LOG_WARNING (std::string ("flags should be lower case \"") + flag + "\" is not");
        }
        else
        {
            throw (unrecognizedParameter (flag));
        }
    }
}

bool coreObject::getFlag (const std::string &flag) const
{
    bool ret = false;
    if (flag == "enabled")
    {
        ret = isEnabled ();
    }
    else if (flag == "updates")
    {
        ret = hasUpdates ();
    }
    return ret;
}

double coreObject::get (const std::string &param, gridUnits::units_t unitType) const
{
    double val = kNullVal;
    if (param == "eventcode")
    {
        val = 0.0;
    }
    else if (param == "period")
    {
        val = gridUnits::unitConversion (static_cast<double> (updatePeriod), gridUnits::sec, unitType);
    }
    else if ((param == "time") || (param == "currenttime"))
    {
        val = static_cast<double> (prevTime);
    }
    else if ((param == "update") || (param == "nextupdate"))
    {
        val = static_cast<double> (nextUpdateTime);
    }
    else if (param == "lastupdate")
    {
        val = static_cast<double> (lastUpdateTime);
    }
    return val;
}

void coreObject::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if ((param == "updateperiod") || (param == "period"))
    {
        updatePeriod = gridUnits::unitConversion (val, unitType, gridUnits::sec);
    }
    else if ((param == "updaterate") || (param == "rate"))
    {
        double rt = gridUnits::unitConversion (val, unitType, gridUnits::Hz);
        if (rt <= 0.0)
        {
            updatePeriod = kBigNum;
        }
        else
        {
            updatePeriod = 1.0 / rt;
        }
    }
    else if (param == "nextupdatetime")
    {
        nextUpdateTime = gridUnits::unitConversion (val, unitType, gridUnits::sec);
    }
    else if ((param == "number") || (param == "renumber") || (param == "id"))
    {
        setUserID (static_cast<index_t> (val));
    }
    else if ((param.empty ()) || (param.front () == '#'))
    {
        // comment parameter meant to do nothing
    }
    else
    {
        try
        {
            setFlag (param, (val > 0.1));
        }
        catch (const unrecognizedParameter &e)
        {
            auto lower = convertToLowerCase (param);
            if (lower != param)
            {
                set (lower, val, unitType);
                LOG_WARNING (std::string ("parameters should be lower case \"") + param + "\" is not");
            }
            else
            {
                throw;
            }
        }
    }
}

std::string coreObject::getString (const std::string &param) const
{
    std::string out ("NA");
    if (param == "name")
    {
        out = name;
    }
    else if (param == "description")
    {
        out = getDescription ();
    }
    else if (param == "parent")
    {
        if (parent != nullptr)
        {
            out = parent->getName ();
        }
    }
    return out;
}

coreObject *coreObject::getSubObject (const std::string & /*typeName*/, index_t /*num*/) const { return nullptr; }
coreObject *coreObject::findByUserID (const std::string & /*typeName*/, index_t searchID) const
{
    if (searchID == id)
    {
        return const_cast<coreObject *> (this);
    }
    return nullptr;
}

void coreObject::updateA (coreTime time) { lastUpdateTime = time; }
coreTime coreObject::updateB ()
{
    assert (nextUpdateTime > negTime / 2.0);  // The assert is to check for spurious calls
    if (nextUpdateTime < maxTime)
    {
        while (lastUpdateTime >= nextUpdateTime)
        {
            nextUpdateTime += updatePeriod;
        }
    }
    return nextUpdateTime;
}

void coreObject::enable () { enabled = true; }
void coreObject::disable () { enabled = false; }
bool coreObject::isEnabled () const { return enabled; }
// core objects are cloneable derived classes may not be
bool coreObject::isCloneable () const { return true; }
void coreObject::alert (coreObject *object, int code) { parent->alert (object, code); }
void coreObject::log (coreObject *object, print_level level, const std::string &message)
{
    parent->log (object, level, message);
}

void coreObject::makeNewOID () { m_oid = ++s_obcnt; }
// NOTE: there is some potential for recursion here if the parent object searches in lower objects
// But in some cases you search up, and others you want to search down so we will rely on intelligence on the part
// of the implementer
coreObject *coreObject::find (const std::string &object) const { return (parent->find (object)); }

int coreObject::getInt (const std::string &param) const { return static_cast<int> (get (param)); }

std::string fullObjectName (const coreObject *obj)
{
    if (obj->parent->m_oid != 0u)  // the nullobject oid==0
    {
        if (obj->parent->parent->m_oid != 0u)
        {
            return fullObjectName (obj->parent) + "::" + obj->getName ();  // yay recursion
        }
        return obj->getName ();  // the objective is to be searchable from the root object so don't need to
        // list the root object name
    }
    return obj->getName ();
}

void removeReference (coreObject *objToDelete)
{
    if (objToDelete != nullptr)
    {
        if (objToDelete->m_refCount <= 1)  // don't do a write unless we absolutely need to
        {
            delete objToDelete;
        }
        else if (--objToDelete->m_refCount <= 0)  // now we need to check again if we need to delete
        {
            delete objToDelete;
        }
    }
}

void removeReference (coreObject *objToDelete, const coreObject *parent)
{
    if (objToDelete != nullptr)
    {
        if (objToDelete->m_refCount <= 1)  // don't do a write on an atomic unless we absolutely need to
        {
            delete objToDelete;
        }
        else if (--objToDelete->m_refCount <= 0)  // this is an atomic operation
        {
            delete objToDelete;
        }
        else if (parent == objToDelete->parent)
        {
            objToDelete->parent = nullptr;
        }
    }
}

void setMultipleFlags (coreObject *obj, const std::string &flags)
{
    auto lcflags = convertToLowerCase (flags);
    auto flgs = utilities::string_viewOps::split (lcflags);
    utilities::string_viewOps::trim (flgs);
    for (const auto &flag : flgs)
    {
        if (flag.empty ())
        {
            continue;
        }
        if (flag.front () != '-')
        {
            obj->setFlag (flag.to_string (), true);
        }
        else
        {
            obj->setFlag (flag.substr (1, utilities::string_view::npos).to_string (), false);
        }
    }
}

static const std::unordered_map<std::string, print_level> printLevelsMap{
  {"none", print_level::no_print},       {"error", print_level::error},
  {"warning", print_level::warning},     {"normal", print_level::normal},
  {"summary", print_level::summary},     {"debug", print_level::debug},
  {"trace", print_level::trace},         {"no_print", print_level::no_print},
  {"error_print", print_level::error},   {"warning_print", print_level::warning},
  {"normal_print", print_level::normal}, {"summary_print", print_level::summary},
  {"debug_print", print_level::debug},   {"trace_print", print_level::trace},
};

print_level stringToPrintLevel (const std::string &level)
{
    auto fnd = printLevelsMap.find (level);
    if (fnd != printLevelsMap.end ())
    {
        return fnd->second;
    }
    throw (invalidParameterValue (level));
}

}  // namespace griddyn
