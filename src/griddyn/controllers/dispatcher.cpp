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

#include "dispatcher.h"
#include "core/coreExceptions.h"

namespace griddyn
{
dispatcher::dispatcher (const std::string &objName) : coreObject (objName) {}

dispatcher::~dispatcher () {}
coreObject *dispatcher::clone (coreObject * /*obj*/) const { return nullptr; }

void dispatcher::moveSchedulers (dispatcher * /*dis*/) {}
double dispatcher::initialize (coreTime /*time0*/, double /*dispatch*/) { return 0; }

double dispatcher::updateP (coreTime /*time*/, double /*required*/, double /*targetTime*/) { return 0; }
double dispatcher::testP (coreTime /*time*/, double /*required*/, double /*targetTime*/) { return 0; }

void dispatcher::add (coreObject * /*obj*/) { throw (objectAddFailure (this)); }
void dispatcher::add (scheduler * /*sched*/) { throw (objectAddFailure (this)); }
void dispatcher::remove (coreObject * /*obj*/) {}
void dispatcher::remove (scheduler * /*sched*/) {}

void dispatcher::set (const std::string &param, const std::string &val) { return coreObject::set (param, val); }
void dispatcher::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    return coreObject::set (param, val, unitType);
}

void dispatcher::checkGen () {}

void dispatcher::dispatch (double /*level*/) {}

}  // namespace griddyn