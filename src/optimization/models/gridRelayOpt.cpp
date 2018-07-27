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
#include "gridRelayOpt.h"
#include "../optObjectFactory.h"
#include "core/coreExceptions.h"
#include "gridAreaOpt.h"
#include "griddyn/Relay.h"
#include "utilities/vectData.hpp"
#include "utilities/vectorOps.hpp"
#include <cmath>
#include <utility>

namespace griddyn
{
static optObjectFactory<gridRelayOpt, Relay> opRelay ("basic", "relay");

using namespace gridUnits;

gridRelayOpt::gridRelayOpt (const std::string &objName) : gridOptObject (objName) {}

gridRelayOpt::gridRelayOpt (coreObject *obj, const std::string &objName)
    : gridOptObject (objName), relay (dynamic_cast<Relay *> (obj))
{
    if (relay != nullptr)
    {
        if (getName ().empty ())
        {
            setName (relay->getName ());
        }
        setUserID (relay->getUserID ());
    }
}

coreObject *gridRelayOpt::clone (coreObject *obj) const
{
    gridRelayOpt *nobj;
    if (obj == nullptr)
    {
        nobj = new gridRelayOpt ();
    }
    else
    {
        nobj = dynamic_cast<gridRelayOpt *> (obj);
        if (nobj == nullptr)
        {
            // if we can't cast the pointer clone at the next lower level
            gridOptObject::clone (obj);
            return obj;
        }
    }
    gridOptObject::clone (nobj);

    // now clone all the loads and generators
    // cloning the links from this component would be bad
    // clone the generators and loads

    return nobj;
}

void gridRelayOpt::add (coreObject *obj)
{
    if (dynamic_cast<Relay *> (obj) != nullptr)
    {
        relay = static_cast<Relay *> (obj);
        if (getName ().empty ())
        {
            setName (relay->getName ());
        }
        setUserID (relay->getUserID ());
    }
    else
    {
        throw (unrecognizedObjectException (this));
    }
}

count_t gridRelayOpt::objSize (const optimMode & /* oMode */)
{
    count_t objs = 0;

    return objs;
}
count_t gridRelayOpt::contObjSize (const optimMode & /* oMode */)
{
    count_t objs = 0;

    return objs;
}

count_t gridRelayOpt::intObjSize (const optimMode & /* oMode */)
{
    count_t objs = 0;

    return objs;
}

count_t gridRelayOpt::constraintSize (const optimMode &oMode)
{
    count_t objs = 0;
    switch (oMode.linMode)
    {
    case linearityMode_t::linear:
    case linearityMode_t::quadratic:
    case linearityMode_t::nonlinear:
    default:
        objs = 0;
    }

    return objs;
}

void gridRelayOpt::dynObjectInitializeA (std::uint32_t /*flags*/) {}

void gridRelayOpt::remove (coreObject *) {}

void gridRelayOpt::setValues (const optimData & /* oD */, const optimMode & /* oMode */) {}
// for saving the state
void gridRelayOpt::guessState (double /*time*/, double /*val*/[], const optimMode & /* oMode */) {}

void gridRelayOpt::getVariableType (double /*sdata*/[], const optimMode & /* oMode */) {}

void gridRelayOpt::getTols (double /*tols*/[], const optimMode & /* oMode */) {}
void gridRelayOpt::valueBounds (double /*time*/,
                                double /*upperLimit*/[],
                                double /*lowerLimit*/[],
                                const optimMode & /* oMode */)
{
}

void gridRelayOpt::linearObj (const optimData & /* oD */,
                              vectData<double> & /*linObj*/,
                              const optimMode & /* oMode */)
{
}
void gridRelayOpt::quadraticObj (const optimData & /* oD */,
                                 vectData<double> & /*linObj*/,
                                 vectData<double> & /*quadObj*/,
                                 const optimMode & /* oMode */)
{
}

void gridRelayOpt::constraintValue (const optimData & /* oD */, double /*cVals*/[], const optimMode & /* oMode */)
{
}
void gridRelayOpt::constraintJacobianElements (const optimData & /* oD */,
                                               matrixData<double> & /*md*/,
                                               const optimMode & /* oMode */)
{
}

double gridRelayOpt::objValue (const optimData & /* oD */, const optimMode & /* oMode */)
{
    double cost = 0;

    return cost;
}

void gridRelayOpt::gradient (const optimData & /* oD */, double /*deriv*/[], const optimMode & /* oMode */) {}
void gridRelayOpt::jacobianElements (const optimData &, matrixData<double> & /*md*/, const optimMode & /* oMode */)
{
}
void gridRelayOpt::getConstraints (const optimData & /* oD */,
                                   matrixData<double> & /*cons*/,
                                   double /*upperLimit*/[],
                                   double /*lowerLimit*/[],
                                   const optimMode & /* oMode */)
{
}
void gridRelayOpt::getObjName (stringVec & /*objNames*/, const optimMode &, const std::string & /*prefix*/) {}

void gridRelayOpt::disable () { gridOptObject::disable (); }

void gridRelayOpt::setOffsets (const optimOffsets & /*newOffset*/, const optimMode & /*oMode*/) {}

// destructor
gridRelayOpt::~gridRelayOpt () {}

// set properties
void gridRelayOpt::set (const std::string &param, const std::string &val)
{
    if (param == "#")
    {
    }
    else
    {
        gridOptObject::set (param, val);
    }
}

void gridRelayOpt::set (const std::string &param, double val, units_t unitType)
{
    if ((param == "voltagetolerance") || (param == "vtol"))
    {
    }
    else if ((param == "angleetolerance") || (param == "atol"))
    {
    }
    else
    {
        gridOptObject::set (param, val, unitType);
    }
}

coreObject *gridRelayOpt::find (const std::string &objName) const
{
    coreObject *obj = nullptr;
    if ((objName == getName ()) || (objName == "relay"))
    {
        return const_cast<gridRelayOpt *> (this);
    }

    return obj;
}

coreObject *gridRelayOpt::getSubObject (const std::string &typeName, index_t /*num*/) const
{
    return nullptr;
    /*
    if (typeName == "target")
    {
        return nullptr;
    }

    else
    {
        return nullptr;
    }
    */
}

coreObject *gridRelayOpt::findByUserID (const std::string &typeName, index_t searchID) const
{
    if (typeName == "relay")
    {
        if (searchID == getUserID ())
        {
            return const_cast<gridRelayOpt *> (this);
        }
    }
    return nullptr;
}

double gridRelayOpt::get (const std::string &param, gridUnits::units_t unitType) const
{
    double val = kNullVal;
    if (param[0] == '#')
    {
    }
    else
    {
        val = gridOptObject::get (param, unitType);
    }
    return val;
}

}  // namespace griddyn
