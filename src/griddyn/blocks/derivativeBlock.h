/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef DERIVATIVE_BLOCK_H_
#define DERIVATIVE_BLOCK_H_
#pragma once
#include "../Block.h"

namespace griddyn
{
namespace blocks
{
/** @brief class implementing a derivative
 block implementing \f$H(S)=\frac{K s}{1+T_1 s}\f$
if the time constant is very small it reverts to the basic block
*/
class derivativeBlock : public Block
{
  protected:
    parameter_t m_T1 = 0.1;  //!< delay time constant for the derivative filtering operation
  public:
    //!< default constructor
    explicit derivativeBlock (const std::string &objName = "derivBlock_#");
    /** alternate constructor to add in the time constant
    @param[in] t1  the time constant
    */
    derivativeBlock (double t1, const std::string &objName = "derivBlock_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

  protected:
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  public:
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    // virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

    virtual void blockDerivative (double input,
                                  double didt,
                                  const stateData &sD,
                                  double deriv[],
                                  const solverMode &sMode) override;
    virtual void
    blockAlgebraicUpdate (double input, const stateData &sD, double update[], const solverMode &sMode) override;
    // only called if the genModel is not present
    virtual void blockJacobianElements (double input,
                                        double didt,
                                        const stateData &sD,
                                        matrixData<double> &md,
                                        index_t argLoc,
                                        const solverMode &sMode) override;
    virtual double step (coreTime time, double inputA) override;

    virtual stringVec localStateNames () const override;
    // virtual void setTime(coreTime time){prevTime=time;};
};
}  // namespace blocks
}  // namespace griddyn

#endif  // DERIVATIVE_BLOCK_H_