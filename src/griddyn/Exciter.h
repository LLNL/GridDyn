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

#ifndef GRIDDYNEXCITER_H_
#define GRIDDYNEXCITER_H_

#include "gridSubModel.h"
namespace griddyn
{
class Generator;
/** @brief the exciter class defines the interface for power grid exciters and implements a very simple version of
 * such a device
 */

const int exciterVoltageInLocation = 0;
const int exciterVsetInLocation = 1;
const int exciterPmechInLocation = 2;
const int exciterOmegaInLocation = 3;

/** class defining the interface for an exciter as well a trivial implementation of such
 */
class Exciter : public gridSubModel
{
  public:
    enum exciter_flags
    {
        outside_vlim = object_flag3,
        etrigger_high = object_flag4,
    };

  protected:
    parameter_t Vrmin = -5.1;  //!< [pu] lower voltage limit
    parameter_t Vrmax = 6;  //!< [pu] upper voltage limit
    parameter_t Vref = 1.0;  //!< [pu] reference voltage for voltage regulator
    parameter_t Ka = 10;  //!< [pu] amplifier gain
    parameter_t Ta = 0.004;  //!< [s]    amplifier time constant
    parameter_t vBias =
      0.0;  //!< bias field level for adjusting the field output so the ref can remain at some nominal level
    int limitState = 0;  //!< indicator of which state has the limits applied
  public:
    /** @brief constructor*/
    explicit Exciter (const std::string &objName = "exciter_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual stringVec localStateNames () const override;

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual void
    derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    // handle the rootfinding functions
    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double root[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;
    virtual change_code
    rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

    // virtual void setTime(coreTime time){prevTime=time;};
    virtual const std::vector<stringVec> &inputNames () const override;
    virtual const std::vector<stringVec> &outputNames () const override;

  protected:
    void checkForLimits ();
};

}  // namespace griddyn

#endif  // GRIDDYNEXCITER_H_
