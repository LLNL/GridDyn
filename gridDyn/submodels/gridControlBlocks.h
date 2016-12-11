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

#ifndef GRIDCONTROLBLOCKS_H_
#define GRIDCONTROLBLOCKS_H_

#include "gridObjects.h"
/** @brief class implementing basic control system block
 the basic block class takes a single input X  the output is then \f$K*(X+bias)\f$
optionally implementing limiters Omax and Omin  the limiters have a reset level specified by resetLevel
once the object is initialized the determination of whether to use the ramps is fixed and cannot be changed
unless the object is reinitialized directly

the blocks take 1 or 2 inputs the first being the single input,  if the differential input is set then the second argument is the time derivative of the input
*/
class basicBlock : public gridSubModel
{

public:
  /** @brief flags common for all control blocks
  */
  enum controller_flags
  {
    use_state = object_flag1,                   //!< flag indicating that the basic block should not control the state before the limiters
    has_limits = object_armed_flag,             //!< flag indicating the block has limits of some kind
    use_block_limits = object_flag2,            //!< flag indicating the block has upper and lower limits
    use_ramp_limits = object_flag3,             //!< flag indicating the block has ramp limits
    outside_lim = object_flag4,  //!< flag indicating that the value is outside the limit
    trigger_high = object_flag5,  //!< flag indicating the limit has triggered on the high side
    ramp_limit_triggered = object_flag6,    //!< flag indicating the ramp limit is triggered
    ramp_limit_triggered_high = object_flag7,    //!< flag indicating the ramp limit triggered on the high side

    differential_input = object_flag8,      //!< flag indicating that the input is a differential state
    simplified = object_flag9,                         //!< flag indicating that the block should revert to basic block behavior [used only by derived object]
    anti_windup_limits = object_flag10,         //!< flag indicating that the limits should be anti-windup [used only by derived objects]
  };
protected:
  double K = 1.0;                               //!<  gain
  double Omax = kBigNum;                //!< max output value
  double Omin = -kBigNum;               //!< min output value
  double rampMax = kBigNum;             //!< rate of change max value
  double rampMin = -kBigNum;    //!< rate of change min value
  double bias = 0.0;                    //!< bias
  double resetLevel = -0.001;   //!< the level below or above the max/min that the limiters should be removed
  double prevInput = 0.0;  //!< variable to hold previous input values;
  int limiter_alg = 0;          //!< the number of alg states used by the limiters
  int limiter_diff = 0;         //!< the number of diff states used by the limiters
public:
  /** @brief default constructor*/
  explicit basicBlock (const std::string &objName = "block_#");
  /** @brief alternate constructor
  @param[in] gain  the desired gain of the block
  */
  basicBlock (double gain, const std::string &objName = "block_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
protected:
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
  virtual void setFlag (const std::string &flag, bool val) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  //virtual void derivative(const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode);

  /** @brief simplifying function in place of residual since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] resid the location to store the Jacobian elements
  @param[in] sMode the solverMode that corresponds to the state data
  */
  virtual void residElements (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode);

  virtual void residual (const IOdata &args, const stateData &sD, double resid[], const solverMode &sMode) override;

  /** @brief simplifying function in place of derivative call since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] deriv the location to store the derivative elements
  @param[in] sMode the solverMode that corresponds to the state data
  @return the output
  */
  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode);
  virtual void derivative (const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode) override;

  /** @brief simplifying function in place of algebraicUpdate call since block have only one input/output
  @param[in] input  the block input
  @param[in] sD the state data
  @param[out] deriv the location to store the derivative elements
  @param[in] sMode the solverMode that corresponds to the state data
  @return the output
  */
  virtual void algElements (double input, const stateData &sD, double deriv[], const solverMode &sMode);
  virtual void algebraicUpdate (const IOdata &args, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;

  /** @brief simplifying function in place of Jacobian elements since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] ad the location to store the Jacobian elements
  @param[in] argLoc the index location of the input
  @param[in] sMode the solverMode that corresponds to the state data
  */
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode);

