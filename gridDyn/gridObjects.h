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

#ifndef GRIDOBJECT_H_
#define GRIDOBJECT_H_
#include "gridCore.h"
#include "gridObjectsHelperClasses.h"

#include <bitset>

class gridBus;
class gridArea;
class gridLink;
class gridRelay;

//forward declare the templated class matrixData
template<class Y>
class matrixData;



class violation;

/** @brief base object for gridDynSimulations
* the basic object for creating a power system encapsulating some common functions and data that is needed by all objects in the simulation
* and defining some common methods for use by all objects.  This object is not really intended to be instantiated directly and is mostly a
* common interface to inheriting objects gridPrimary, gridSecondary, and gridSubModel as it encapsulated common functionality between those objects
**/
class gridObject : public gridCoreObject
{

protected:
	count_t numParams = 0;       //!< the number of parameters to store in an archive
	double m_baseFreq = kWS;              //!<[radians per second] the base frequency of the system default to 60Hz
	double systemBasePower = 100;		//!<[MVA] base power for all PU computations 
    offsetTable offsets;              //!<a table of offsets for the different solver modes
  std::bitset<64> opFlags;                    //!< operational flags these flags are designed to be normal false
  std::vector<double> m_state;              //!<storage location for internal state
  std::vector<double> m_dstate_dt;             //!<storage location for internal state differential
  std::vector<gridObject *>subObjectList;        //!<a vector of all the subObjects;
  // stringVec stateNames;           //!<a vector with the names of the states
public:
  /** @brief default constructor*/
  explicit gridObject (const std::string &objName = "");
  /** @brief virtual destructor*/
  virtual ~gridObject ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype = paramStringType::all) const override;
  virtual void set (const std::string &param, const std::string &val) override;

  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void setFlag (const std::string &flag, bool val = true) override;
  virtual bool getFlag (const std::string &param) const override;
  virtual double get(const std::string &param, gridUnits::units_t unitType=gridUnits::defUnit) const override;

  /** @brief method for checking a specific known flag
  @param[in] flagID the index of the flag to check
  @return boolean indicator of the flag
  */
  bool checkFlag (index_t flagID) const;

  inline std::uint64_t cascadingFlags () const
  {
    return (opFlags.to_ullong () & flagMask);
  }

  /** @brief set the offsets of an object for a particular optimization mode using a single offset.
  \param[in] offset the offset index all variables are sequential.
  \param sMode the solver mode to use.
  */
  virtual void setOffset (index_t offset, const solverMode &sMode);

  /** @brief set the offsets of an object for a particular optimization mode using a single offset.
  \param newOffsets the offset index all variables are sequential.
  \param sMode the solver mode to use.
  */
  virtual void setOffsets (const solverOffsets &newOffsets, const solverMode &sMode);

  /** @brief get a state value
  \param offset the offset index: all state variables are sequential.
  @return the value of the state in question
  */
  virtual double getState (index_t offset) const;

  /** @brief set the offsets for root finding
  \param[in] Roffset the offset index all variables are sequential.
  \param[in] sMode the solver mode to use.
  */
  virtual void setRootOffset (index_t Roffset, const solverMode &sMode);

  /** @brief set the offsets for parameter control
  \param[in] Poffset the offset index all variables are sequential.
  \param[in] sMode the solver mode to use.
  */
  virtual void setParamOffset (index_t Poffset, const solverMode &sMode);

  /** @brief get the number of states
  @param[in] sMode  the solverMode to get the stateSize for
    @return the number of states
  */
  count_t stateSize (const solverMode &sMode);

  /** @brief get the total number of Algebraic States algSize+vSize+aSize
  @param[in] sMode solverMode to base the count on
  @return the number of algebraic states
  */
  count_t totalAlgSize (const solverMode &sMode);

  /** @brief get the number of Jacobian non-zeros elements (max not necessarily actual)
  @param[in] sMode solverMode to base the count on
  @return the number of non-zeros in the Jacobian
  */
  count_t jacSize (const solverMode &sMode);
  /** @brief get the number of roots
  @param[in] sMode solverMode to base the count on
  @return the number of roots
  */
  count_t rootSize (const solverMode &sMode);
  /** @brief get the number of algebraic states not including voltage and angle states
  @param[in] sMode solverMode to base the count on
  @return the number of algebraic states
  */
  count_t algSize (const solverMode &sMode);
  /** @brief get the number of differential states
   @param[in] sMode solverMode to base the count on
  @return the number of differential states
  */
  count_t diffSize (const solverMode &sMode);
  /** @brief get the number of voltage states
  @param[in] sMode solverMode to base the count on
  @return the number of voltage states
  */
  count_t voltageStateCount (const solverMode &sMode);
  /** @brief get the number of angle states
  @param[in] sMode  solverMode to base the count on
  @return the number of angle states
  */
  count_t angleStateCount (const solverMode &sMode);
  /** @brief get the controllable parameters
  @param[in] sMode  the solverMode to base the count on
  @return the number of parameters
  */
  count_t paramSize (const solverMode &sMode);

  /** @brief get the number of states
  @param[in] sMode  the solverMode to get the stateSize for
  @return the number of states
  */
  count_t stateSize (const solverMode &sMode) const;

  /** @brief get the total number of Algebraic States algSize+vSize+aSize
  @param[in] sMode solverMode to base the count on
  @return the number of algebraic states
  */
  count_t totalAlgSize (const solverMode &sMode) const;

  /** @brief get the number of Jacobian non-zeros elements (max not necessarily actual)
  @param[in] sMode solverMode to base the count on
  @return the number of non-zeros in the Jacobian
  */
  count_t jacSize (const solverMode &sMode) const;
  /** @brief get the number of roots
  @param[in] sMode solverMode to base the count on
  @return the number of roots
  */
  count_t rootSize (const solverMode &sMode) const;
  /** @brief get the number of algebraic states not including voltage and angle states
  @param[in] sMode solverMode to base the count on
  @return the number of algebraic states
  */
  count_t algSize (const solverMode &sMode) const;
  /** @brief get the number of differential states
  @param[in] sMode solverMode to base the count on
  @return the number of differential states
  */
  count_t diffSize (const solverMode &sMode) const;
  /** @brief get the number of voltage states
  @param[in] sMode solverMode to base the count on
  @return the number of voltage states
  */
  count_t voltageStateCount (const solverMode &sMode) const;
  /** @brief get the number of angle states
  @param[in] sMode  solverMode to base the count on
  @return the number of angle states
  */
  count_t angleStateCount (const solverMode &sMode) const;
  /** @brief get the controllable parameters
  @param[in] sMode  the solverMode to base the count on
  @return the number of parameters
  */
  count_t paramSize (const solverMode &sMode) const;
  /** @brief function to get a constant pointer to the object offsets 
   * @param[in] sMode the mode to get the offsets for
   * return a const pointer to the solver Offsets
   */
  const solverOffsets *getOffsets(const solverMode &sMode) const;

  /** @brief checks if the object is loaded
  @param[in] sMode  the solverMode to get the stateSize for
  @return boolean true if the object is loaded false if not
  */
  bool isLoaded (const solverMode &sMode, bool dynonly) const;
  /** @brief convenience function for checking armed status
  @return boolean true if the object is armed false if not
  */
  bool isArmed () const;
  /** @brief function for checking connected status
  @return boolean true if the object is connect false if not
  */
  virtual bool isConnected () const;

  virtual void reconnect ();
  virtual void disconnect ();

  /** @brief common function to load some flags appropriate to pFlow.
  */
  void setupPFlowFlags ();
  /** @brief common function to load some flags appropriate to dynamic simulation.
  */
  void setupDynFlags ();
  /** @brief compute the sizes and store them in the offsetTables.
  \param sMode the optimization mode to use.
  \param dynOnly set to true if only the dynamic conditions are of concern
  */
  virtual void loadSizes (const solverMode &sMode, bool dynOnly = false);
  /** @brief compute the sizes of the subobject and store them in the offsetTables.
  \param sMode the solver mode to use.
  \param dynOnly set to true if only the dynamic conditions are of concern
  */
  void loadSizesSub (const solverMode &sMode, bool dynOnly);
  /** @brief transfer a computed state to the objects
  \param ttime -the time the state corresponds to
  \param state -- a double array pointing to the state information
  \param dstate_dt a double array pointing to the state derivative information (not necessary for states with no corresponding time derivative
  \param sMode  -- the solverMode corresponding to the computed state.
  */
  virtual void setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode);
  /** @brief transfer state information from the objects to a vector
  \param ttime -the time the state corresponds to
  \param[out] state -- a double array pointing to the state information
  \param[out] dstate_dt a double array pointing to the state derivative information (not necessary for states with no corresponding time derivative
  \param sMode  -- the solverMode corresponding to the computed state.
  */
  virtual void guess (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode);
  /** @brief load tolerance information from the objects
  \param[out] tols -- a double array with the state tolerance information
  \param[in] sMode  -- the solverMode corresponding to the computed state.
  */
  virtual void getTols (double tols[], const solverMode &sMode);
  /** @brief load variable information 1 for algebraic state 0 for differential state
  \param[out] sdata -- a double array with the state tolerance information
  \param[in] sMode  -- the solverMode corresponding to the computed state.
  */
  virtual void getVariableType (double sdata[], const solverMode &sMode);
  /** @brief load constraint information
   0 for no constraints
  1 for > 0
  -1 for <0
  2 for >=0
  -2 for <=0
  \param[out] constraints -- a double array with the constraint
  \param[in] sMode  -- the solverMode corresponding to the computed state.
  */
  virtual void getConstraints (double constraints[], const solverMode &sMode);
  /** @brief update cascading flag information
  \param[in] dynamics  if true only do so for flags corresponding to dynamic solution
  */
  virtual void updateFlags (bool dynamics = true);
  /** @brief get the names for all the states
  \param[out] stNames -- the output state names
  \param[in] sMode  -- the solverMode corresponding to the computed state.
  \param[in] prefix  a string prefix to put before the state names of the object-- intended for cascading calls
  */
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const;
  friend class offsetTable;
};

