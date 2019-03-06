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

// headers
#include "gridAreaOpt.h"
#include "../optObjectFactory.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "gridBusOpt.h"
#include "gridLinkOpt.h"
#include "gridRelayOpt.h"
#include "griddyn/Area.h"
#include "griddyn/Link.h"
#include "griddyn/gridBus.h"
#include "utilities/stringOps.h"
#include "utilities/vectData.hpp"
#include "utilities/vectorOps.hpp"

namespace griddyn
{
using namespace gridUnits;

static optObjectFactory<gridAreaOpt, Area> opa ("basic", "area");

gridAreaOpt::gridAreaOpt (const std::string &objName) : gridOptObject (objName) {}

gridAreaOpt::gridAreaOpt (coreObject *obj, const std::string &objName)
    : gridOptObject (objName), area (dynamic_cast<Area *> (obj))
{
    if (area != nullptr)
    {
        if (getName ().empty ())
        {
            setName (area->getName ());
        }
        setUserID (area->getUserID ());
    }
}

// destructor
gridAreaOpt::~gridAreaOpt ()
{
    for (auto &obj : objectList)
    {
        removeReference (obj, this);
    }
}

coreObject *gridAreaOpt::clone (coreObject *obj) const
{
    auto nobj = cloneBase<gridAreaOpt, gridOptObject> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    // now clone all the loads and generators
    // cloning the links from this component would be bad
    // clone the generators and loads

    for (size_t kk = 0; kk < busList.size (); ++kk)
    {
        if (kk >= nobj->busList.size ())
        {
            nobj->add (static_cast<gridBusOpt *> (busList[kk]->clone (nullptr)));
        }
        else
        {
            busList[kk]->clone (nobj->busList[kk]);
        }
    }
    for (size_t kk = 0; kk < areaList.size (); ++kk)
    {
        if (kk >= nobj->areaList.size ())
        {
            nobj->add (static_cast<gridAreaOpt *> (areaList[kk]->clone (nullptr)));
        }
        else
        {
            areaList[kk]->clone (nobj->areaList[kk]);
        }
    }
    for (size_t kk = 0; kk < linkList.size (); ++kk)
    {
        if (kk >= nobj->linkList.size ())
        {
            nobj->add (static_cast<gridLinkOpt *> (linkList[kk]->clone (nullptr)));
        }
        else
        {
            linkList[kk]->clone (nobj->linkList[kk]);
        }
    }
    for (size_t kk = 0; kk < relayList.size (); ++kk)
    {
        if (kk >= nobj->relayList.size ())
        {
            nobj->add (static_cast<gridRelayOpt *> (relayList[kk]->clone (nullptr)));
        }
        else
        {
            relayList[kk]->clone (nobj->relayList[kk]);
        }
    }
    return nobj;
}

void gridAreaOpt::dynObjectInitializeA (std::uint32_t flags)
{
    // first do a check to make sure all gridDyn areas are represented by gridDynOpt Area
    Link *Lnk;
    auto coof = coreOptObjectFactory::instance ();
    bool found;
    std::vector<gridOptObject *> newObj;
    gridOptObject *oo;

    index_t kk = 0;

    // make sure all areas have an opt object
    auto areaObj = area->getArea (kk);

    while (areaObj != nullptr)
    {
        found = false;
        for (auto oa : areaList)
        {
            if (areaObj->getID () == oa->getID ())
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            oo = coof->createObject (areaObj);
            newObj.push_back (oo);
        }
        ++kk;
        areaObj = area->getArea (kk);
    }
    for (auto no : newObj)
    {
        add (no);
    }
    newObj.clear ();
    // make sure all buses have an opt object
    auto bus = area->getBus (kk);

    while (bus != nullptr)
    {
        found = false;
        for (auto oa : busList)
        {
            if (isSameObject (bus, oa))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            oo = coof->createObject (bus);
            newObj.push_back (oo);
        }
        ++kk;
        bus = area->getBus (kk);
    }
    for (auto no : newObj)
    {
        add (no);
    }
    newObj.clear ();
    // make sure all links have an opt object
    Lnk = area->getLink (kk);

    while (Lnk != nullptr)
    {
        found = false;
        for (auto oa : linkList)
        {
            if (isSameObject (Lnk, oa))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            oo = coof->createObject (Lnk);
            newObj.push_back (oo);
        }
        ++kk;
        Lnk = area->getLink (kk);
    }
    for (auto no : newObj)
    {
        add (no);
    }
    newObj.clear ();

    for (auto obj : objectList)
    {
        obj->dynInitializeA (flags);
    }
}

void gridAreaOpt::loadSizes (const optimMode &oMode)
{
    auto &oo = offsets.getOffsets (oMode);
    oo.reset ();
    switch (oMode.flowMode)
    {
    case flowModel_t::none:
    default:
        break;
    case flowModel_t::transport:
        break;
    case flowModel_t::dc:
    case flowModel_t::ac:
        break;
    }

    for (auto obj : objectList)
    {
        obj->loadSizes (oMode);
        oo.addSizes (obj->offsets.getOffsets (oMode));
    }
}

void gridAreaOpt::setValues (const optimData &oD, const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->setValues (oD, oMode);
    }
}
// for saving the state
void gridAreaOpt::guessState (double time, double val[], const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->guessState (time, val, oMode);
    }
}

