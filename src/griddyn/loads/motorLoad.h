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

#ifndef MOTOR_LOAD_H_
#define MOTOR_LOAD_H_

#include "Load.h"
namespace griddyn
{
namespace loads
{
/** @brief class defining operations and equations for an induction motor load
*/
class motorLoad : public Load
{
public:
  /** @brief motor load flags*/
  enum motor_load_flags
  {
    init_transient = object_flag8, //!< flag indicating that the motor starts with a load transient
    stalled = object_flag9,   //!< flag indicating that the motor is stalled
    resettable = object_flag10,  //!< flag indicating that the motor can be reset once stalled
  };

protected:
	parameter_t Pmot = -kBigNum;	//!< the mechanical power of the motor
	parameter_t r = 0.01;	//!< the motor resistance
	parameter_t x = 0.15;  //!< the motor impedance
	parameter_t r1 = 0.05;  //!< primary resistance on the motor
	parameter_t x1 = 0.15;  //!< primary inductance of the motor
	parameter_t xm = 5.0;      //!< the inductive load of the motor
	parameter_t H = 3.0;  //!< the inertial constant on the motor
	parameter_t alpha = 1.0;  //!< alpha parameter for torque conversion
	parameter_t beta = 0;  //!< beta parameter for torque conversion
	parameter_t gamma = 0; //!< gamma parameter for torque conversion
	parameter_t a = 1.0;  //!< a parameter for torque conversion
	parameter_t b = 0;  //!< b parameter for torque conversion
	parameter_t c = 0;  //!< c parameter for torque conversion
	parameter_t mBase = -100;  //!< system machine base
	parameter_t Vcontrol = 1.0;  //!< whether the motor has some voltage controls for tweaking power (basically a transformer attached motor
	parameter_t init_slip = -1.0;  //!< the initial slip of the motor
	parameter_t scale = 1.0;  //!< scaling factor for the motor
public:
  /** @brief constructor
  @param[in] objName  the name of the object
  */
  explicit motorLoad (const std::string &objName = "motor_$");


  virtual coreObject * clone (coreObject *obj = nullptr) const override;
protected:
  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;        //for saving the state
  virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;     //return D[0]=dP/dV D[1]=dP/dtheta,D[2]=dQ/dV,D[3]=dQ/dtheta

  virtual void outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
  virtual count_t outputDependencyCount(index_t num, const solverMode &sMode) const override;
  virtual void ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void jacobianElements  (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  virtual void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

  /** @brief compute the mechanical torque on the motor as a function of slip
  @param[in]  slip  the slip on the motor
  @return the mechanical output power
  */
  double mechPower (double slip) const;
  /** @brief compute the partial derivative of the torque with respect to the slip
  @param[in]  slip  the slip on the motor
  @return dTorque/dslip
  */
  double dmechds (double slip) const;

  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

  virtual double getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;
protected:
  /** @brief compute the slip based on an elecrical load
  @param[in] Pout  the electrical load of the motor
  @return the computed slip
  */
  double computeSlip (double Ptarget) const;

  /** @brief compute the real load of the motor based on voltage and slip
  @param[in] Vin  the the motor terminal voltage
  @param[in] slip the slip of the motor
  @return the computed real electrical load
  */
  double rPower (double vin, double slip) const;

  /** @brief compute the reactive load of the motor based on voltage and slip
  @param[in] Vin  the the motor terminal voltage
  @param[in] slip the slip of the motor
  @return the computed reactive electrical load
  */
  double qPower (double vin, double slip) const;
};

/** @brief class implementing a model of a 3rd order induction motor
*/
class motorLoad3 : public motorLoad
{
protected:
  parameter_t xp = 0.0;  //!< transient reactance of the motor
  parameter_t T0p = 0.0; //!< transient time constant of the motor
  parameter_t x0 = 0.0;  //!< x0 parameter
  
  //double theta=0;
public:
  /** @brief constructor
  @param[in] objName  the name of the object
  */
  motorLoad3 (const std::string &objName = "motor3_$");


  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata & desiredOutput, IOdata &fieldSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;        //for saving the state
  virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;       //return D[0]=dP/dV D[1]=dP/dtheta,D[2]=dQ/dV,D[3]=dQ/dtheta
  virtual void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

  virtual void outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
  virtual count_t outputDependencyCount(index_t num, const solverMode &sMode) const override;

  virtual void ioPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void jacobianElements  (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

  virtual double getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getRealPower (double V) const override;
  virtual double getReactivePower (double V) const override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;
  virtual void updateCurrents (const IOdata &inputs, const stateData &sD, const solverMode &sMode);
private:
  /** @brief estimate the initial state values of the motor
  */
  void converge ();
};

/** @brief class implementing a model of a 3rd order induction motor
*/
class motorLoad5 : public motorLoad3
{
private:
  /** @brief private enumerations of state variable locations in powerflow*/
  enum pLocA
  {
    irA = 0,imA = 1,slipA = 2,erpA = 3,empA = 4,erppA = 5,emppA = 6
  };
  /** @brief private enumerations of state variable locations in dynamic calculations*/
  enum pLocD
  {
    slipD = 0, erpD = 1, empD = 2, erppD = 3, emppD = 4
  };
protected:
  parameter_t r2 = 0.002;  //!< 3rd level loop resistance
  parameter_t x2 = 0.04;  //!< 3 impedance loop reactance
  parameter_t T0pp = 0.0;  //!< subtransient time constant
  parameter_t xpp = 0.0;  //!< subtransient reactance
public:
  /** @brief constructor
  @param[in] objName  the name of the object
  */
  explicit motorLoad5 (const std::string &objName = "motor5_$");


  virtual coreObject * clone (coreObject *obj = nullptr) const override;
protected:
  virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;


  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

  virtual void jacobianElements (const IOdata &inputs, const stateData &sD, matrixData<double> &md, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void getStateName  (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

  //TODO:: change to algebraic update
  virtual void updateCurrents (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
private:
  /** @brief estimate the initial state values of the motor
  */
  void converge ();
};
}//namespace loads
}//namespace griddyn
#endif
