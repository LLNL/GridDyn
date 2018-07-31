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

#ifndef FUSE_RELAY_H_
#define FUSE_RELAY_H_

#include "../Relay.h"
#include <cstdint>
namespace griddyn
{
namespace relays
{
/** fuse implements a standard power system fuse which can blow on time or using and I^2t calculation
 */
class fuse : public Relay
{
  public:
    /** flags for fuses*/
    enum fuse_flags
    {
        overlimit_flag = object_flag10,  //!< flag indicating the current is over the limit
        fuse_blown_flag = object_flag11,  //!< flag indicting the fuse was blown
        nonlink_source_flag = object_flag12,  //!< flag indicating the source is a not a link
    };

  protected:
    index_t m_terminal = 1;  //!< line terminal  (typically 1 or 2)
    // 4 byte gap here to use for something if need be
    parameter_t limit = kBigNum;  //!<[puA] maximum current
    parameter_t mp_I2T = 0.0;  //!<[puA^2*s] I squared t characteristic of fuse 1 in puA^2*s
    coreTime minBlowTime = 0.001;  //!<[s] the minimum time required to for the fuse to blow only used if I2T>0;
  private:
    double cI2T = 0.0;  //!< calculated I2t value for fuse
    double Vbase = 120;  //!<[kV] Voltage base for bus
    gridBus *bus = nullptr;  //!< storage for a bus which the line terminal or other object
    bool &useI2T;  //!< internal flag for using the i^2t functionality
  public:
    explicit fuse (const std::string &objName = "fuse_$");
    virtual coreObject *clone (coreObject *obj) const override;
    virtual void setFlag (const std::string &flag, bool val = true) override;
    virtual void set (const std::string &param, const std::string &val) override;

    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

    // dynamic functions for evaluation with a limit exceeded
    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    virtual void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
    virtual void converge (coreTime time,
                           double state[],
                           double dstate_dt[],
                           const solverMode &sMode,
                           converge_mode = converge_mode::high_error_only,
                           double tol = 0.01) override;
    virtual stateSizes LocalStateSizes (const solverMode &sMode) const override;

    virtual count_t LocalJacobianCount (const solverMode &sMode) const override;

    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  protected:
    virtual void conditionTriggered (index_t conditionNum, coreTime triggerTime) override;
    /** function to setup the numerical calculations associated with the fuse*/
    change_code setupFuseEvaluation ();

  private:
    /** helper calculation function to compute the i^2 t value based on a current*/
    double I2Tequation (double current);
    /** actually blow the fuse
    @return a change_code associated with the action to match a function signature*/
    change_code blowFuse ();
};
}  // namespace relays
}  // namespace griddyn
#endif
