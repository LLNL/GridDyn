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

#include "core/propertyBuffer.h"
#include "../fmi_import/fmiInfo.h"
#include "fmiSupport.h"
#include "griddyn/gridSubModel.h"
#include <map>

class fmi2ModelExchangeObject;

enum class fmuMode;  // forward declare enumeration

namespace griddyn
{
namespace fmi
{
class outputEstimator;
/** class defining a subModel interacting with an FMU v2.0 object for model exchange*/
class fmiMESubModel : public gridSubModel
{
  public:
    enum fmiSubModelFlags
    {
        use_output_estimator = object_flag2,
        fixed_output_interval = object_flag3,
        reprobe_flag = object_flag4,
        has_derivative_function = object_flag5,
    };

  protected:
    count_t m_stateSize = 0;  //!< the total state count
    count_t m_jacElements = 0;  //!< the number of Jacobian elements
    count_t m_eventCount = 0;  //!< the number of event indicators
    std::shared_ptr<fmi2ModelExchangeObject> me;

    std::vector<outputEstimator *>
      oEst;  //!< vector of objects used for output estimation //TODO:: Make this an actual vector of objects
    coreTime localIntegrationTime = 0.01;
    fmuMode prevFmiState = fmuMode::instantiatedMode;
    std::vector<vInfo> stateInformation;
    std::vector<vInfo> outputInformation;
    std::vector<int> inputVarIndices;
    propertyBuffer paramBuffer;

  private:
    count_t lastSeqID = 0;
    std::vector<double> tempState;
    std::vector<double> tempdState;

  public:
    fmiMESubModel (const std::string &newName = "fmisubmodel2_#",
                   std::shared_ptr<fmi2ModelExchangeObject> fmi = nullptr);

    fmiMESubModel (std::shared_ptr<fmi2ModelExchangeObject> fmi = nullptr);
    virtual ~fmiMESubModel ();
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

  protected:
    virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void pFlowObjectInitializeB () override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  public:
    virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
    virtual stringVec getOutputNames () const;
    virtual stringVec getInputNames () const;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
    virtual stateSizes LocalStateSizes (const solverMode &sMode) const override;

    virtual count_t LocalJacobianCount (const solverMode &sMode) const override;

    virtual std::pair<count_t, count_t> LocalRootCount (const solverMode & /* sMode */) const override;
    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual void
    derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
    virtual void ioPartialDerivatives (const IOdata &inputs,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;
    virtual void outputPartialDerivatives (const IOdata &inputs,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;

    IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double getDoutdt (const IOdata &inputs,
                              const stateData &sD,
                              const solverMode &sMode,
                              index_t outputNum = 0) const override;
    virtual double getOutput (const IOdata &inputs,
                              const stateData &sD,
                              const solverMode &sMode,
                              index_t outputNum = 0) const override;

    virtual double getOutput (index_t outputNum = 0) const override;
    virtual index_t getOutputLoc (const solverMode &sMode, index_t outputNum = 0) const override;

    virtual void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
    // for saving the state
    virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;

    virtual void getTols (double tols[], const solverMode &sMode) override;

    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;

    virtual bool isLoaded () const;

    virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

  protected:
    void makeSettableState ();
    void resetState ();
    double getPartial (int depIndex, int refIndex, refMode_t mode);
    void probeFMU ();
    void loadOutputJac (int index = -1);
    // int searchByRef(fmi2_value_reference_t ref);
};

}  // namespace fmi
}  // namespace griddyn
