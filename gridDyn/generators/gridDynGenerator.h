/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef GRIDDYNGENERATOR_H_
#define GRIDDYNGENERATOR_H_

#include "gridObjects.h"

class gridDynExciter;
class gridDynGovernor;
class gridDynGenModel;
class gridDynPSS;

class scheduler;

class gridBus;

/**
@ brief class describing a generator unit
 a generator is a power production unit in Griddyn.  the base generator class implements methods set forth in the gridSecondary class and inherits from that class
it has mechanics and interfaces for handling any and all of 4 different components, namely and exciter, governor, generator model, and a power system stabilizer.
*/
class gridDynGenerator : public gridSecondary
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
  /** @brief flags for controlling operation of the generator*/
  enum generator_flags
  {
    variable_generation = object_flag1,  //!< flag indicating that the generator has uncontrolled variable generation
    reserve_capable = object_flag2,             //!< flag indicating the generator can act as spinning reserve
    agc_capable = object_flag3,                 //!< flag indicating the generator is capable of agc response
    use_capability_curve = object_flag4,                //!< flag indicating that the generator should use a capability curve rather than a fixed limit
    no_voltage_derate = object_flag5,                   //!< flag turning off voltage derating for low voltage power flow
    independent_machine_base = object_flag6,    //!< flag indicating that the generator has a different machine base than the simulation
    at_limit = object_flag7,                            //!< flag indicating the generator is operating at a limit
    indirect_voltage_control_level = object_flag8,  //!< flag indicating that the generator should perform voltage control indirectly in power flow
    internal_frequency_calculation = object_flag9,  //!<flag indicating that the generator computes the frequency internally
  };
  /** @brief enum indicating submodel locations in the subObject structure*/
  enum submodel_locations
  {
    genmodel_loc = 1, exciter_loc = 2, governor_loc = 3,pss_loc = 4
  };
  static count_t genCount;                                      //!< generator cound
  double baseVoltage = 120;             //!< [V] base voltage
protected:
  double Qmax = kBigNum;                                             //!< [pu mbase] max steady state reactive power values for Power flow analysis
  double Qmin = -kBigNum;                               //!< [pu mbase] min steady state reactive power values for Power flow analysis
  double Pmax = kBigNum;              //!< [pu mbase]max steady state real power values for the generator
  double Pmin = -kBigNum;         //!< [pu mbase] min steady state real power values for the generator
  double participation = 1.0;                                   //!< [%]a participation factor used in auto allocating load.
  double vRegFraction = 1.0;                 //!< [%]  fraction of output reactive power to maintain voltage regulation
  double machineBasePower = 100;                          //!< MW the internal base power of the generator;
  gridBus *bus = nullptr;                                               //!< pointer to the parent bus (usually just a cast of the parent object
  gridDynGenModel *genModel = nullptr;              //!< generator model
  gridDynExciter *ext = nullptr;                    //!< exciter model
  gridDynGovernor *gov = nullptr;                   //!< governor model
  gridDynPSS *pss = nullptr;                        //!< power system stabilizer type
  double P = 0.0;                    //!< [pu] Electrical generation real power output
  double Q = 0.0;                    //!< [pu] Electrical generation reactive power output
  double Pset = -kBigNum;                    //!< [pu] target power set point
  double dPdt = 0.0;                                //!< define the power ramp
  double dQdt = 0.0;                                  //!< define the reactive power ramp
  const double *m_stateTemp = nullptr;                       //!< temporary state vector(assumed not writable)
  const double *m_dstate_dt_Temp = nullptr;                   //!<  a temporary deriv vector;

  std::vector<double> m_state_ind;                              //!< a vector holding the indices to indicate if a variable is a algebraic or differential state
  count_t SSize = 0;                                                             //!< the total number of states
  double m_Eft = 0;                                             //!< place to store a constant the exciter field
  double m_Pmech = 0;                                           //!< place to store a constant power output
  double m_Vtarget = -1;                               //!< voltage target for the generator at the control bus
  double m_Rs = 0.0;                            //!< the real part of the generator impedance
  double m_Xs = 1.0;   //!< generatore impedence defined on Mbase;
  gridBus *remoteBus = nullptr;  //!< the bus for remote control
  scheduler *sched = nullptr;                   //!< pointer to a scheduler
  std::vector<double> PC;                       //!< power control point for the capability curve
  std::vector<double> minQPC;           //!< min reactive power corresponding to the PC point
  std::vector<double> maxQPC;           //!< max reactive power corresponding to the PC points