/** @brief locations for secondary input parameters (aka bus output locations)*/
enum secondary_input_locations
{
  voltageInLocation = 0,
  angleInLocation = 1,
  frequencyInLocation = 2,
};
/** @brief locations grid secondary output locations*/
enum secondary_output_locations
{
  PoutLocation = 0,
  QoutLocation = 1,
};

/** @brief base class for top level simulation objects including gridBus, gridLink, gridRelays, and gridArea
  gridPrimary class defines the interface for gridPrimary objects which are nominally objects that can be contained by a root object which is an area usually,  though there is no restriction
 in other classes also containing primary objects.

**/
class gridPrimary : public gridObject
{
public:
  /**@brief default constructor*/
  explicit gridPrimary (const std::string &objName = "");

  /** @brief initialize object for power flow part A
   after part A of the initialization the object should know how many states it has as part of the powerflow, before this calling the load sizes
  powerflow related solverModes will be unspecified. This function is a wrapper around the pFlowObjectInitializeA function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  @return  an execution success  will return FUNCTION_EXECUTION_SUCCESS(0) if nothing went wrong
  */
  void pFlowInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize object for power flow part B
   partB is to actually initialize the object so an initial guess will be meaningful,  many objects just do everything in part A if there is no need to separate the functions
  This function is a wrapper around the pFlowObjectInitializeB function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  */
  void pFlowInitializeB ();

