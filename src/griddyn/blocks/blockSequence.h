/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BLOCK_SEQUENCE_H_
#define BLOCK_SEQUENCE_H_
#pragma once
#include "../Block.h"

namespace griddyn {
namespace blocks {
    /** @brief class implementing a sequence of blocks as a single block
A block is defined as a single input single output subModel.  This object takes any number of blocks
in a sequence and processes them in the appropriate fashion.
*/
    class blockSequence: public Block {
      public:
      protected:
        enum sequence_flags {
            differential_inputActual =
                object_flag11,  //!< flag specifying that the outer input is differential
        };
        std::vector<Block*> blocks;  //!< the building blocks in the sequence
        std::vector<int> sequence;  //!< a numerical ordering of the sequence
      private:
        std::vector<double> blockOutputs;  //!< temporary storage for block outputs
        std::vector<double> blockDoutDt;  //!< temporary storage for block ramp outputs
        index_t seqID = 0;  //!< sequence ID for last update
        // NOTE:: 4 byte gap here
        double sampleTime = kBigNum;  //!< the minimum local sampling time for local integration
      public:
        /** @brief default constructor*/
        explicit blockSequence(const std::string& objName = "block_#");

        virtual coreObject* clone(coreObject* obj = nullptr) const override;

      protected:
        virtual void dynObjectInitializeA(coreTime time0, std::uint32_t flags) override;
        virtual void dynObjectInitializeB(const IOdata& inputs,
                                          const IOdata& desiredOutput,
                                          IOdata& fieldSet) override;

      public:
        virtual void add(coreObject* obj) override;
        virtual void add(Block* blk);

        virtual void setFlag(const std::string& flag, bool val) override;
        virtual void set(const std::string& param, const std::string& val) override;
        virtual void set(const std::string& param,
                         double val,
                         units::unit unitType = units::defunit) override;

        virtual void blockResidual(double input,
                                   double didt,
                                   const stateData& sD,
                                   double resid[],
                                   const solverMode& sMode) override;

        virtual void blockDerivative(double input,
                                     double didt,
                                     const stateData& sD,
                                     double deriv[],
                                     const solverMode& sMode) override;

        virtual void blockAlgebraicUpdate(double input,
                                          const stateData& sD,
                                          double update[],
                                          const solverMode& sMode) override;

        virtual void blockJacobianElements(double input,
                                           double didt,
                                           const stateData& sD,
                                           matrixData<double>& md,
                                           index_t argLoc,
                                           const solverMode& sMode) override;

        virtual double step(coreTime time, double input) override;
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
        // virtual void setTime(coreTime time){prevTime=time;};
        /** get the output of one of the component blocks
    *@param[in] blockNum the index of the block to the get the output of
    @return the output value of the requested block
    */
        double subBlockOutput(index_t blockNum) const;
        /** get the output of one of the component blocks by name
    *@param[in] blockname the name of the block to the get the output of
    @return the output value of the requested block
    */
        double subBlockOutput(const std::string& blockname) const;

        virtual void updateLocalCache(const IOdata& inputs,
                                      const stateData& sD,
                                      const solverMode& sMode) override;

        virtual coreObject* getSubObject(const std::string& typeName, index_t num) const override;

        virtual coreObject* findByUserID(const std::string& typeName,
                                         index_t searchID) const override;
    };
}  // namespace blocks
}  // namespace griddyn
#endif
