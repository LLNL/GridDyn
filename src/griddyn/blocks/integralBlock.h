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

#ifndef INTEGRAL_BLOCK_H_
#define INTEGRAL_BLOCK_H_
#pragma once

#include "Block.h"

namespace griddyn
{
namespace blocks
{
/** @brief class implementing an integral block
 computes the integral of the input
*/
class integralBlock : public Block
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

  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  //virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

  virtual void blockDerivative (double input, double didt, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void blockResidual (double input, double didt, const stateData &sD, double resid[], const solverMode &sMode) override;
  //only called if the genModel is not present
  virtual void blockJacobianElements (double input, double didt, const stateData &sD, matrixData<double> &md, index_t argLoc, const solverMode &sMode) override;
  virtual double step (coreTime time, double inputA) override;
  // virtual void timestep(coreTime time, const IOdata &inputs, const solverMode &sMode);
  //virtual void setTime(coreTime time){prevTime=time;};
};
}//namespace blocks
}//namespace griddyn

#endif //INTEGRAL_BLOCK_H_