  /** @brief initialize object for dynamic simulation part A
   after part A of the initialization the object should know how many states it has as part of the dynamic simulation, before this calling the load sizes
  dynamic related solverModes will be unspecified. This function is a wrapper around the dynObjectInitializeA function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  void dynInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize object for dynamic simulation part B
   partB is to actually initialize the object so an initial guess will be meaningful,  many objects just do everything in part A if there is no need to separate the functions
  This function is a wrapper around the dynObjectInitializeB function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in]  outputSet the desired output of the gridPrimary
  */
  void dynInitializeB (IOdata &outputSet);

protected:
  /** @brief initialize local object for power flow part A
 see pFlowInitializeA for more details
@param[in] time0 the time0 at which the power flow will take place
@param[in] flags  any flags indicating how the initialization or execution will take place
*/
  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize local object for power flow part B
   see pFlowInitializeB for more details
  */
  virtual void pFlowObjectInitializeB ();

  /** @brief initialize local object for dynamics part A
   see dynInitializeA for more details
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize local object for dynamics part B
   see dynInitializeB for more details
  @param[in]  the desired initial output of the gridPrimary
  */
  virtual void dynObjectInitializeB (IOdata &outputSet);
public:
  /** @brief call any objects that needs 2 part execution to allow for parallelism
 do any prework for the calculation calls (residual, derivative, Jacobian, algebraicUpdate) later in the calculations
it is assumed any appropriate data would be cached during this time and not rerun if called multiple times
@param[in] sD the data representing the current state to operate on
@param[in] sMode the solverMode which is being solved for
*/
  virtual void preEx (const stateData *sD, const solverMode &sMode);

