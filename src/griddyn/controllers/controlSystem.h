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

#ifndef CONTROLSYSTEM_H_
#define CONTROLSYSTEM_H_

#include "../gridSubModel.h"

#include "utilities/matrixDataSparse.hpp"
namespace griddyn
{
class Block;

/** @brief class implementing a control system built from the defined control blocks*/
class controlSystem : public gridSubModel
{
  protected:
    std::vector<Block *> blocks;  //!< the set of blocks to operate on
    matrixDataSparse<double> inputMult;  //!< multipliers for the input to the blocks
    matrixDataSparse<double> outputMult;  //!< multipliers for the outputs
    matrixDataSparse<double> connections;  //!< multipliers for the block inputs

    std::vector<double> blockOutputs;  //!< current vector of block outputs

  public:
    explicit controlSystem (const std::string &objName = "control_system_#");
    virtual ~controlSystem ();

    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void add (coreObject *obj) override;
    virtual void add (Block *blk);

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;

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
    // virtual void setTime(coreTime time){prevTime=time;};
};

}  // namespace griddyn
#endif