void gridAreaOpt::getTols (double tols[], const optimMode &oMode)

{
    for (auto obj : objectList)
    {
        obj->getTols (tols, oMode);
    }
}

void gridAreaOpt::getVariableType (double sdata[], const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->getVariableType (sdata, oMode);
    }
}

void gridAreaOpt::valueBounds (double time, double upperLimit[], double lowerLimit[], const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->valueBounds (time, upperLimit, lowerLimit, oMode);
    }
}

void gridAreaOpt::linearObj (const optimData &oD, vectData<double> &linObj, const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->linearObj (oD, linObj, oMode);
    }
}
void gridAreaOpt::quadraticObj (const optimData &oD,
                                vectData<double> &linObj,
                                vectData<double> &quadObj,
                                const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->quadraticObj (oD, linObj, quadObj, oMode);
    }
}

double gridAreaOpt::objValue (const optimData &oD, const optimMode &oMode)
{
    double cost = 0;
    for (auto obj : objectList)
    {
        cost += obj->objValue (oD, oMode);
    }

    return cost;
}

void gridAreaOpt::gradient (const optimData &oD, double deriv[], const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->gradient (oD, deriv, oMode);
    }
}
void gridAreaOpt::jacobianElements (const optimData &oD, matrixData<double> &md, const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->jacobianElements (oD, md, oMode);
    }
}
void gridAreaOpt::getConstraints (const optimData &oD,
                                  matrixData<double> &cons,
                                  double upperLimit[],
                                  double lowerLimit[],
                                  const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->getConstraints (oD, cons, upperLimit, lowerLimit, oMode);
    }
}

void gridAreaOpt::constraintValue (const optimData &oD, double cVals[], const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->constraintValue (oD, cVals, oMode);
    }
}

void gridAreaOpt::constraintJacobianElements (const optimData &oD, matrixData<double> &md, const optimMode &oMode)
{
    for (auto obj : objectList)
    {
        obj->constraintJacobianElements (oD, md, oMode);
    }
}
void gridAreaOpt::getObjName (stringVec &objNames, const optimMode &oMode, const std::string &prefix)
{
    for (auto obj : objectList)
    {
        obj->getObjName (objNames, oMode, prefix);
    }
}

void gridAreaOpt::disable ()
{
    gridOptObject::disable ();
    for (auto &link : linkList)
    {
        link->disable ();
    }
}

void gridAreaOpt::setOffsets (const optimOffsets &newOffsets, const optimMode &oMode)
{
    offsets.setOffsets (newOffsets, oMode);
    optimOffsets no (offsets.getOffsets (oMode));
    no.localLoad ();

    for (auto sa : areaList)
    {
        sa->setOffsets (no, oMode);
        no.increment (sa->offsets.getOffsets (oMode));
    }
    for (auto bus : busList)
    {
        bus->setOffsets (no, oMode);
        no.increment (bus->offsets.getOffsets (oMode));
    }
    for (auto link : linkList)
    {
        link->setOffsets (no, oMode);
        no.increment (link->offsets.getOffsets (oMode));
    }
    for (auto relay : relayList)
    {
        relay->setOffsets (no, oMode);
        no.increment (relay->offsets.getOffsets (oMode));
    }
}

