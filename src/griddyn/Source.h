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
#pragma once
#include "gridSubModel.h"

namespace griddyn {
/** Source is a signal generator in GridDyn.
The component Definition class defines the interface for a Source
*/
class Source: public gridSubModel {
  public:
    std::string purpose_;  //!< string for use by applications to indicate usage
  protected:
    double m_tempOut = 0;  //!< temporary output corresponding to desired time
    coreTime lastTime = timeZero;  //!< storage for the previously queried time
    units::unit outputUnits_ = units::defunit;  //!< specify the units of the output
  public:
    /** constructor
    @param[in] objname the name of the object
    @param[in] startVal the starting Value of the object
    */
    Source(const std::string& objName = "source_#", double startVal = 0.0);
    virtual coreObject* clone(coreObject* obj = nullptr) const override;

    virtual void set(const std::string& param, const std::string& val) override;
    virtual void
        set(const std::string& param, double val, units::unit unitType = units::defunit) override;

    virtual void timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override;

    virtual IOdata getOutputs(const IOdata& inputs,
                              const stateData& sD,
                              const solverMode& sMode) const override;
    virtual double getOutput(const IOdata& inputs,
                             const stateData& sD,
                             const solverMode& sMode,
                             index_t num = 0) const override;

    virtual double getOutput(index_t outputNum = 0) const override;
    virtual index_t getOutputLoc(const solverMode& sMode, index_t num = 0) const override;

    virtual units::unit outputUnits(index_t outputNum) const override;

    virtual count_t outputDependencyCount(index_t num, const solverMode& sMode) const override;
    virtual void setState(coreTime time,
                          const double state[],
                          const double dstate_dt[],
                          const solverMode& sMode) override;
    /** update the output to correspond to a new time value*/
    virtual void updateOutput(coreTime time);
    virtual void updateLocalCache(const IOdata& inputs,
                                  const stateData& sD,
                                  const solverMode& sMode) override;

    /** update the source output and advance the model time
    @param[in] time  the time to update to
    */
    virtual double computeOutput(coreTime time) const;
    /** set the output level
    @param[in] newLevel the level to set the output at
    */
    virtual void setLevel(double newLevel);
};

}  // namespace griddyn