  //Jacobian computation
  /** @brief compute the Jacobian for a given state
  @param[in] sD the data representing the current state to operate on
  @param[out] ad the matrixData structure to store the Jacobian values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);

  /**
  *@brief compute the partial derivatives of the output states with respect to internal states
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] sMode the operations mode
  **/
  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);

  //residual computation
  /** @brief compute the residual for a given state
  @param[in] sD the data representing the current state to operate on
  @param[out] resid the array to store the residual values in
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode);

  /** @brief compute the time derivatives for a given state for the differential variables
  @param[in] sD the data representing the current state to operate on
  @param[out] deriv the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void derivative (const stateData *sD, double deriv[], const solverMode &sMode);

  /** @brief compute updates for all the algebraic variable
  @param[in] sD the data representing the current state to operate on
  @param[out] update the array to store the computed algebraic states
  @param[in] sMode the solverMode which corresponds to the stateData
  @param[in] alpha the convergence gain
  */
  virtual void algebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha);

  /** @brief get the residual computation for object requiring a delay
    basically calls the residual calculation on the delayed objects
  @param[in] sD the data representing the current state to operate on
  @param[out] resid the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void delayedResidual (const stateData *sD, double resid[], const solverMode &sMode);

  /** @brief get the residual computation for object requiring a delay
    basically calls the derivative calculation on the delayed objects
  @param[in] sD the data representing the current state to operate on
  @param[out] deriv the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void delayedDerivative (const stateData *sD, double deriv[], const solverMode &sMode);

  /** @brief get the algebraic update for object requesting a delay
    basically calls the residual calculation on the delayed objects
  @param[in] sD the data representing the current state to operate on
  @param[out] update the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void delayedAlgebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha);

  /** @brief get the residual computation for object requiring a delay
    basically calls the Jacobian calculation on the delayed objects
  @param[in] sD the data representing the current state to operate on
  @param[out] ad the matrixData structure to store the Jacobian values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void delayedJacobian (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);

  //for the stepwise dynamic system
  /** @brief move the object forward in time using local calculations
    tells the object to progress time to a certain point the sMode is guidance on how to do it not necessarily indicative of a particular solver
  it is meant as a suggestion not a requirement.
  @param[in] ttime the time to progress to
  @param[in] sMode the solverMode to give guidance to objects on how to perform internal calculations
  */
  virtual void timestep (gridDyn_time ttime, const solverMode &sMode);

  /**
  *@brief set all the values of particular type of object to some value
  * @param[in] type the type of unit to set
  * @param[in] param the parameter to set
  * @param[in] val, the value to set it to
  * @param[in] unitType the units of the val
  */
  virtual void setAll (const std::string &type, std::string param, double val, gridUnits::units_t unitType = gridUnits::defUnit);

  //for rootfinding in the dynamic solver
  /**
  *@brief evaluate the root functions and return the value
  * @param[in] sD the current state data for the simulation
  * @param[out] roots the memory to store the root evaluation
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTest (const stateData *, double roots[], const solverMode &sMode);

  /**
  *@brief a root has occurred now take action
  * @param[in] ttime the simulation time the root evaluation takes place
  * @param[in] rootMask an integer array the same size as roots where a 1 indicates a root has been found
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTrigger (gridDyn_time ttime, const std::vector<int> &rootMask, const solverMode &sMode);

  /**
  *@brief evaluate the root functions and execute trigger from local state information
   for operation after an initial condition check or timestep
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] level the level of roots to check can be REVERSIBLE or NON_REVERSIBLE
  @return a change code indicating what if any changes in the object took place
  **/
  virtual change_code rootCheck (const stateData *sD, const solverMode & sMode, check_level_t level);

  /** @brief check power flow adjustable parameters
  * adjust any parameters or checks needed on a power flow solution,  such as generator power limits, voltage levels, stepped adjustable taps, etc
  * @param[in] flags,  a set of flags for controlling the adjustments
  @param[in] level - the level of adjustments can be REVERSIBLE or NON_REVERSIBLE
  @return a change code indicating what if any changes in the object took place
  */
  virtual change_code powerFlowAdjust (unsigned long flags, check_level_t level);

  /** @brief  try to shift the states to something more consistent
    called when the current states do not make a consistent condition,  calling converge will attempt to move them to a more valid state
  mode controls how this is done  0- does a single iteration loop
  mode=1 tries to iterate until convergence based on tol
  mode=2  tries harder
  mode=3 does it with voltage only
  @pararm[in] ttime  the time of the corresponding states
  @param[in,out]  state the states of the system at present and shifted to match the updates
  @param[in,out] dstate_dt  the derivatives of the state that get updated
  @param[in] sMode the solverMode matching the states
  @param[in] mode  the mode of the convergence
  @param[in] tol  the convergence tolerance
  */
  virtual void converge (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode mode= converge_mode::high_error_only, double tol = 0.01);
  /** @brief reset voltages
  * resets the voltage levels and any other parameters changed in power to a base level depending on the level
  * @param[in] level  the level of the reset
  */
  virtual void reset (reset_levels level = reset_levels::minimal);

  /** @brief do a check on the power flow results
  * checks for an violations of recommended power flow levels such as voltage, power limits, transfer capacity, angle limits, etc
  * @param[out] Violation_vector, a storage location for any detected violations
  */
  virtual void pFlowCheck (std::vector<violation> & Violation_vector);

  /** @brief do any power computation based on the state specified and cache results
  @param[in] sD  the state data to do the computation on
  @param[in] sMode the solverMode corresponding to the state data*/
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode);

  /** @brief do any local computation to get ready for measurements*/
  virtual void updateLocalCache ();

  /**
  *@brief get a vector of all outputs
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs
  **/
  virtual IOdata getOutputs (const stateData *sD, const solverMode &sMode);

  /**
  *@brief get a vector state indices for the output
  @details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs locations,  kNullLocation if there is no state representing the output
  **/
  virtual IOlocs getOutputLocs  (const solverMode &sMode) const;

  /**
  *@brief get the state index of an output
  @details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the state requested
  **/
  virtual index_t getOutputLoc (const solverMode &sMode, index_t num = 0) const;

  /**
  *@brief get the time derivative of a single state
   if the output is algebraic the return value may not be meaningful
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the time derivative of a state being requested
  **/
  virtual double getDoutdt (const stateData *sD, const solverMode &sMode, index_t num = 0) const;

  /**
  *@brief get a single state
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (const stateData *sD, const solverMode &sMode, index_t num = 0) const;

  /**
  *@brief get a single state based on local data
  @param[in] num the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (index_t /*num*/ = 0) const;


  virtual void alert (gridCoreObject *, int code) override;

  /**
  *@brief get a pointer for a particular bus
  @param[in] num the index of the bus being requested
  @return a pointer to the requested bus or nullptr
  **/
  virtual gridBus * getBus (index_t num) const;

  /**
  *@brief get a pointer for a particular Link
  @param[in] num the index of the link being requested
  @return a pointer to the requested link or nullptr
  **/
  virtual gridLink * getLink (index_t num) const;

  /**
  *@brief get a pointer for a particular Area
  @param[in] num the index of the area being requested
  @return a pointer to the requested area or nullptr
  **/
  virtual gridArea * getArea (index_t num) const;

  /**
  *@brief get a pointer for a particular relay
  @param[in] num the index of the relay being requested
  @return a pointer to the requested relay or nullptr
  **/
  virtual gridRelay * getRelay (index_t num) const;


};






