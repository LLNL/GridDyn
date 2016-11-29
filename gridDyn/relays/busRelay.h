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

#ifndef BUS_RELAY_H_
#define BUS_RELAY_H_

#include "gridRelay.h"
/** relay object for bus protection can isolate a bus based on voltage or frequency
with a controllable delay time
*/
class busRelay : public gridRelay
{
public:
  enum busrelay_flags
  {
    nondirectional_flag = object_flag10,
  };
protected:
  double cutoutVoltage = 0.0;		//!<[puV] low voltage limit
  double cutoutFrequency = 0.0;		//!<[puHz] trip on low frequency
  gridDyn_time voltageDelay = timeZero;		//!< [s] period of time the voltage must be below limit to activate
  gridDyn_time frequencyDelay = timeZero;		//!< [s] period of time the frequency must be below limit to activate
public:
  explicit busRelay (const std::string &objName = "busrelay_$");
  virtual gridCoreObject * clone (gridCoreObject *obj) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, gridDyn_time actionTime) override;

};


#endif