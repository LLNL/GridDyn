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

#ifndef DEADBANDBLOCK_H_
#define DEADBANDBLOCK_H_
#pragma once
#include "../Block.h"

namespace griddyn
{
namespace blocks
{
/** @brief class implementing a deadband system
 TOBE added
*/
class deadbandBlock : public Block
{
  public:
    /** @brief flags for the deadband block*/
    enum deadbandblock_flags
    {
        uses_deadband = object_flag10,  //!< flag indicating the deadband is in use
        uses_shiftedoutput = object_flag11,  //!< flag indicating the output should shift for continuity
        dbtrigger_high = object_flag12,  //!< flag indicating the deadband has been triggered on the high side
    };
    /** states for the deadband block*/
    enum class deadbandstate_t
    {
        normal,
        rampup,
        outside,
        rampdown,
        shifted
    };

  protected:
    parameter_t deadbandHigh = -kBigNum;  //!< upper limit on the deadband
    parameter_t deadbandLow = kBigNum;  //!< lower deadband limit
    parameter_t rampUpband = 0;  //!< ramp band on the up side
    parameter_t rampDownband = 0;  //!< ramp band on the low side
    parameter_t resetHigh = -kBigNum;  //!< the reset level to go off the deadband
    parameter_t resetLow = kBigNum;  //!< the reset level to go back in the deadband on the low side
    parameter_t deadbandLevel = 0.0;  //!<  the output level while the input is inside the deadband
    parameter_t tolerance = 1e-6;  //!< the tolerance for resetting on the check function
    deadbandstate_t dbstate = deadbandstate_t::normal;  //!< the current state of the deadband block

  public:
    /** @brief the default constructor*/
    explicit deadbandBlock (const std::string &objName = "deadband_#");
    /** @brief alternate constructor with a deadband argument
    @param[in] db the size of the deadband
    */
    deadbandBlock (double db, const std::string &objName = "deadband_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void setFlag (const std::string &flag, bool val) override;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    // virtual index_t findIndex(const std::string &field, const solverMode &sMode) const;

    // virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);
    virtual void blockDerivative (double input,
                                  double didt,
                                  const stateData &sD,
                                  double deriv[],
                                  const solverMode &sMode) override;
    virtual void
    blockAlgebraicUpdate (double input, const stateData &sD, double update[], const solverMode &sMode) override;

    virtual void blockJacobianElements (double input,
                                        double didt,
                                        const stateData &sD,
                                        matrixData<double> &md,
                                        index_t argLoc,
                                        const solverMode &sMode) override;
    virtual double step (coreTime time, double input) override;
    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;
    virtual change_code
    rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;
    /** @brief get the deadband state
    @return the state of the deadband block
    */
    deadbandstate_t getDBState () const { return dbstate; }
    /** @brief get the output of the deadband portion {not including the gain and limiters
    @param[in] the input value
    @return the computed output value
    */
    double computeValue (double input) const;
    /** @brief compute the partial derivative of the output with respect to the input
    @param[in] the input value
    @return the computed derivative
    */
    double computeDoutDin (double input) const;
    // virtual void setTime(coreTime time){prevTime=time;};
};

}  // namespace blocks
}  // namespace griddyn
#endif  // DEADBANDBLOCK_H_