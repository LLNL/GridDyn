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

#ifndef BUSCONTROLS_H_
#define BUSCONTROLS_H_

#include "basicDefs.h"
#include "gridDynTypes.h"
#include <vector>

class gridSecondary;
class acBus;
class gridBus;
class gridLink;
class gridObject;

/** @brief a set of  controls for a bus that manages controllable generators and loads
 provides autogen functionality and manages controlled generators to help with the transition from power flow to dynamic calculations
also manages the direct connected buses and buses tied together by perfect links
*/
class busControls
{
public:
  acBus *controlledBus;         //!< the bus that is being controlled

  double Qmin = -kBigNum;        //!< [p.u.]    reactive power minimum
  double Qmax = kBigNum;         //!< [p.u.]    reactive power maximum
  double Pmin = -kBigNum;        //!< [p.u.]    real power maximum
  double Pmax = kBigNum;        //!< [p.u.]    real power maximum
  double autogenP = kBigNum;       //!< use an automatic generator to local match P load
  double autogenQ = kBigNum;       //!<use an automatic generator to locally match Q load
  double autogenDelay = 0.0;        //!<time constant for automatic generation
  double autogenPact = 0;       //!< use an automatic generator to local match P load
  double autogenQact = 0;       //!<use an automatic generator to locally match Q load

  //for managing voltage control objects
  std::vector<gridSecondary *> vControlObjects;        //!< object which control the voltage of the bus
  std::vector<gridLink *> proxyVControlObject;        //!< object which act as an interface for remote objects acting on a bus
  std::vector<gridLink *> vControlLinks;          //!< set of Link which themselves act as controllable objects;
  std::vector<double> vcfrac;        //!< the fraction of control power which should be allocated to a specific object

  std::vector<double> vclinkFrac;       //!< the fraction of control power which should be allocated to a specific controllable link

  //for managing p control objects
  std::vector<gridSecondary *> pControlObjects;      //!< object which control the angle of a bus
  std::vector<gridLink *> proxyPControlObject;       //!< object which act as an interface for remote objects acting on a bus
  std::vector<gridLink *> pControlLinks;          //!< set of Link which themselves act as controllable power objects;
  std::vector<double> pcfrac;       //!< the fraction of control power which should be allocated to a specific object
  std::vector<double> pclinkFrac;       //!< the fraction of control power which should be allocated to a specific controllable link

  //for coordinating node-breaker models and directly connected buses
  std::vector<acBus *> slaveBusses;        //!< buses which are slaved to this bus
  gridBus *masterBus = nullptr;         //!< if the bus is a slave this is the master
  gridBus *directBus = nullptr;         //!< if the bus is direct connected this is the master

public:
  /** @brief const*/
  busControls ();

  bool hasVoltageAdjustments (index_t sid) const;
  bool hasPowerAdjustments (index_t sid) const;

  double getAdjustableCapacityUp (double time) const;
  double getAdjustableCapacityDown (double time) const;

  void addPowerControlObject (gridObject * obj,bool update);
  void addVoltageControlObject (gridObject *obj, bool update);

  void removePowerControlObject (index_t oid, bool update);
  void removeVoltageControlObject (index_t oid, bool update);

  /** @brief  update the values used in voltage control*/
  void updateVoltageControls ();
  /** @brief  update the values used in power control*/
  void updatePowerControls ();

  bool checkIdenticalControls ();

  void mergeBus (acBus *mbus);
  void unmergeBus (acBus *mbus);
  void checkMerge ();
};


#endif
