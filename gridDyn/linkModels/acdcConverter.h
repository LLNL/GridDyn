/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
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

#ifndef GRID_DC_INVERTER_H_
#define GRID_DC_INVERTER_H_

#include "linkModels/gridLink.h"

class pidBlock;
class delayBlock;

/** class defines an object that converts operation between dc and ac, can act as a inverter, a rectifier or a bidirectional mode
*/
class acdcConverter : public gridLink
{
public:
  enum inverter_flags
  {
    fixed_power_control = object_flag6,
  };
  enum class mode_t
  {
    rectifier, inverter, bidirectional
  };

protected:
  enum class control_mode_t
  {
    current, power, voltage
  };
  double r = 0.0;							//!< [puOhm] per unit resistance
  double x = 0.001;							//!< [puOhm] per unit reactance
  double tap = 1.0;							//!< converter tap
  double angle = 0.0;					//!< converter firing or extinction angle
  double Idcmax = kBigNum;                //!<[puA] max rectifier reference current
  double Idcmin = -kBigNum;                //!<[puA] min rectifier reference current
  double mp_Ki = 0.03;                    //!<integral gain angle control
  double mp_Kp = 0.97;                         //!<proportional gain angle control
  double Idc;								//!< storage for dc current
  mode_t type = mode_t::bidirectional;			//!< converter type
  double vTarget = 1.0;							//!< [puV] ac voltage target
  double mp_controlKi = -0.03;                    //!<integral gain angle control
  double mp_controlKp = -0.97;                     //!<proportional gain angle control
  double tD = 0.01;								//!< controller time delay
  double baseTap = 1.0;							//!< base l evel tap of the converter
  double dirMult = 1.0;							
  double minAngle = -kPI / 2.0;					//!< [rad] minimum tap angle
  double maxAngle = kPI / 2.0;					//!< [rad]  maximum tap angle
  control_mode_t control_mode = control_mode_t::voltage;
  //TODO:: CHANGE this to be direct instantiation?
  std::shared_ptr<pidBlock> firingAngleControl;   //!<block controlling firing angle
  std::shared_ptr<pidBlock> powerLevelControl;   //!<block controlling power
  std::shared_ptr<delayBlock> controlDelay;   //!<delayblock for control of tap


public:
  explicit acdcConverter (const std::string &objName = "acdcConveter_$");
  //name will be based on opType
  acdcConverter (mode_t opType, const std::string &objName = "");
  acdcConverter (double rP, double xP, const std::string &objName = "acdcConveter_$");

  virtual ~acdcConverter ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;


  virtual double getMaxTransfer () const override;

  //virtual void pFlowCheck (std::vector<violation> &Violation_vector);
  //virtual void getVariableType (double sdata[], const solverMode &sMode);      //has no state variables
  virtual void updateBus (gridBus *bus, index_t busnumber) override;

  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode) override;
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeB (IOdata &outputSet) override;

  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void timestep (double ttime, const solverMode &sMode) override;

  virtual double quickupdateP () override
  {
    return 0;
  }

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  //initializeB dynamics
  //virtual void dynObjectInitializeA (double time0, unsigned long flags);

  virtual void ioPartialDerivatives (index_t busId, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives  (index_t busId, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;

  virtual void jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;
  //for computing all the Jacobian elements at once
  virtual int fixRealPower (double power, index_t  terminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual int fixPower (double rPower, double qPower, index_t  mterminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
private:
	/** build out the components of the converter*/
  void buildSubsystem ();
};


#endif