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

#ifndef BREAKER_RELAY_H_
#define BREAKER_RELAY_H_

#include "gridRelay.h"

/** relay implementing a overcurrent breaker for a transmission line
*/
class breaker : public gridRelay
{
public:
  enum breaker_flags
  {
    nondirectional_flag = object_flag8,
    overlimit_flag = object_flag9,
    breaker_tripped_flag = object_flag10,
    nonlink_source_flag = object_flag11,
  };
protected:
  double minClearingTime = 0.0;   //!<[s] minimum clearing time for from bus breaker
  double recloseTime1 = 1;      //!<[s] first reclose time
  double recloseTime2 = 5;    //!<[s] second reclose time
  double recloserTap = 0;       //!< From side tap multiplier
  double limit = 1.0;         //!<[puA] maximum current in puA
  double lastRecloseTime = -kBigNum;     //!<[s] last reclose time
  double recloserResetTime = 60;    //!<[s] time the breaker has to be on before the recloser count resets
  count_t maxRecloseAttempts = 0;        //!< total number of recloses
  index_t m_terminal = 1;       //!< link terminal
  gridBus *bus = nullptr;
private:
  count_t recloseAttempts = 0;        //!< reclose attempt counter1
  double cTI = 0;			//!< storage for the current integral
  double Vbase = 120;       //!< Voltage base for bus1
  bool useCTI = false;		//!< internal flag to use the CTI stuff
public:
  breaker (const std::string &objName = "breaker_$");
  virtual gridCoreObject * clone (gridCoreObject *obj) const override;
  virtual int setFlag (const std::string &flag, bool val = true) override;
  virtual int set (const std::string &param,  const std::string &val) override;

  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
  virtual void updateA (double time) override;

  //dynamic state functions
  virtual double timestep (double ttime, const solverMode &sMode) override;
  virtual void jacobianElements (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

protected:
  virtual void conditionTriggered (index_t conditionNum, double triggeredTime) override;
  /** trip the breaker
  @param[in] time current time
  */
  void tripBreaker (double time);
  /** reset the breaker
  @param[in] time current time
  */
  void resetBreaker (double time);

};

#endif