/** @brief base class for objects that can attach to a bus like gridLoad and gridDynGenerator
*  gridSecondary class defines the interface for secondary objects which are nominally objects that hang directly off a bus
**/
class gridSecondary : public gridObject
{
public:
  /** @brief default constructor*/
  explicit gridSecondary (const std::string &objName="");

  /** @brief initialize object for power flow part A
   after part A of the initialization the object should know how many states it has as part of the powerflow, before this calling the load sizes
  powerflow related solverModes will be unspecified. This function is a wrapper around the pFlowObjectInitializeA function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  void pFlowInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize object for power flow part B
   partB is to actually initialize the object so an initial guess will be meaningful,  many objects just do everything in part A if there is no need to separate the functions
  This function is a wrapper around the pFlowObjectInitializeB function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  */
  void pFlowInitializeB ();

  /** @brief initialize object for dynamic simulation part A
   after part A of the initialization the object should know how many states it has as part of the dynamic simulation, before this calling the load sizes
  dynamic related solverModes will be unspecified. This function is a wrapper around the dynObjectInitializeA function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  void dynInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize object for dynamic simulation part B
   partB is to actually initialize the object so an initial guess will be meaningful,  many objects just do everything in part A if there is no need to separate the functions
  This function is a wrapper around the dynObjectInitializeB function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] args  the input arguments typically [voltage, angle, frequency]
  @param[in]  outputSet  the desired output of the gridSecondary [P,Q]
  */
  void dynInitializeB (const IOdata & args, const IOdata & outputSet);

protected:
  /** @brief initialize local object for power flow part A
 see pFlowInitializeA for more details
@param[in] time0 the time0 at which the power flow will take place
@param[in] flags  any flags indicating how the initialization or execution will take place
@return  an execution success  will return FUNCTION_EXECUTION_SUCCESS(0) if nothing went wrong
*/
  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize local object for power flow part B
   see pFlowInitializeB for more details
  @return  an execution success  will return FUNCTION_EXECUTION_SUCCESS(0) if nothing went wrong
  */
  virtual void pFlowObjectInitializeB ();

  /** @brief initialize local object for dynamics part A
   see dynInitializeA for more details
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags);

  /** @brief initialize local object for dynamics part B
   see dynInitializeB for more details
  @param[in] args  the input arguments typically [voltage, angle, frequency]
  @param[in]  the desired initial output of the gridPrimary
  */
  virtual void dynObjectInitializeB (const IOdata & args, const IOdata & outputSet);
public:
  //for the stepwise dynamic system
  /** @brief move the object forward in time using local calculations
    tells the object to progress time to a certain point the sMode is guidance on how to do it not necessarily indicative of a particular solver
  it is meant as a suggestion not a requirement.
  @param[in] ttime the time to progress to
  @param[in] args  the input arguments
  @param[in] sMode the solverMode to give guidance to objects on how to perform internal calculations
  */
  virtual void timestep (gridDyn_time ttime, const IOdata & args, const solverMode & sMode);

  /** @brief call any objects that need 2 part execution to allow for parallelism
   do any prework for a residual call later in the calculations
  secondary objects do not allow partial computation like primary objects can so there is no delayed computation calls
  just the regular call and the primary object will handle delaying action.
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void preEx (const IOdata & args, const stateData *sD, const solverMode & sMode);

  //residual computation
  /** @brief compute the residual for a given state
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] resid the array to store the residual values in
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void residual (const IOdata & args, const stateData *sD, double resid[], const solverMode & sMode);

  /** @brief compute an update to all the algebraic variables in the object
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] update the array to store the computed state valuse
  @param[in] sMode the solverMode which is being solved for
  @param[in] alpha the convergence gain
  */

