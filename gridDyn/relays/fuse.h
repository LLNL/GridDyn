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

#ifndef FUSE_RELAY_H_
#define FUSE_RELAY_H_

#include "gridRelay.h"
#include <cstdint>

/** fuse implements a standard power system fuse which can blow on time or using and I^2t calculation
*/
class fuse : public gridRelay
{
public:
	/** flags for fuses*/
  enum fuse_flags
  {
    overlimit_flag = object_flag10,   //!< flag indicating the current is over the limit
    fuse_blown_flag = object_flag11,	//!< flag indicting the fuse was blown
    nonlink_source_flag = object_flag12,	//!< flag indicating the source is a not a link
  };
protected:
  index_t m_terminal = 1;  //!< line terminal  (typically 1 or 2)
  double limit = kBigNum;         //!<[puA] maximum current
  double mp_I2T = 0.0;         //!<[puA^2*s] I squared t characteristic of fuse 1 in puA^2*s
  double minBlowTime = 0.001;   //!<[s] the minimum time required to for the fuse to blow only used if I2T>0;
private:
  double cI2T;        //!< calculated I2t value for fuse
  double Vbase = 120;       //!<[kV] Voltage base for bus
  gridBus *bus = nullptr;		//!< storage for a bus which the line terminal or other object
  bool useI2T = false;  //!< internal flag for using the i^2t functionality
public:
  explicit fuse (const std::string &objName = "fuse_$");
  virtual gridCoreObject * clone (gridCoreObject *obj) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual void set (const std::string &param,  const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;

  //dynamic functions for evaluation with a limit exceeded
  virtual void timestep (double ttime, const solverMode &sMode) override;
  virtual void jacobianElements (const stateData *sD, matrixData<double> *ad, const solverMode &sMode) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;
  virtual void converge (double ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode = converge_mode::high_error_only, double tol = 0.01) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;
protected:
	virtual void conditionTriggered (index_t conditionNum, double triggerTime) override;
  change_code setupFuseEvaluation ();
private:
  double I2Tequation (double current);
  change_code blowFuse ();
};

#endif
