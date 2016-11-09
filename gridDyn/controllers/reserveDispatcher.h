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

#ifndef RESERVEDISPATCHER_H_
#define RESERVEDISPATCHER_H_

#include "gridCore.h"
#include "vectorOps.hpp"

#include <vector>

class gridArea;
class schedulerRamp;

/** in development object to manage the dispatch of reserve generation
*/
class reserveDispatcher : public gridCoreObject
{
public:
protected:
  double thresholdStart = kBigNum;
  double thresholdStop = kBigNum;
  double currDispatch = 0;
  double reserveAvailable = 0;
  double dispatchTime = -kBigNum;
  double dispatchInterval = 60.0 * 5.0;

  count_t schedCount;
  std::vector<schedulerRamp *> schedList;
  std::vector<double> resAvailable;
  std::vector<double> resUsed;

public:
  reserveDispatcher (const std::string &objName = "reserveDispatch_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual ~reserveDispatcher ();


  virtual double initializeA (double time0,double dispatch);

  void moveSchedulers (reserveDispatcher *rD);
  void setTime (double time) override;
  virtual double updateP (double time,double pShort);
  virtual double testP (double time,double pShort);
  double getOutput (index_t /*num*/ = 0)
  {
    return currDispatch;
  }

  virtual void add (schedulerRamp *sched);
  virtual void add (gridCoreObject *obj) override;

  virtual void remove (schedulerRamp *sched);
  virtual void remove (gridCoreObject *obj) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  double getAvailable ()
  {
    return reserveAvailable;
  }

  virtual void schedChange ();
protected:
  virtual void checkGen ();
  virtual void dispatch (double level);
};


#endif