  virtual void algebraicUpdate (const IOdata & args, const stateData *sD, double update[], const solverMode & sMode, double alpha);

  /** @brief compute the time derivative for a given state
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] deriv the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void derivative (const IOdata & args, const stateData *sD, double deriv[], const solverMode &sMode);

  /**
  *@brief get the real output power
  @detail  these are the main data output functions for secondary object.  The intention is to supply the load/gen to the bus
  they may have other outputs but first and second are the real and reactive power negative numbers indicate power generation
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return the real output power consumed or generated by the device
  **/
  virtual double getRealPower (const IOdata & args, const stateData *sD, const solverMode & sMode);

  /**
  *@brief get the reactive output power
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return the reactive output power consumed or generated by the device
  **/
  virtual double getReactivePower (const IOdata & args, const stateData *sD, const solverMode & sMode);

  /**
  *@brief get the real output power from local data
  @return a the real power output
  **/
  virtual double getRealPower () const;

  /**
  *@brief get the reactive output power from local data
  @return a the reactive power output
  **/
  virtual double getReactivePower () const;

  /**
  *@brief get a vector of all outputs
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs
  **/
  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode);

  /**
  *@brief get a vector state indices for the output
  @ details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs locations,  kNullLocation if there is no state representing the output
  **/
  virtual IOlocs getOutputLocs  (const solverMode &sMode) const;

  /**
  *@brief get a single output and location
  @ details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the location of the output state requested
  **/
  virtual index_t getOutputLoc ( const solverMode &sMode, index_t num = 0);

  /**
  *@brief get the time derivative of a single state
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the time derivative of a state being requested
  **/
  virtual double getDoutdt (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0);

  /**
  *@brief get a single state
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0);

  /**
  *@brief get a single state based on local data
  @param[in] the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (index_t num = 0) const;

  /**
  *@brief predict what the outputs will be at a specific time in the future
  @param[in] ptime the time for which to make a prediction
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the predicted outputs
  **/
  virtual IOdata predictOutputs (double ptime, const IOdata &args, const stateData *sD, const solverMode &sMode);

  /**
  *@brief get the available upwards generating capacity of a system
  @param[in] time the time period within which to do the adjustment
  @return the available up capacity of the gridSecondary unit
  **/
  virtual double getAdjustableCapacityUp (gridDyn_time time = kBigNum) const;

  /**
  *@brief get the available downwards generating capacity of a system
  @param[in] the time period within which to do the adjustment
  @return the available up capacity of the gridSecondary unit
  **/
  virtual double getAdjustableCapacityDown (gridDyn_time time = kBigNum) const;

  /**
  *@brief compute the partial derivatives of the internal states with respect to inputs and other internal states
   @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] argLocs the vector of input argument locations
  * @param[in] sMode the operations mode
  **/
  virtual void jacobianElements (const IOdata & args, const stateData *sD, matrixData<double> &ad, const IOlocs & argLocs, const solverMode & sMode);

  /**
  *@brief compute the partial derivatives of the output states with respect to internal states
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] sMode the operations mode
  **/
  virtual void outputPartialDerivatives  (const IOdata & args, const stateData *sD, matrixData<double> &ad, const solverMode & sMode);

  /**
  *@brief compute the partial derivatives of the output states with respect to inputs
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] argLocs the vector of input argument locations
  * @param[in] sMode the operations mode
  **/
  virtual void ioPartialDerivatives (const IOdata & args, const stateData *sD, matrixData<double> &ad, const IOlocs & argLocs, const solverMode & sMode);

  /**
  *evaluate the root functions and return the value
  @param[in] args the inputs for the secondary object
  * @param[in] sD the state of the system
  * @param[out] roots the memory to store the root evaluation
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTest  (const IOdata & args, const stateData *sD, double roots[], const solverMode & sMode);

  /**
  *a root has occurred now take action
  * @param[in] ttime the simulation time the root evaluation takes place
  @param[in] a vector of ints representing a rootMask  (only object having a value of 1 in their root locations should actually trigger
  * @param[in] rootMask an integer array the same size as roots where a 1 indicates a root has been found
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTrigger (gridDyn_time ttime, const IOdata & args, const std::vector<int> & rootMask, const solverMode & sMode);

  /**
  *evaluate the root functions and execute trigger from a static state for operation after an initial condition check
  * @param[in] ttime the simulation time the root evaluation takes place
  @param[in] args the inputs for the secondary object
  * @param[in] sD the state of the system
  * @param[in] sMode the mode the solver is in
  @param[in] level the level of root to check for
  **/
  virtual change_code rootCheck (const IOdata & args, const stateData *sD, const solverMode &sMode, check_level_t level);

  /** @brief reset power flow parameters
  *  resets the voltage levels and any other parameters changed in power to a base level depending on the level
  * @param[in] level,  the level of the reset
  */
  virtual void reset (reset_levels level = reset_levels::minimal);

  /** @brief adjust the power flow solution if needed
  @param[in] args the inputs for the secondary object
  @param[in]  flags for suggesting how the object handle the adjustments
  * @param[in] level  the level of the adjustments to perform
  */
  virtual change_code powerFlowAdjust (const IOdata & args, unsigned long flags, check_level_t level);

  /** @brief locate a state index based on field name
  @param[in] the name of the field to search for
  @param[in]  the solverMode to find the location for
  @return the index of the state  some number if valid  kInvalidLocation if not found, kNullLocation if not initialized yet(try again later)
  */
  virtual index_t findIndex (const std::string & field, const solverMode & sMode) const;

  /**brief update any local cached information about a particular state/input set
  @param[in] args the input args
  @param[in] sD  the stage data to cache information from
  @param[in] sMode the solverMode corresponding to the stateData
  */
  virtual void updateLocalCache (const IOdata & args, const stateData *sD, const solverMode &sMode);
};