  virtual void jacobianElements (const IOdata &args, const stateData &sD,
                                 matrixData<double> &ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void timestep  (gridDyn_time ttime, const IOdata &args, const solverMode &sMode) override;
  /** @brief simplifying function in place of timestep since block have only one input/output
  @param[in] time  the time to step to
  @param[in] input  the input argument
  @return the output
  */
  virtual double step (gridDyn_time time, double input);
  virtual void rootTest (const IOdata &args, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &args, const stateData &sD, const solverMode &sMode, check_level_t level) override;
  //virtual void setTime(gridDyn_time time){prevTime=time;};
  virtual stringVec localStateNames () const override;
  virtual double getBlockOutput (const stateData &sD, const solverMode &sMode);
  virtual double getBlockOutput ();
};

/** @brief class implementing an integral block
 computes the integral of the input
*/
class integralBlock : public basicBlock
{

public:
protected:
  double iv = 0.0;  //!< the initial value(current value) of the integral
public:
  //!< default constructor
  explicit integralBlock (const std::string &objName = "integralBlock_#");
  /** alternate constructor to add in the gain
  @param[in] gain  the multiplication factor of the block
  */
  integralBlock (double gain, const std::string &objName = "integralBlock_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;

  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void residElements (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (gridDyn_time time, double input) override;
  // virtual void timestep(gridDyn_time ttime, const IOdata &args, const solverMode &sMode);
  //virtual void setTime(gridDyn_time time){prevTime=time;};
};

/** @brief class implementing a delay block
 block implementing \f$H(S)=\frac{K}{1+T_1 s}\f$
if the time constant is very small it reverts to the basic block
*/
class delayBlock : public basicBlock
{

public:
protected:
  double m_T1 = 0.1;  //!< the time constant
public:
  //!< default constructor
  explicit delayBlock (const std::string &objName = "delayBlock_#");
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant
  @param[in] objName the name of the block
  */
  delayBlock (double t1, const std::string &objName = "delayBlock_#");
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant
  @param[in] gain the block gain
  @param[in] objName the name of the object
  */
  delayBlock (double t1, double gain, const std::string &objName = "delayBlock_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
protected:
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (gridDyn_time time, double input) override;
  //virtual void setTime(gridDyn_time time){prevTime=time;};
};


/** @brief class implementing a derivative
 block implementing \f$H(S)=\frac{K s}{1+T_1 s}\f$
if the time constant is very small it reverts to the basic block
*/
class derivativeBlock : public basicBlock
{
protected:
  double m_T1 = 0.1; //!< delay time constant for the derivative filtering operation
public:
  //!< default constructor
  explicit derivativeBlock (const std::string &objName = "derivBlock_#");
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant
  */
  derivativeBlock (double t1, const std::string &objName = "derivBlock_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
protected:
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;
public:
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void algElements (double input, const stateData &sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (gridDyn_time time, double input) override;

  virtual stringVec localStateNames () const override;
  //virtual void setTime(gridDyn_time time){prevTime=time;};
};

/** @brief class implementing a control block
 block implementing \f$H(S)=\frac{K(1+T_2 s}{1+T_1 s}\f$
default is \f$T_2 =0\f$ for behavior equivalent to a delay block
if T1 is 0 it behaves like the basic block
*/
class controlBlock : public basicBlock
{

public:
protected:
  double m_T1 = 0.1;  //!< delay time constant
  double m_T2 = 0.0;  //!< upper time constant
public:
  //!< default constructor
  explicit controlBlock (const std::string &objName = "controlBlock_#");
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant
  @param[in] objName the name of the block
  */
  controlBlock (double t1, const std::string &objName = "controlBlock_#");  //convert to the equivalent of a delay block with t2=0;
  /** alternate constructor to add in the time constant
  @param[in] t1  the time constant
  @param[in] t2 the upper time constant
  @param[in] objName the name of the block
  */
  controlBlock (double t1, double t2, const std::string &objName = "controlBlock_#");  //convert to the equivalent of a delay block with t2=0;
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void algElements (double input, const stateData &sD, double deriv[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (gridDyn_time time, double input) override;
  //virtual void setTime(gridDyn_time time){prevTime=time;};
  virtual stringVec localStateNames () const override;
};

/** @brief class implementing a deadband system
 TOBE added
*/
class deadbandBlock : public basicBlock
{

public:
  /** @brief flags for the deadband block*/
  enum deadbandblock_flags
  {
    uses_deadband = object_flag10,  //!< flag indicating the deadband is in use
    uses_shiftedoutput = object_flag11,  //!< flag indicating the output should shift for continuity
    dbtrigger_high = object_flag12,   //!< flag indicating the deadband has been triggered on the high side
  };
  /** states for the deadband block*/
  enum class deadbandstate_t
  {
    normal, rampup, outside, rampdown, shifted
  };
protected:
  double deadbandHigh = -kBigNum;  //!< upper limit on the deadband
  double deadbandLow = kBigNum;   //!< lower deadband limit
  double rampUpband = 0;                //!< ramp band on the up side
  double rampDownband = 0;              //!< ramp band on the low side
  double resetHigh = -kBigNum;  //!< the reset level to go off the deadband
  double resetLow = kBigNum;            //!< the reset level to go back in the deadband on the low side
  double deadbandLevel = 0.0;           //!<  the output level while the input is inside the deadband
  deadbandstate_t dbstate = deadbandstate_t::normal;  //!< the current state of the deadband block

public:
  /** @brief the default constructor*/
  explicit deadbandBlock (const std::string &objName = "deadband_#");
  /** @brief alternate constructor with a deadband argument
  @param[in] db the size of the deadband
  */
  deadbandBlock (double db, const std::string &objName = "deadband_#");
  virtual coreObject * clone (coreObject *obj = nullptr) const override;
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual void setFlag (const std::string &flag, bool val) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  //virtual void derivative(const IOdata &args, const stateData &sD, double deriv[], const solverMode &sMode);
  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void algElements (double input, const stateData &sD, double deriv[], const solverMode &sMode) override;

  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &ad, index_t argLoc, const solverMode &sMode) override;
  virtual double step (gridDyn_time time, double input) override;
  virtual void rootTest (const IOdata &args, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (gridDyn_time ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &args, const stateData &sD, const solverMode &sMode, check_level_t level) override;
  /** @brief get the deadband state
  @return the state of the deadband block
  */
  deadbandstate_t getDBState ()
  {
    return dbstate;
  }
  /** @brief get the output of the deadband portion {not including the gain and limiters
  @param[in] the input value
  @return the computed output value
  */
  double computeValue (double input);
  /** @brief compute the partial derivative of the output with respect to the input
  @param[in] the input value
  @return the computed derivative
  */
  double computeDoutDin (double input);
  //virtual void setTime(gridDyn_time time){prevTime=time;};
};


/** @brief generate a shared pointer to a block based on a string input
@param[in] blockstr  a string defining a block
@return a shared pointer to a block
*/
std::shared_ptr<basicBlock> make_block (const std::string &blockstr);
#endif //GRIDCONTROLBLOCKS_H_