void gridAreaOpt::setOffset (index_t offset, index_t constraintOffset, const optimMode &oMode)
{
    for (auto sa : areaList)
    {
        sa->setOffset (offset, constraintOffset, oMode);
        constraintOffset += sa->constraintSize (oMode);
        offset += sa->objSize (oMode);
    }
    for (auto bus : busList)
    {
        bus->setOffset (offset, constraintOffset, oMode);
        constraintOffset += bus->constraintSize (oMode);
        offset += bus->objSize (oMode);
    }
    for (auto link : linkList)
    {
        link->setOffset (offset, constraintOffset, oMode);
        constraintOffset += link->constraintSize (oMode);
        offset += link->objSize (oMode);
    }
    for (auto relay : relayList)
    {
        relay->setOffset (offset, constraintOffset, oMode);
        constraintOffset += relay->constraintSize (oMode);
        offset += relay->objSize (oMode);
    }
    offsets.setConstraintOffset (constraintOffset, oMode);
    offsets.setOffset (offset, oMode);
}

void gridAreaOpt::add (coreObject *obj)
{
    if (dynamic_cast<Area *> (obj) != nullptr)
    {
        area = static_cast<Area *> (obj);
        if (getName ().empty ())
        {
            setName (area->getName ());
        }
        setUserID (area->getUserID ());
        return;
    }
    auto *sa = dynamic_cast<gridAreaOpt *> (obj);
    if (sa != nullptr)
    {
        return add (sa);
    }

    auto *bus = dynamic_cast<gridBusOpt *> (obj);
    if (bus != nullptr)
    {
        return add (bus);
    }

    auto *lnk = dynamic_cast<gridLinkOpt *> (obj);
    if (lnk != nullptr)
    {
        return add (lnk);
    }
    auto *relay = dynamic_cast<gridRelayOpt *> (obj);
    if (relay != nullptr)
    {
        return add (relay);
    }
    throw (unrecognizedObjectException (this));
}

void gridAreaOpt::remove (coreObject *obj)
{
    auto *sa = dynamic_cast<gridAreaOpt *> (obj);
    if (sa != nullptr)
    {
        return remove (sa);
    }

    auto *bus = dynamic_cast<gridBusOpt *> (obj);
    if (bus != nullptr)
    {
        return remove (bus);
    }

    auto *lnk = dynamic_cast<gridLinkOpt *> (obj);
    if (lnk != nullptr)
    {
        return remove (lnk);
    }
    auto *relay = dynamic_cast<gridRelayOpt *> (obj);
    if (relay != nullptr)
    {
        return remove (relay);
    }
    throw (unrecognizedObjectException (this));
}

// TODO:: make this work like Area
void gridAreaOpt::add (gridBusOpt *bus)
{
    if (!isMember (bus))
    {
        busList.push_back (bus);
        bus->setParent (this);
        bus->locIndex = static_cast<index_t> (busList.size ()) - 1;
        optObList.insert (bus);
        objectList.push_back (bus);
    }
}

void gridAreaOpt::add (gridAreaOpt *areaOpt)
{
    if (!isMember (areaOpt))
    {
        areaList.push_back (areaOpt);
        areaOpt->setParent (this);
        areaOpt->locIndex = static_cast<index_t> (areaList.size ()) - 1;
        optObList.insert (areaOpt);
        objectList.push_back (areaOpt);
    }
}

// add link
void gridAreaOpt::add (gridLinkOpt *lnk)
{
    if (!isMember (lnk))
    {
        linkList.push_back (lnk);
        lnk->setParent (this);
        lnk->locIndex = static_cast<index_t> (linkList.size ()) - 1;
        optObList.insert (lnk);
        objectList.push_back (lnk);
    }
}

// add link
void gridAreaOpt::add (gridRelayOpt *relay)
{
    if (!isMember (relay))
    {
        relayList.push_back (relay);
        relay->setParent (this);
        relay->locIndex = static_cast<index_t> (relayList.size ()) - 1;
        optObList.insert (relay);
        objectList.push_back (relay);
    }
}

// --------------- remove components ---------------
// remove bus
void gridAreaOpt::remove (gridBusOpt *bus)
{
    if (busList[bus->locIndex]->getID () == bus->getID ())
    {
        busList[bus->locIndex]->setParent (nullptr);
        busList.erase (busList.begin () + bus->locIndex);
        for (auto kk = bus->locIndex; kk < static_cast<index_t> (busList.size ()); ++kk)
        {
            busList[kk]->locIndex = kk;
        }
        auto oLb = objectList.begin ();
        auto oLe = objectList.end ();
        auto bid = bus->getID ();
        while (oLb != oLe)
        {
            if ((*oLb)->getID () == bid)
            {
                objectList.erase (oLb);
            }
            break;
        }
    }
}

