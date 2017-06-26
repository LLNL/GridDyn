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

#ifndef PID_BLOCK_H_
#define PID_BLOCK_H_
#pragma once

#include "Block.h"

namespace griddyn
{
namespace blocks
{
/** @brief class implementing a PID controller
the derivative operator has a prefilter operation on it with a time constant T1 and the output has a delay of Td*/
class pidBlock : public Block
{
  public:
  protected:
    double m_P = 1;  //!< proportional control constant
    double m_I = 0;  //!< integral control constant
    double m_D = 0;  //!< differential control constant
    double m_T1 = 0.01;  //!< filtering delay on the input for the differential calculation
    double m_Td = 0.01;  //!< filtering delay on the output
    double iv = 0;  //!< intermediate value for calculations
    bool no_D = true;  //!< ignore the derivative part of the calculations
  public:
    /** @brief default constructor*/
    explicit pidBlock (const std::string &objName = "pidBlock_#");
    /** @brief alternate constructor
    @param[in] P the proportional gain
    @param[in] I the integral gain
    @param[in] D the derivative gain
    */
    pidBlock (double P, double I, double D, const std::string &objName = "pidBlock_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

    virtual void derivElements (double input,
                                double didt,
                                const stateData &sD,
                                double deriv[],
                                const solverMode &sMode) override;
    // only called if the genModel is not present
    virtual void jacElements (double input,
                              double didt,
                              const stateData &sD,
                              matrixData<double> &md,
                              index_t argLoc,
                              const solverMode &sMode) override;
    virtual double step (coreTime time, double inputA) override;
    virtual stringVec localStateNames () const override;
};

}//namespace blocks
}//namespace griddyn

#endif  // PID_BLOCK_H_