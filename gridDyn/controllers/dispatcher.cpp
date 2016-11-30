/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/



#include "dispatcher.h"
#include "core/gridDynExceptions.h"


dispatcher::dispatcher (const std::string &objName) : gridCoreObject (objName)
{
}

dispatcher::~dispatcher ()
{

}
gridCoreObject * dispatcher::clone (gridCoreObject * /*obj*/) const
{
  return nullptr;
}

void dispatcher::moveSchedulers (dispatcher * /*dis*/)
{
}
double dispatcher::initialize (gridDyn_time /*time0*/, double /*dispatch*/)
{
  return 0;
}

void dispatcher::setTime (gridDyn_time /*time*/)
{
}

double dispatcher::updateP (gridDyn_time /*time*/, double /*required*/, double /*targetTime*/)
{
  return 0;
}
double dispatcher::testP (gridDyn_time /*time*/, double /*required*/, double /*targetTime*/)
{
  return 0;
}


void dispatcher::add (gridCoreObject * /*obj*/)
{
	throw(objectAddFailure(this));
}
void dispatcher::add (scheduler * /*sched*/)
{
	throw(objectAddFailure(this));
}
void dispatcher::remove (gridCoreObject * /*obj*/)
{

}
void dispatcher::remove (scheduler * /*sched*/)
{

}

void dispatcher::set (const std::string &param, const std::string &val)
{
  return gridCoreObject::set (param, val);
}
void dispatcher::set (const std::string &param, double val, gridUnits::units_t unitType)
{
  return gridCoreObject::set (param, val, unitType);
}

void dispatcher::checkGen ()
{
}

void dispatcher::dispatch (double /*level*/)
{
}

