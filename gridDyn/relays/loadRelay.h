/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef LOAD_RELAY_H_
#define LOAD_RELAY_H_

#include "gridRelay.h"

class loadRelay : public gridRelay
{
public:
  enum busrelay_flags
  {
    nondirectional_flag = object_flag10,
  };
protected:
  double cutoutVoltage = 0;
  double cutoutFrequency = 0;
  double voltageDelay = 0;
  double frequencyDelay = 0;
  double offTime = kBigNum;
public:
  loadRelay (const std::string &objName = "loadRelay_$");
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual int setFlag (const std::string &flag, bool val = true) override;
  virtual int set (const std::string &param,  const std::string &val) override;

  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
protected:
  void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, double actionTime) override;
  void conditionTriggered (index_t conditionNum, double triggerTime) override;
  void conditionCleared (index_t conditionNum, double triggerTime) override;

};


#endif