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

#ifndef GOVERNOR_IEEE_SIMPLE_H_
#define GOVERNOR_IEEE_SIMPLE_H_
#pragma once

#include "GovernorIeeeSimple.h"

namespace griddyn
{
namespace governors
{
class GovernorSteamNR : public GovernorIeeeSimple
{
  public:
  protected:
    parameter_t Tch;  //!< [s] steam reheat chest time constant
  public:
    GovernorSteamNR (const std::string &objName = "govSteamNR_#");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual ~GovernorSteamNR ();
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    // only called if the genModel is not present
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
};

}  // namespace governors
}  // namespace griddyn

#endif  // GOVERNOR_IEEE_SIMPLE_H_
