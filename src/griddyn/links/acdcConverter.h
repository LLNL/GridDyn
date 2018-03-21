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

#ifndef GRID_ACDCCONVERTER_H_
#define GRID_ACDCCONVERTER_H_

#include "../Link.h"
#include "core/coreOwningPtr.hpp"

namespace griddyn
{
namespace blocks
{
class pidBlock;
class delayBlock;
}

namespace links
{
/** class defines an object that converts operation between dc and ac, can act as a inverter, a rectifier or a
 * bidirectional mode
 */
class acdcConverter : public Link
{
  public:
    enum inverter_flags
    {
        fixed_power_control = object_flag6,
    };
    enum class mode_t
    {
        rectifier,
        inverter,
        bidirectional
    };

  protected:
    enum class control_mode_t
    {
        current,
        power,
        voltage
    };
    parameter_t r = 0.0;  //!< [puOhm] per unit resistance
    parameter_t x = 0.001;  //!< [puOhm] per unit reactance
    parameter_t tap = 1.0;  //!< converter tap
    double angle = 0.0;  //!< converter firing or extinction angle
    parameter_t Idcmax = kBigNum;  //!<[puA] max reference current
    parameter_t Idcmin = -kBigNum;  //!<[puA] min reference current
    parameter_t mp_Ki = 0.03;  //!<integral gain angle control
    parameter_t mp_Kp = 0.97;  //!<proportional gain angle control
    double Idc = 0.0;  //!< storage for dc current
    mode_t type = mode_t::bidirectional;  //!< converter type
    parameter_t vTarget = 1.0;  //!< [puV] ac voltage target
    parameter_t mp_controlKi = -0.03;  //!<integral gain angle control
    parameter_t mp_controlKp = -0.97;  //!<proportional gain angle control
    parameter_t tD = 0.01;  //!< controller time delay
    parameter_t baseTap = 1.0;  //!< base l evel tap of the converter
    double dirMult = 1.0;
    parameter_t minAngle = -kPI / 2.0;  //!< [rad] minimum tap angle
    parameter_t maxAngle = kPI / 2.0;  //!< [rad]  maximum tap angle
    control_mode_t control_mode = control_mode_t::voltage;

    coreOwningPtr<blocks::pidBlock> firingAngleControl;  //!<block controlling firing angle
    coreOwningPtr<blocks::pidBlock> powerLevelControl;  //!<block controlling power
    coreOwningPtr<blocks::delayBlock> controlDelay;  //!<delayblock for control of tap

  public:
    explicit acdcConverter (const std::string &objName = "acdcConveter_$");
    // name will be based on opType
    acdcConverter (mode_t opType, const std::string &objName = "");
    acdcConverter (double rP, double xP, const std::string &objName = "acdcConveter_$");

    virtual ~acdcConverter ();
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual double getMaxTransfer () const override;

    // virtual void pFlowCheck (std::vector<violation> &Violation_vector);
    // virtual void getVariableType (double sdata[], const solverMode &sMode);      //has no state variables
    virtual void updateBus (gridBus *bus, index_t busnumber) override;

    virtual void updateLocalCache () override;
    virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
    virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

    virtual double quickupdateP () override { return 0; }

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    // dynInitializeB dynamics
    // virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags);

	using Link::ioPartialDerivatives;
    virtual void ioPartialDerivatives (id_type_t busId,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;

    virtual void outputPartialDerivatives (const IOdata &inputs,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;

    virtual void outputPartialDerivatives (id_type_t busId,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
    virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
    // for computing all the Jacobian elements at once
    virtual int fixRealPower (double power,
                              id_type_t terminal,
                              id_type_t fixedTerminal = 0,
                              gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual int fixPower (double rPower,
                          double qPower,
                          id_type_t measureTerminal,
                          id_type_t fixedTerminal = 0,
                          gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;

  private:
    /** build out the components of the converter*/
    void buildSubsystem ();
};

}  // namespace links
}  // namespace griddyn
#endif