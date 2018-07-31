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

#ifndef MOTOR_LOAD5_H_
#define MOTOR_LOAD5_H_

#include "motorLoad3.h"
namespace griddyn
{
namespace loads
{
/** @brief class implementing a model of a 3rd order induction motor
 */
class motorLoad5 : public motorLoad3
{
  private:
    /** @brief private enumerations of state variable locations in powerflow*/
    enum pLocA
    {
        irA = 0,
        imA = 1,
        slipA = 2,
        erpA = 3,
        empA = 4,
        erppA = 5,
        emppA = 6
    };
    /** @brief private enumerations of state variable locations in dynamic calculations*/
    enum pLocD
    {
        slipD = 0,
        erpD = 1,
        empD = 2,
        erppD = 3,
        emppD = 4
    };

  protected:
    double r2 = 0.002;  //!< 3rd level loop resistance
    double x2 = 0.04;  //!< 3 impedance loop reactance
    double T0pp = 0.0;  //!< subtransient time constant
    double xpp = 0.0;  //!< subtransient reactance
  public:
    /** @brief constructor
    @param[in] objName  the name of the object
    */
    explicit motorLoad5 (const std::string &objName = "motor5_$");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;

  protected:
    virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  public:
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual stateSizes LocalStateSizes (const solverMode &sMode) const override;

    virtual count_t LocalJacobianCount (const solverMode &sMode) const override;

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

    virtual void
    derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;
    virtual change_code
    rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

    // TODO:: change to algebraic update
    virtual void updateCurrents (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

  private:
    /** @brief estimate the initial state values of the motor
     */
    void converge ();
};
}  // namespace loads
}  // namespace griddyn
#endif