/** @brief base class for any model can act as a component of another model
* gridSubModel class defines the interface for models which can act as components of other models such as gridDynExciter, or gridDynGovernor
most of the differential equations are contained in submodels.  The interface is meant to be flexible so unlike gridSecondary models there is no predefined interface, but at the same time
many of the function calls are intended to be the same,  The main difference being there is only one initialize function, they can operate in power flow but those objects just call initialize twice

**/
class gridSubModel : public gridObject
{
protected:
  count_t m_inputSize = 0;            //!< the required size of the args input
  count_t m_outputSize = 1;            //!< the number of outputs the submodel produces
  double m_output = 0.0;              //!< storage location for the current output
public:
  /** @brief default constructor*/
  explicit gridSubModel (const std::string &objName = "submodel_#");

  //TODO:: PT add a clone function to copy over the input and output sizes
  /** @brief initialize the submodel for partA
   after part A of the initialization the object should know how many states it has as part of the simulation, before calling this function load sizes
  on the related solverModes will be unspecified. This function is a wrapper around the objectInitializeA function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  void initializeA (gridDyn_time time, unsigned long flags);
  /** @brief initialize object for simulation part B
   partB is to actually initialize the object so an initial guess will be meaningful,  many objects just do everything in part A if there is no need to separate the functions
  This function is a wrapper around the dynObjectInitializeB function which does the local object init
  this function handles any global setup and makes sure all the flags are set properly
  @param[in] args  the input arguments
  @param[in]  outputSet  the desired output
  @param[out] fieldSet the basic concept is that either the args or outputSet can be given or both if available
                        fieldSet then contains the appropriate input or output depending on what was given
  */
  void initializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet);
protected:
  /** @brief initialize local object for power flow part A
   see initializeA for more details
  @param[in] time0 the time0 at which the power flow will take place
  @param[in] flags  any flags indicating how the initialization or execution will take place
  */
  virtual void objectInitializeA (gridDyn_time time0, unsigned long flags);
  /** @brief initialize local object part B
   see initializeB for more details ,all arguments are pass through from the initializeB function
  @param[in] args  the input arguments
  @param[in]  the desired initial output
  @param[out] fieldSet the basic concept is that either the args or outputSet can be given or both if available
        fieldSet then contains the appropriate input or output depending on what was given
  */
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &fieldSet);

