/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTROLSYSTEM_H_
#define CONTROLSYSTEM_H_

#include "../gridSubModel.h"
#include "utilities/matrixDataSparse.hpp"
namespace griddyn {
class Block;

/** @brief class implementing a control system built from the defined control blocks*/
class controlSystem: public gridSubModel {
  protected:
    std::vector<Block*> blocks;  //!< the set of blocks to operate on
    matrixDataSparse<double> inputMult;  //!< multipliers for the input to the blocks
    matrixDataSparse<double> outputMult;  //!< multipliers for the outputs
    matrixDataSparse<double> connections;  //!< multipliers for the block inputs

    std::vector<double> blockOutputs;  //!< current vector of block outputs

  public:
    explicit controlSystem(const std::string& objName = "control_system_#");
    virtual ~controlSystem();

    virtual coreObject* clone(coreObject* obj = nullptr) const override;
    virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
    virtual void dynObjectInitializeB(const IOdata& inputs,
                                      const IOdata& desiredOutput,
                                      IOdata& fieldSet) override;

    virtual void add(coreObject* obj) override;
    virtual void add(Block* blk);

    virtual void set(const std::string& param, const std::string& val) override;
    virtual void
        set(const std::string& param, double val, units::unit unitType = units::defunit) override;
    virtual index_t findIndex(const std::string& field, const solverMode& sMode) const override;

    virtual void residual(const IOdata& inputs,
                          const stateData& sD,
                          double resid[],
                          const solverMode& sMode) override;

    virtual void jacobianElements(const IOdata& inputs,
                                  const stateData& sD,
                                  matrixData<double>& md,
                                  const IOlocs& inputLocs,
                                  const solverMode& sMode) override;

    virtual void timestep(coreTime time, const IOdata& inputs, const solverMode& sMode) override;

    virtual void rootTest(const IOdata& inputs,
                          const stateData& sD,
                          double roots[],
                          const solverMode& sMode) override;
    virtual void rootTrigger(coreTime time,
                             const IOdata& inputs,
                             const std::vector<int>& rootMask,
                             const solverMode& sMode) override;
    virtual change_code rootCheck(const IOdata& inputs,
                                  const stateData& sD,
                                  const solverMode& sMode,
                                  check_level_t level) override;

    virtual void limitTest(const IOdata& inputs,
                           const stateData& sD,
                           double limits[],
                           const solverMode& sMode) override;

    virtual void limitTrigger(double state[],
                              double dstate_dt[],
                              const std::vector<int>& limitMask,
                              const solverMode& sMode) override;

    // virtual void setTime(coreTime time){prevTime=time;};
};

}  // namespace griddyn
#endif
