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
#include "../Generator.h"

namespace griddyn
{
class GenModel;
class Exciter;
class Stabilizer;
class Governor;
class isocController;
class Source;
/**
@ brief class describing a generator intended for dynamic simulations
 a generator is a power production unit in GridDyn.  the base generator class implements methods set forth in the
gridSecondary class and inherits from that class it has mechanics and interfaces for handling any and all of 4
different components, namely and exciter, governor, generator model, and a power system stabilizer. as well as
control for the power set point and voltage set point
*/
class DynamicGenerator : public Generator
{
  public:
    enum class dynModel_t
    {
        invalid,
        simple,
        dc,
        transient,
        detailed,
        model_only,
        typical,
        subtransient,
        renewable,
        none,
    };

    /** @brief enum indicating subModel locations in the subObject structure*/
    enum submodel_locations
    {
        genmodel_loc = 1,
        exciter_loc = 2,
        governor_loc = 3,
        pss_loc = 4,
        pset_loc = 5,
        vset_loc = 6,
        isoc_control = 7,
    };

  protected:
    GenModel *genModel = nullptr;  //!< generator model
    Exciter *ext = nullptr;  //!< exciter model
    Governor *gov = nullptr;  //!< governor model
    Stabilizer *pss = nullptr;  //!< power system stabilizer type
    Source *pSetControl = nullptr;  //!< source for throttle control
    Source *vSetControl = nullptr;  //!< source for voltage level control
    isocController *isoc = nullptr;  //!< pointer to a isochronous controller
    // const double *m_stateTemp = nullptr;                       //!< temporary state vector(assumed not writable)
    // const double *m_dstate_dt_Temp = nullptr;                   //!<  a temporary deriv vector;

    // std::vector<double> m_state_ind;                              //!< a vector holding the indices to indicate
    // if a variable is a algebraic or differential state count_t SSize = 0; //!< the total number of states
    double m_Eft = 0;  //!< place to store a constant the exciter field
    double m_Pmech = 0;  //!< place to store a constant power output
  public:
    static dynModel_t dynModelFromString (const std::string &dynModelType);
    /** @brief default constructor
    @param[in] dynModel  a string with the dynmodel description*/
    DynamicGenerator (dynModel_t dynModel, const std::string &objName = "gen_$");
    explicit DynamicGenerator (const std::string &objName = "gen_$");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

    virtual void
    dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &fieldSet) override;
    virtual void setState (coreTime time,
                           const double state[],
                           const double dstate_dt[],
                           const solverMode &sMode) override;  // for saving the state
    virtual void guessState (coreTime time,
                             double state[],
                             double dstate_dt[],
                             const solverMode &sMode) override;  // for initial setting of the state
    virtual void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    // virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const
    // override;
    virtual void setFlag (const std::string &flag, bool val = true) override;

    virtual void add (coreObject *obj) override;
    /** @brief additional add function specific to subModels
    @param[in] a submodel to add
    @throw unrecognizedObjectError is object is not valid*/
    virtual void add (gridSubModel *obj) override;

    virtual void algebraicUpdate (const IOdata &inputs,
                                  const stateData &sD,
                                  double update[],
                                  const solverMode &sMode,
                                  double alpha) override;
    virtual void
    residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    virtual IOdata getOutputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;

    virtual void
    derivative (const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;

    virtual void outputPartialDerivatives (const IOdata &inputs,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual void ioPartialDerivatives (const IOdata &inputs,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;
    virtual count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;

    virtual void jacobianElements (const IOdata &inputs,
                                   const stateData &sD,
                                   matrixData<double> &md,
                                   const IOlocs &inputLocs,
                                   const solverMode &sMode) override;
    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

    virtual void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;

    virtual void
    rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    virtual void rootTrigger (coreTime time,
                              const IOdata &inputs,
                              const std::vector<int> &rootMask,
                              const solverMode &sMode) override;
    virtual change_code
    rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level) override;

    using Generator::getReactivePower;
    using Generator::getRealPower;
    virtual double
    getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double
    getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;

    virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
    virtual coreObject *find (const std::string &object) const override;
    virtual coreObject *getSubObject (const std::string &typeName, index_t num) const override;
    virtual double
    getFreq (const stateData &sD, const solverMode &sMode, index_t *freqOffset = nullptr) const override;
    virtual double
    getAngle (const stateData &sD, const solverMode &sMode, index_t *angleOffset = nullptr) const override;

  protected:
    virtual double pSetControlUpdate (const IOdata &inputs, const stateData &sD, const solverMode &sMode);
    virtual double vSetControlUpdate (const IOdata &inputs, const stateData &sD, const solverMode &sMode);
    virtual index_t pSetLocation (const solverMode &sMode);
    virtual index_t vSetLocation (const solverMode &sMode);

  protected:
    class subModelInputs
    {
      public:
        std::vector<IOdata> inputs;
        count_t seqID = 0;
        subModelInputs ();
    };
    class subModelInputLocs
    {
      public:
        IOlocs genModelInputLocsInternal;
        IOlocs genModelInputLocsExternal;
        std::vector<IOlocs> inputLocs;
        count_t seqID = 0;
        subModelInputLocs ();
    };
    subModelInputs subInputs;
    subModelInputLocs subInputLocs;

    virtual void generateSubModelInputs (const IOdata &inputs, const stateData &sD, const solverMode &sMode);
    virtual void generateSubModelInputLocs (const IOlocs &inputLocs, const stateData &sD, const solverMode &sMode);

    gridSubModel *replaceModel (gridSubModel *newObject, gridSubModel *oldObject, index_t newIndex);

    void buildDynModel (dynModel_t dynModel);
};

}  // namespace griddyn
