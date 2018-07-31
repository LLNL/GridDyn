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

#ifndef DCBUS_H_
#define DCBUS_H_

// headers
#include "../gridBus.h"
#include "DcBusControls.h"
#include "utilities/matrixDataTranslate.hpp"

namespace griddyn
{
// forward classes

/** @brief class implements a DC powered bus
 the DC has only one state (voltage) and can only attach to links that are DC capable
*/
class dcBus : public gridBus
{
    friend class DcBusControls;

  public:
    enum bus_flags
    {
        use_autogen = object_flag2,  //!< indicator if the bus is using an autogen
        slave_bus = object_flag3,  //!< indicator that the bus is a slave Bus
        master_bus = object_flag4,  //!< indicator that a bus is a master bus
        directconnect = object_flag5,  //!< indicator that a bus is direct connected to another bus
    };

  protected:
    double vTarget = 1.0;  //!< a target voltage
    double participation = 1.0;
    DcBusControls busController;  //!< pointer to the busController object
    busType prevType = busType::PQ;  //!< previous type container if the type automatically changes
    dynBusType prevDynType = dynBusType::normal;  //!< previous type container if the type automatically changes
    double dVdP = 0.0;  //!< storage for the dVdP terms from all the secondary objects
    matrixDataTranslate<1> od;

  public:
    explicit dcBus (const std::string &objName = "dcBus_$");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    // add components
    using gridBus::add;
    virtual void add (Link *lnk) override;  // this add function checks for DC capable links

    // dynInitializeB

    virtual stateSizes LocalStateSizes (const solverMode &sMode) const override;

    virtual count_t LocalJacobianCount (const solverMode &sMode) const override;

  protected:
    virtual void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void pFlowObjectInitializeB () override;

  public:
    virtual change_code powerFlowAdjust (const IOdata &inputs,
                                         std::uint32_t flags,
                                         check_level_t level) override;  // only applicable in pFlow
    // virtual  void generationAdjust(double adjustment);
    virtual void pFlowCheck (std::vector<violation> &Violation_vector) override;
    // dynInitializeB dynamics
  protected:
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;

  public:
    // parameter set functions
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
    virtual void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;

    void computeDerivatives (const stateData &sD, const solverMode &sMode);

    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual void converge (coreTime time,
                           double state[],
                           double dstate_dt[],
                           const solverMode &sMode,
                           converge_mode mode = converge_mode::local_iteration,
                           double tol = 0.01) override;

    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

    virtual double getVoltage (const double state[], const solverMode &sMode) const override;

    virtual double getVoltage (const stateData &sD, const solverMode &sMode) const override;

    virtual IOlocs getOutputLocs (const solverMode &sMode) const override;

    virtual index_t getOutputLoc (const solverMode &sMode, index_t num = 0) const override;

    virtual bool useVoltage (const solverMode &sMode) const;
    virtual int getMode (const solverMode &sMode) const;
    virtual int propogatePower (bool makeSlack = false) override;

    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  protected:
    void computePowerAdjustments ();
};

}  // namespace griddyn
#endif
