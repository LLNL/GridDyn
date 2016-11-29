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
#ifndef ZONAL_RELAY_H_
#define ZONAL_RELAY_H_

#include "gridRelay.h"
/** class building off of gridRelay to define a zonal relays
 the number of zones is arbitrary and it works by checking the impedances of the associated link and comparing to specific thresholds.
This zonal relays runs off a single impedance number
*/
class zonalRelay : public gridRelay
{
public:
  enum zonalrelay_flags
  {
    nondirectional_flag = object_flag10,
  };
protected:
  count_t m_zones = 2;       //!< the number of zones for the relay
  index_t m_terminal = 1;        //!< the side of the line to connect 1=from side 2=to side, 3+ for multiterminal devices
  double m_resetMargin = 0.01; //!<! the reset margin for clearing a fault
  std::vector<double> m_zoneLevels;        //!< the level of impedance to trigger
  std::vector<double> m_zoneDelays;       //!< the delay upon which to act for the relay
  count_t m_condition_level = kInvalidCount;    //!< the level of condition that has been triggered
  int autoName = -1;                                    //!< storage for indicator of the type of autoname to use
public:
  explicit zonalRelay (const std::string &objName = "zonalRelay_$");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
protected:
  virtual void actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, gridDyn_time actionTime) override;
  virtual void conditionTriggered (index_t conditionNum, gridDyn_time triggerTime) override;
  virtual void conditionCleared (index_t conditionNum, gridDyn_time triggerTime) override;
  virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
  /** function to automatically generate the comm system names
  @param[in] code  a code value representing the method of generating the name
  @return the generated name
  */
  std::string generateAutoName (int code);
};



#endif