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

#ifndef BLOCK_SOURCE_H_
#define BLOCK_SOURCE_H_

#include "../Source.h"

namespace griddyn
{
class Block;
namespace sources
{
/** define a source object that contains another source which is followed by a Block object
 */
class blockSource : public Source
{
  private:
    Source *src = nullptr;  //!< pointer to the source object
    Block *blk = nullptr;  //!< pointer to the Block object
    double maxStepSize = kBigNum;  //!< calculation for the maximum step size that should be taken in a timestep
  public:
    blockSource (const std::string &objName = "blocksource_#");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void add (coreObject *obj) override;
    virtual void remove (coreObject *obj) override;

  protected:
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  public:
    virtual void setFlag (const std::string &flag, bool val) override;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, units::unit unitType = units::defunit) override;
    virtual double get (const std::string &param, units::unit unitType = units::defunit) const override;

    // virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode);

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

    virtual void
    derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;

    virtual void algebraicUpdate (const IOdata &inputs,
                                  const stateData &sD,
                                  double update[],
                                  const solverMode &sMode,
                                  double alpha) override;

    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;

    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;
    virtual change_code
    rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

    virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

    virtual IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double
    getOutput (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

    virtual double getOutput (index_t outputNum = 0) const override;

    virtual double
    getDoutdt (const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

    virtual void setLevel (double newLevel) override;

    virtual coreObject *find (const std::string &object) const override;
    virtual coreObject *getSubObject (const std::string &typeName, index_t num) const override;
};
}  // namespace sources
}  // namespace griddyn

#endif
