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

#ifndef GRIDDYNGENMODEL_H_
#define GRIDDYNGENMODEL_H_
#pragma once

#include "gridSubModel.h"
namespace griddyn
{
constexpr index_t genModelEftInLocation = 2;
constexpr index_t genModelPmechInLocation = 3;

class gridBus;

/** @brief model simulation implementing a generator model into GridDyn
 the GenModel implements a very basic generator model with 4 states the real and reactive
currents as algebraic states and the generator rotational speed and angle otherwise known as the second order model
*/
class GenModel : public gridSubModel
{
  public:
    /** @brief set of flags used by genModels for variations in computation
     */
    enum genModel_flags
    {

        use_saturation_flag = object_flag2,  //!< flag indicating that the simulation should use a saturation model
        use_speed_field_adjustment =
          object_flag3,  //!< flag indicating that the simulation should use a speed field adjustment
        use_frequency_impedance_correction =
          object_flag4,  //!< flag indicating that the model should use frequency impedence corrections
        internal_frequency_calculation = object_flag5,
        at_angle_limits = object_flag6,
    };

  protected:
    double machineBasePower = 100;  //!< [pu]  the operating base of the generator
    double Xd = 1.05;  //!< [pu] d-axis reactance
    double Rs = 0.0;  //!< [pu] generator resistance
    gridBus *bus = nullptr;  //!< reference to the connected bus;
  public:
    //!< @brief default constructor
    explicit GenModel (const std::string &objName = "genModel_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;

    virtual double getOutput (const IOdata &inputs,
                              const stateData &sD,
                              const solverMode &sMode,
                              index_t outNum = 0) const override;
    virtual double getOutput (index_t outNum = 0) const override;

    virtual void ioPartialDerivatives (const IOdata &inputs,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;

    virtual count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;
    // TODO:: PT split these into separate functions for getting the value and getting the offset
    virtual double getFreq (const stateData &sD, const solverMode &sMode, index_t *freqOffset = nullptr) const;
    virtual double getAngle (const stateData &sD, const solverMode &sMode, index_t *angleOffset = nullptr) const;

    virtual const std::vector<stringVec> &inputNames () const override;
    virtual const std::vector<stringVec> &outputNames () const override;
};
}  // namespace griddyn
#endif  // GRIDDYNGENMODEL_H_
