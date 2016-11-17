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

#ifndef GRIDSVD_H_
#define GRIDSVD_H_

#include "loadModels/otherLoads.h"
/** @brief defining the interface for a static var device*/
class svd : public gridRampLoad
{
public:
  enum svd_flags
  {
    continuous_flag = object_flag6,
    locked_flag = object_flag7,
    reactive_control_flag = object_flag8,
  };

protected:
  gridBus *controlBus = nullptr;    //!< pointer to the control bus
  double Qmin = -kBigNum;                       //!<[puMVA] the minimum reactive power
  double Qmax = kBigNum;                        //!<[puMVA] the maximum reactive power output
  double Vmin = 0.8;                            //!<[puV] the low voltage threshold
  double Vmax = 1.2;                            //!<[puV] the high voltage threshold

  double Qlow = 0;                              //!<[puMVA] the lowest available Q block level
  double Qhigh = kBigNum;                       //!<[puMVA] the maximum reactive power block level
  int currentStep = 0;                          //!< the current step level
  int stepCount = 0;                            //!< the total number of steps available
  std::vector < std::pair < int, double >> Cblocks;     // a vector containing the capacitive blocks (count, size[puMW])

  double participation = 1.0;    //!< a participation factor

public:
  svd (const std::string &objName = "svd_$");
  svd (double rP, double rQ, const std::string &objName = "svd_$");
  virtual ~svd ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time, unsigned long flags) override;

  virtual void dynObjectInitializeB (const IOdata &args, const IOdata &outputSet) override;
  virtual void setLoad (double level, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void setLoad (double Plevel, double Qlevel, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;        //for saving the state
  virtual void guess (double ttime, double state[], double dstate_dt[], const solverMode &sMode) override;                //for initial setting of the state
  //for identifying which variables are algebraic vs differential
  virtual void getVariableType (double sdata[], const solverMode &sMode) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual void setControlBus (gridBus *cBus);

  /** add a reactive block to the controller
  @param[in] steps the number of steps in the block
  @param[in] Qstep  the size of each step 
  @param[in] unitType  the units of Qstep
  */
  void addBlock (int steps, double Qstep, gridUnits::units_t unitType = gridUnits::defUnit);

  virtual change_code powerFlowAdjust (const IOdata &args, unsigned long flags, check_level_t level) override;
  virtual void reset (reset_levels level = reset_levels::minimal) override;

  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;

  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;

  virtual void outputPartialDerivatives (const IOdata &args, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;

  virtual void jacobianElements (const IOdata &args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void getStateName  (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  virtual void timestep (double ttime, const IOdata &args, const solverMode &sMode) override;

  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
protected:
	/** get the setting corresponding to a specfic output level
	@param[in] level the reactive output level desired [puMW] 
	@return the step number corresponding to that level (best effort)
	*/
  virtual int checkSetting (double level);
  /** change the output setting to correspond to a specific step number
  @param step the step number for the update
  */
  virtual void updateSetting (int step);
};


#endif