public:
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
  //for the stepwise dynamic system
  /** @brief move the object forward in time using local calculations
    tells the object to progress time to a certain point the sMode is guidance on how to do it not necessarily indicative of a particular solver
  it is meant as a suggestion not a requirement.
  @param[in] ttime the time to progress to
  @param[in] args  the input arguments
  @param[in] sMode the solverMode to give guidance to objects on how to perform internal calculations
  */
  virtual void timestep (gridDyn_time ttime, const IOdata & args, const solverMode &sMode);

  /**
  *evaluate the root functions and return the value
  @param[in] args the inputs for the secondary object
  * @param[in] sD the state of the system
  * @param[out] roots the memory to store the root evaluation
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTest (const IOdata & args, const stateData *sD, double roots[], const solverMode &sMode);
  /**
  *a root has occurred now take action
  * @param[in] ttime the simulation time the root evaluation takes place
  @param[in] a vector of ints representing a rootMask  (only object having a value of 1 in their root locations should actually trigger
  * @param[in] rootMask an integer array the same size as roots where a 1 indicates a root has been found
  * @param[in] sMode the mode the solver is in
  **/
  virtual void rootTrigger (gridDyn_time ttime, const IOdata & args, const std::vector<int> & rootMask, const solverMode & sMode);
  /**
  *evaluate the root functions and execute trigger from a static state for operation after an initial condition check
  * @param[in] ttime the simulation time the root evaluation takes place
  @param[in] args the inputs for the secondary object
  * @param[in] sD the state of the system
  * @param[in] sMode the mode the solver is in
  @param[in] level the level of root to check for
  **/
  virtual change_code rootCheck (const IOdata & args, const stateData *sD, const solverMode &sMode, check_level_t level);
  //residual computation
  /** @brief compute the residual for a given state
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] resid the array to store the residual values in
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void residual (const IOdata & args, const stateData *sD, double resid[], const solverMode &sMode);
  /** @brief compute the time derivative for a given state
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] deriv the array to store the computed derivative values
  @param[in] sMode the solverMode which is being solved for
  */
  virtual void derivative (const IOdata & args, const stateData *sD, double deriv[], const solverMode & sMode);
  /** @brief compute an update to all the algebraic variables in the object
  @param[in] args  the input arguments
  @param[in] sD the data representing the current state to operate on
  @param[out] update the array to store the computed state values
  @param[in] sMode the solverMode which is being solved for
  @param[in] alpha the convergence gain
  */
  virtual void algebraicUpdate (const IOdata & args, const stateData *sD, double update[], const solverMode & sMode, double alpha);
  /**
  *@brief compute the partial derivatives of the internal states with respect to inputs and other internal states
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] argLocs the vector of input argument locations
  * @param[in] sMode the operations mode
  **/
  virtual void jacobianElements (const IOdata & args, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode);
  /**
  *@brief compute the partial derivatives of the output states with respect to internal states
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] sMode the operations mode
  **/
  virtual void outputPartialDerivatives  (const IOdata & args, const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  /**
  *@brief compute the partial derivatives of the output states with respect to inputs
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[out] ad  the array to store the information in
  * @param[in] argLocs the vector of input argument locations
  * @param[in] sMode the operations mode
  **/
  virtual void ioPartialDerivatives (const IOdata & args, const stateData *sD, matrixData<double> &ad, const IOlocs & argLocs, const solverMode & sMode);

  /** @brief adjust the power flow solution if needed
  @param[in] args the inputs for the secondary object
  @param[in] flags for suggesting how the object handle the adjustments
  * @param[in] level  the level of the adjustments to perform
  */
  virtual change_code powerFlowAdjust (const IOdata & args, unsigned long flags, check_level_t level);

  /** @brief get the number of outputs
  @return the number of outputs
  */
  count_t numOutputs () const
  {
    return m_outputSize;
  }
  /** @brief get the number of inputs
  @return the number of inputs
  */
  count_t numInputs () const
  {
    return m_inputSize;
  }
  /**
  *@brief get a vector of all outputs
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs
  **/
  virtual IOdata getOutputs (const IOdata &args, const stateData *sD, const solverMode &sMode);
  /**
  *@brief get the time derivative of a single state
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the time derivative of a state being requested
  **/
  virtual double getDoutdt (const stateData *sD, const solverMode &sMode, index_t num = 0);
  /**
  *@brief get a single state
  @param[in] args the inputs for the secondary object
  * @param[in] sD the current state data for the simulation
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (const IOdata &args, const stateData *sD, const solverMode &sMode, index_t num = 0) const;
  /**
  *@brief get a single state based on local data
  @param[in] the number of the state being requested
  @return the value of the state being requested
  **/
  virtual double getOutput (index_t num = 0) const;
  /**
  *@brief get a single output and location
  @ details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @param[in] num the number of the state being requested
  @return the value of the state requested
  **/
  virtual index_t getOutputLoc (const solverMode &sMode, index_t num = 0) const;
  /**
  *@brief get a vector state indices for the output
  @ details used in cases where the state of one object is used int the computation of another for computation of the Jacobian
  * @param[in] sMode the mode the solver is in
  @return a vector containing  all the outputs locations,  kNullLocation if there is no state representing the output
  **/
  virtual IOlocs getOutputLocs  (const solverMode &sMode) const;
  /** @brief locate a state index based on field name
  @param[in] field the name of the field to search for
  @param[in] sMode the solverMode to find the location for
  @return the index of the state  some number if valid  kInvalidLocation if not found, kNullLocation if not initialized yet(try again later)
  */
  virtual index_t findIndex (const std::string & field, const solverMode & sMode) const;
  /** @brief get the local state names
  used within a couple functions to automate the population of the state names and finding of the indices  states should be algebraic states first,  then differential states
  @return a string vector of the local state names
  */
  virtual stringVec localStateNames () const;

  /**brief update any local cached information about a particular state/input set
   this function performs equivalent purpose as updateLocalCache in gridPrimary and intended to be used to compute cached values or prepare an object for calling the const output functions
  @param[in] args the input args
  @param[in] sD  the stage data to cache information from
  @param[in] sMode the solverMode corresponding to the stateData
  */
  virtual void updateLocalCache (const IOdata & args, const stateData *sD, const solverMode &sMode);
protected:
};

/** @brief display all the state names to the screen
@param obj the object to display the states for
@param sMode the mode which to display the states for
*/
void printStateNames (gridObject *obj, const solverMode &sMode);



#endif
