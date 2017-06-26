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

#ifndef GRIDCONTROLBLOCKS_H_
#define GRIDCONTROLBLOCKS_H_

#pragma once
#include "gridSubModel.h"

namespace griddyn
{
namespace blocks
{
class valueLimiter;
class rampLimiter;
}

/** @brief class implementing basic control system block
 the basic block class takes a single input X  the output is then \f$K*(X+bias)\f$
optionally implementing limiters Omax and Omin  the limiters have a reset level specified by resetLevel
once the object is initialized the determination of whether to use the ramps is fixed and cannot be changed
unless the object is reinitialized directly

the blocks take 1 or 2 inputs the first being the single input,  if the differential input is set then the second argument is the time derivative of the input
*/
class Block : public gridSubModel
{

public:
  /** @brief flags common for all control blocks
  */
  enum controller_flags
  {
	  step_only = object_flag1,				//!< flag indicating that the block does not have any state
    use_state = object_flag2,                   //!< flag indicating that the basic block should not control the state before the limiters
    has_limits = object_armed_flag,             //!< flag indicating the block has limits of some kind
    use_block_limits = object_flag3,            //!< flag indicating the block has upper and lower limits
    use_ramp_limits = object_flag4,             //!< flag indicating the block has ramp limits

    differential_input = object_flag5,      //!< flag indicating that the input is a differential state
	use_direct = object_flag6,				//!< flag indicating that the block should just use the input directly
    simplified = object_flag7,                         //!< flag indicating that the block should revert to basic block behavior [used only by derived object]
    anti_windup_limits = object_flag8,         //!< flag indicating that the limits should be anti-windup [used only by derived objects]
  };
protected:
  parameter_t K = 1.0;                               //!<  gain
  parameter_t Omax = kBigNum;                //!< max output value
  parameter_t Omin = -kBigNum;               //!< min output value
  parameter_t rampMax = kBigNum;             //!< rate of change max value
  parameter_t rampMin = -kBigNum;    //!< rate of change min value
  parameter_t bias = 0.0;                    //!< bias
  parameter_t resetLevel = -0.001;   //!< the level below or above the max/min that the limiters should be removed
  double prevInput = 0.0;  //!< variable to hold previous input values;
  count_t limiter_alg = 0;          //!< the number of algebraic states used by the limiters
  count_t limiter_diff = 0;         //!< the number of differential states used by the limiters
  std::string outputName = "output";  //!< the name of the output state
  std::unique_ptr<blocks::valueLimiter> vLimiter;  //!< a pointer to an object that handles the value limits
  std::unique_ptr<blocks::rampLimiter> rLimiter;  //!< a pointer to an object that handles the ramp limits
public:
  /** @brief default constructor*/
  explicit Block (const std::string &objName = "block_#");
  /** @brief alternate constructor
  @param[in] gain  the desired gain of the block
  */
  Block (double gain, const std::string &objName = "block_#");

  virtual ~Block();  //included for separation of types in unique Pointers
  virtual coreObject * clone (coreObject *obj = nullptr) const override;

protected:
  virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
public:
  virtual void setFlag (const std::string &flag, bool val) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual double get(const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  //virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

  /** @brief simplifying function in place of residual since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] resid the location to store the Jacobian elements
  @param[in] sMode the solverMode that corresponds to the state data
  */
  virtual void residElements (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode);

  
  virtual void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

  /** @brief simplifying function in place of derivative call since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] deriv the location to store the derivative elements
  @param[in] sMode the solverMode that corresponds to the state data
  @return the output
  */
  virtual void derivElements (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode);
  virtual void derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;

  /** @brief simplifying function in place of algebraicUpdate call since block have only one input/output
  @param[in] input  the block input
  @param[in] sD the state data
  @param[out] deriv the location to store the derivative elements
  @param[in] sMode the solverMode that corresponds to the state data
  @return the output
  */
  virtual void algElements (double input, const stateData &sD, double update[], const solverMode &sMode);
  virtual void algebraicUpdate (const IOdata &inputs, const stateData &sD, double update[], const solverMode &sMode, double alpha) override;

  /** @brief simplifying function in place of Jacobian elements since block have only one input/output
  @param[in] input  the block input
  @param[in] didt the input derivative used if the differential input flag is set and it is needed otherwise ignored
  @param[in] sD the state data
  @param[out] md the location to store the Jacobian elements
  @param[in] argLoc the index location of the input
  @param[in] sMode the solverMode that corresponds to the state data
  */
  virtual void jacElements (double input, double didt, const stateData &sD, matrixData<double> &md, index_t argLoc, const solverMode &sMode);

  virtual void jacobianElements (const IOdata &inputs, const stateData &sD,
                                 matrixData<double> &md,
                                 const IOlocs &inputLocs, const solverMode &sMode) override;

  virtual void timestep  (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
  /** @brief simplifying function in place of timestep since block have only one input/output
  @param[in] time  the time to step to
  @param[in] input  the input argument
  @return the output
  */
  virtual double step (coreTime time, double input);
  virtual void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (coreTime time, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;
  //virtual void setTime(coreTime time){prevTime=time;};
  virtual stringVec localStateNames () const override;
  virtual double getBlockOutput (const stateData &sD, const solverMode &sMode) const;
  virtual double getBlockOutput () const;
  virtual double getBlockDoutDt(const stateData &sD, const solverMode &sMode) const;
  virtual double getBlockDoutDt() const;
  const std::string &getOutputName() const
  {
	  return outputName;
  }
protected:
  void limiterResidElements(double input, double didt, const stateData &sD, double resid[], const solverMode &sMode);
  double getRateInput(const IOdata &inputs) const;
private:
	  double getTestValue(double input, double currentState) const;
	  double getTestRate(double didt, double currentStateRate) const;
	  bool hasValueState() const;
	  void valLimiterUpdate();
	  void rampLimiterUpdate();
	  double computeDefaultResetLevel();
	  double getLimiterTestValue(double input, const stateData &sD, const solverMode &sMode);
	  
};


/** @brief generate a shared pointer to a block based on a string input
@param[in] blockstr  a string defining a block
@return a unique pointer to a block
*/
std::unique_ptr<Block> make_block (const std::string &blockstr);


}//namespace griddyn

#endif //GRIDCONTROLBLOCKS_H_