public:
  static dynModel_t dynModelFromString (const std::string &dynModelType);
  /** @brief default constructor
  @param[in] dynModel  a string with the dynmodel description*/
  gridDynGenerator (dynModel_t dynModel, const std::string &objName = "gen_$");
  gridDynGenerator (const std::string &objName = "gen_$");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  /** @brief destructor*/
  virtual ~gridDynGenerator ();

  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
  virtual void dynObjectInitializeA (double time, unsigned long flags) override;

  virtual void dynObjectInitializeB (const IOdata &args, const IOdata &outputSet) override;
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;       //for saving the state
  virtual void guess (double ttime, double state[],double dstate_dt[], const solverMode &sMode) override;               //for initial setting of the state

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual int setFlag (const std::string &flag, bool val = true) override;

  virtual int add (gridCoreObject *obj) override;
  /** @brief additional add function specific to subModels
  @param[in] a submodel to add
  @return OBJECT_ADD_SUCCESS if successful OBJECT_ADD_FAILURE if not*/
  virtual int add (gridSubModel *obj);

  void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void alert (gridCoreObject *object, int code) override;
  virtual void algebraicUpdate (const IOdata &args,const stateData *sD, double update[],const solverMode &sMode, double alpha) override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[],const solverMode &sMode) override;
  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode) override;

  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;

  virtual void outputPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void ioPartialDerivatives (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void jacobianElements  (const IOdata &args, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;

  virtual double timestep (double ttime, const IOdata &args, const solverMode &sMode) override;

  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (double ttime, const IOdata &args, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck ( const IOdata &args, const stateData *sD, const solverMode &sMode, check_level_t level) override;
  /** @brief get the generator states
  @return a double pointer to the states*/
  double * getStates ()
  {
    return m_state.data ();
  }
  double getState (index_t ind) const override
  {
    return m_state[ind];
  }
  /** @brief get the current generator set point
  @return the current generator set point*/
  virtual double getPset () const
  {
    return Pset;
  }
  virtual double getRealPower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getReactivePower (const IOdata &args, const stateData *sD, const solverMode &sMode) override;
  virtual double getRealPower () const override;
  virtual double getReactivePower () const override;
  /** @brief function to set the generator capability curve
  @param[in] Ppts  the points on the curve along the real power axis
  @param[in] Qminpts  the minimum reactive power generation corresponding to the Ppts
  @param[in] Qmaxpts  the maximum reactive power generation corresponding to the Ppts
  */
  virtual void setCapabilityCurve (std::vector<double> Ppts, std::vector<double> Qminpts, std::vector<double> Qmaxpts);

  virtual IOdata predictOutputs (double ptime, const IOdata &args, const stateData *sD, const solverMode &sMode) override;

  virtual double getAdjustableCapacityUp (double time = kBigNum) const override;       //get the available adjustment Up within the specified timeframe
  virtual double getAdjustableCapacityDown (double time = kBigNum) const override;       //get the available adjustment Up within the specified timeframe
/** @brief get the maximum generation attainable in a specific amount of time
@param[in] time  the time window to acheive the generation
@return the max real power*/
  virtual double getPmax (double time = kBigNum) const;
  /** @brief get the maximum reactive generation attainable in a specific amount of time
  @param[in] time  the time window to acheive the generation
  @param[in] Ptest the real power output corresponding to the desired attainable generation
  @return the max reactive power*/
  virtual double getQmax (double time = kBigNum,double Ptest = -kBigNum ) const;
  /** @brief get the minimum real generation attainable in a specific amount of time
  @param[in] time  the time window to acheive the generation
  @return the max real power*/
  virtual double getPmin (double time = kBigNum) const;
  /** @brief get the minimum reactive generation attainable in a specific amount of time
  @param[in] time  the time window to acheive the generation
  @param[in] Ptest the real power output corresponding to the desired attainable generation
  @return the min reactive power*/
  virtual double getQmin (double time = kBigNum, double Ptest = -kBigNum) const;
  /** @brief adjust the output generation by the specified amount
  @param[in] adjustment the value of the desired adjustment
  */
  virtual void powerAdjust (double adjustment);
  virtual change_code powerFlowAdjust (const IOdata &args, unsigned long flags, check_level_t level) override;      //only applicable in pFlow
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual gridCoreObject * find (const std::string &object) const override;
  virtual gridCoreObject * getSubObject (const std::string &type, index_t num) const override;
  virtual double getFreq (const stateData *sD, const solverMode &sMode, index_t *FreqOffset = nullptr) const;
  virtual double getAngle (const stateData *sD, const solverMode &sMode, index_t *AngleOffset = nullptr) const;
protected:
  virtual void updateFlags (bool dynOnly = false) override;
private:
  class subModelInputs
  {
public:
    IOdata genModelInputs;
    IOdata exciterInputs;
    IOdata governorInputs;
    IOdata pssInputs;
    count_t seqID = 0;
    subModelInputs ();
  };
  class subModelInputLocs
  {
public:
    IOlocs genModelInputLocsAll;
    IOlocs genModelInputLocsInternal;
    IOlocs genModelInputLocsExternal;
    IOlocs exciterInputLocs;
    IOlocs governorInputLocs;
    IOlocs pssInputLocs;
    count_t seqID = 0;
    subModelInputLocs ();
  };
  subModelInputs subInputs;
  subModelInputLocs subInputLocs;

  void generateSubModelInputs (const IOdata &args, const stateData *sD, const solverMode &sMode);
  void generateSubModelInputLocs (const IOlocs &argLocs, const stateData *sD, const solverMode &sMode);

  gridSubModel * replaceSubObject (gridSubModel *newObject, gridSubModel *oldObject,index_t newIndex);
  void setRemoteBus (gridCoreObject *newRemoteBus);

  void buildDynModel (dynModel_t dynModel);

};

#endif