// remove link
void gridAreaOpt::remove (gridLinkOpt *lnk)
{
    if (linkList[lnk->locIndex]->getID () == lnk->getID ())
    {
        linkList[lnk->locIndex]->setParent (nullptr);
        linkList.erase (linkList.begin () + lnk->locIndex);

        for (auto kk = lnk->locIndex; kk < static_cast<index_t> (linkList.size ()); ++kk)
        {
            linkList[kk]->locIndex = kk;
        }
        auto oLb = objectList.begin ();
        auto oLe = objectList.end ();
        auto lid = lnk->getID ();
        while (oLb != oLe)
        {
            if ((*oLb)->getID () == lid)
            {
                objectList.erase (oLb);
            }
            break;
        }
    }
}

// remove area
void gridAreaOpt::remove (gridAreaOpt *areaOpt)
{
    if (areaList[areaOpt->locIndex]->getID () == areaOpt->getID ())
    {
        areaList[areaOpt->locIndex]->setParent (nullptr);
        areaList.erase (areaList.begin () + areaOpt->locIndex);
        for (auto kk = areaOpt->locIndex; kk < static_cast<index_t> (areaList.size ()); ++kk)
        {
            areaList[kk]->locIndex = kk;
        }
        auto oLb = objectList.begin ();
        auto oLe = objectList.end ();
        auto aid = areaOpt->getID ();
        while (oLb != oLe)
        {
            if ((*oLb)->getID () == aid)
            {
                objectList.erase (oLb);
            }
            break;
        }
    }
}

// remove area
void gridAreaOpt::remove (gridRelayOpt *relay)
{
    if (relayList[relay->locIndex]->getID () == relay->getID ())
    {
        relayList[relay->locIndex]->setParent (nullptr);
        relayList.erase (relayList.begin () + relay->locIndex);
        for (auto kk = relay->locIndex; kk < static_cast<index_t> (relayList.size ()); ++kk)
        {
            relayList[kk]->locIndex = kk;
        }
        auto oLb = objectList.begin ();
        auto oLe = objectList.end ();
        auto rid = relay->getID ();
        while (oLb != oLe)
        {
            if ((*oLb)->getID () == rid)
            {
                objectList.erase (oLb);
            }
            break;
        }
    }
}

void gridAreaOpt::setAll (const std::string &type,
                          const std::string &param,
                          double val,
                          gridUnits::units_t unitType)
{
    if (type == "area")
    {
        set (param, val, unitType);
        for (auto &subarea : areaList)
        {
            subarea->setAll (type, param, val, unitType);
        }
    }
    else if (type == "bus")
    {
        for (auto &bus : busList)
        {
            bus->set (param, val, unitType);
        }
    }
    else if (type == "link")
    {
        for (auto &lnk : linkList)
        {
            lnk->set (param, val, unitType);
        }
    }
    else if (type == "relay")
    {
        for (auto &rel : relayList)
        {
            rel->set (param, val, unitType);
        }
    }
    else if ((type == "gen") || (type == "load") || (type == "generator"))
    {
        for (auto &bus : busList)
        {
            bus->setAll (type, param, val, unitType);
        }
        for (auto &subarea : areaList)
        {
            subarea->setAll (type, param, val, unitType);
        }
    }
}

// set properties
void gridAreaOpt::set (const std::string &param, const std::string &val)
{
    if (param[0] == '#')
    {
    }
    else
    {
        gridOptObject::set (param, val);
    }
}

void gridAreaOpt::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "voltagetolerance") || (param == "vtol"))
    {
    }
    else if ((param == "angletolerance") || (param == "atol"))
    {
    }
    else
    {
        gridOptObject::set (param, val, unitType);
    }
}

coreObject *gridAreaOpt::find (const std::string &objName) const
{
    coreObject *obj = nullptr;
    if (objName == getName ())
    {
        return const_cast<gridAreaOpt *> (this);
    }
    for (auto ob : busList)
    {
        if (objName == ob->getName ())
        {
            obj = ob;
            break;
        }
    }
    if (obj == nullptr)
    {
        for (auto ob : areaList)
        {
            if (objName == ob->getName ())
            {
                obj = ob;
                break;
            }
        }
    }
    return obj;
}

coreObject *gridAreaOpt::getSubObject (const std::string &typeName, index_t num) const
{
    if (typeName == "link")
    {
        return getLink (num - 1);
    }
    if (typeName == "bus")
    {
        return getBus (num - 1);
    }
    if (typeName == "area")
    {
        return getArea (num - 1);
    }
    if (typeName == "relay")
    {
        return getRelay (num - 1);
    }
    return nullptr;
}

coreObject *gridAreaOpt::findByUserID (const std::string &typeName, index_t searchID) const
{
    coreObject *A1;
    if (typeName == "area")
    {
        for (auto &subarea : areaList)
        {
            if (subarea->getUserID () == searchID)
            {
                return subarea;
            }
        }
    }
    if (typeName == "bus")
    {
        for (auto &bus : busList)
        {
            if (bus->getUserID () == searchID)
            {
                return bus;
            }
        }
    }
    else if (typeName == "link")
    {
        for (auto &lnk : linkList)
        {
            if (lnk->getUserID () == searchID)
            {
                return lnk;
            }
        }
    }
    else if (typeName == "relay")
    {
        for (auto &rel : relayList)
        {
            if (rel->getUserID () == searchID)
            {
                return rel;
            }
        }
    }

    else if ((typeName == "gen") || (typeName == "load") || (typeName == "generator"))
    {
        for (auto &bus : busList)
        {
            A1 = bus->findByUserID (typeName, searchID);
            if (A1)
            {
                return A1;
            }
        }
    }
    // if we haven't found it yet search the subareas
    for (auto &subarea : areaList)
    {
        A1 = subarea->findByUserID (typeName, searchID);
        if (A1)
        {
            return A1;
        }
    }
    return nullptr;
}

// check bus members
bool gridAreaOpt::isMember (coreObject *obj) const { return optObList.isMember (obj); }

gridOptObject *gridAreaOpt::getBus (index_t index) const { return (isValidIndex (index, busList)) ? busList[index] : nullptr; }

gridOptObject *gridAreaOpt::getLink (index_t index) const
{
    return (isValidIndex (index, linkList)) ? linkList[index] : nullptr;
}

gridOptObject *gridAreaOpt::getArea (index_t index) const
{
    return (isValidIndex (index, areaList)) ? areaList[index] : nullptr;
}

gridOptObject *gridAreaOpt::getRelay (index_t index) const
{
    return (isValidIndex (index, relayList)) ? relayList[index] : nullptr;
}

double gridAreaOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
    double fval = kNullVal;
    size_t ival = kNullLocation;
    if (param == "buscount")
    {
        ival = busList.size ();
    }
    else if (param == "linkcount")
    {
        ival = linkList.size ();
    }
    else if (param == "areacount")
    {
        ival = areaList.size ();
    }
    else if (param == "relaycount")
    {
        ival = relayList.size ();
    }
    else
    {
        fval = coreObject::get (param, unitType);
    }
    return (ival != kNullLocation) ? static_cast<double> (ival) : fval;
}

gridAreaOpt *getMatchingArea (gridAreaOpt *area, gridOptObject *src, gridOptObject *sec)
{
    if (area->isRoot ())
    {
        return nullptr;
    }

    if (isSameObject (area->getParent (), src))  // if this is true then things are easy
    {
        return static_cast<gridAreaOpt *> (sec->getArea (area->locIndex));
    }

    std::vector<int> lkind;
    auto *par = dynamic_cast<gridOptObject *> (area->getParent ());
    if (par == nullptr)
    {
        return nullptr;
    }
    lkind.push_back (area->locIndex);
    while (par->getID () != src->getID ())
    {
        lkind.push_back (par->locIndex);
        par = dynamic_cast<gridOptObject *> (par->getParent ());
        if (par == nullptr)
        {
            return nullptr;
        }
    }
    // now work our way backwards through the secondary
    par = sec;
    for (auto kk = lkind.size () - 1; kk > 0; --kk)
    {
        par = static_cast<gridOptObject *> (par->getArea (lkind[kk]));
    }
    return static_cast<gridAreaOpt *> (par->getArea (lkind[0]));
}

}  // namespace griddyn
