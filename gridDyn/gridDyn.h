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

#ifndef GRIDDYN_H_
#define GRIDDYN_H_

// debug
//#define DEBUG_IDA
#define GRIDDYN_MAJOR 0
#define GRIDDYN_MINOR 5
#define GRIDDYN_PATCH 5
#define GRIDDYN_DATE "2016-10-19"


// header files
#include "simulation/gridSimulation.h"
#include "simulation/gridDynActions.h"
// libraries
#include <queue>

#define SINGLE (1)
#define MULTICORE       (2)
#define MULTIPROCESSOR (3)

const std::string griddyn_version = std::to_string (GRIDDYN_MAJOR) + "." + std::to_string (GRIDDYN_MINOR) + "." + std::to_string (GRIDDYN_PATCH);
//#define GRIDDYN_VERSION_STRING "GridDyn version " MAKE_VERSION(GRIDDYN_MAJOR,GRIDDYN_MINOR,GRIDDYN_PATCH) GRIDDYN_DATE
const std::string griddyn_version_string = "GridDyn version " + griddyn_version + " " + GRIDDYN_DATE;


class contingency;
class continuationSequence;
class solverInterface;

//!<additional flags for the controlFlags bitset
enum gd_flags
{
  dense_solver = 31,
  power_adjust_enabled = 32,
  dcFlow_initialization = 33,
  parallel_residual_enabled = 34,
  parallel_jacobian_enabled = 35,
  parallel_contingency_enabled = 36,
  mpi_contingency_enabled = 37,
  first_run_limits_only = 38,
  no_reset = 39,
  voltage_constraints_flag = 40,
  record_on_halt_flag = 41,
  no_auto_slack_bus = 42,
  no_auto_disconnect = 43,
  single_step_mode = 44,
  dc_mode = 45,
  force_power_flow = 46,
  power_flow_only = 47,
  no_powerflow_adjustments = 48,
  save_power_flow_data = 49,
  no_powerflow_error_recovery = 50,
  dae_initialization_for_partitioned = 51,
};

//for the status flags bitset


//extra local flags
enum gd_extra_flags
{
  dcJacComp_flag = object_flag6,
  reset_voltage_flag = object_flag7,
  prev_setall_pqvlimit = object_flag8,
  invalid_state_flag = object_flag9,
  check_reset_voltage_flag = object_flag10,
  powerflow_saved = object_flag11,
  low_bus_voltage = object_flag12,
};




/** @brief helper structure for containing tolerances
*/
struct tolerances
{
  double voltageTolerance = 1e-6;  //!< tolerance on the voltage levels
  double angleTolerance = 1e-7;  //!< tolerance on the angles
  double defaultTolerance = 1e-6; //!< default tolerance on all other variables
  double toleranceRelaxationFactor = 1.0;  //!< relax the tolerances to help the solver
  double rtol = 1e-5;  //!< the relative tolerance
  double timeTol = kSmallTime; //!< the allowable time slop in events.  The time span below which the system doesn't really care about
};

class gridRecorder;
class gridEvent;

#define HANDLER_NO_RETURN (-500)

enum class contingency_mode_t;  //forward declare the enumeration
/** @brief the GridDyn Simulation Class
  the gridDynSimulation class contains the mechanics for generating solutions to various power systems problems of interest
*/
class gridDynSimulation : public gridSimulation
{
public:
  friend class powerFlowErrorRecovery;
  friend class dynamicInitialConditionRecovery;
  friend class faultResetRecovery;
  //!< define various contingency modes  [probably will be changed in the near future]
  
  //!< define an enumeration of the dynamic solver methods
  enum class dynamic_solver_methods
  {
    dae,
    partitioned,
    decoupled,
  };
  /** @brief enumeration of ordering schemes for variables*/
  enum class offset_ordering
  {
    mixed = 0,        //!< everything is mixed through each other
    grouped = 1,        //!< all similar variables are grouped together (angles, then voltage, then algebraic, then differential)
    algebraic_grouped = 2,       //!< all the algebraic variables are grouped, then the differential
    voltage_first = 3,       //!< grouped with the voltage coming first
    angle_first = 4,        //!< grouped with the angle coming first
    differential_first = 5,       //!< differential and algebraic grouped with differential first

  };
protected:
  //storageSpace for SUNDIALS solverInterface

  std::bitset<64> controlFlags;         //!< storage container for user settable flags
  // ---------------solution mode-------------
  const solverMode *defPowerFlowMode = &cPflowSolverMode;  //!< link to the default powerFlow mode
  const solverMode *defDAEMode = &cDaeSolverMode;   //!< link to the default DAE solver mode
  const solverMode *defDynAlgMode = &cDynAlgSolverMode;  //!< link to the default algebraic solver mode
  const solverMode *defDynDiffMode = &cDynDiffSolverMode;  //!< link to the default differential solver mode

  dynamic_solver_methods defaultDynamicSolverMethod = dynamic_solver_methods::dae;  //!< specifies which dynamic solver method to use if it is not otherwise specified.
  count_t max_Vadjust_iterations = 30;                  //!< maximum number of Voltage adjust iterations
  count_t max_Padjust_iterations = 15;                  //!< maximum number of Power adjust iterations
  count_t thread_count = 1;                                             //!< maximum thread count
  count_t haltCount = 0;                                                //!< counter for the number of times the solver was halted
  count_t residCount = 0;                                               //!< counter for the number of times the residual function was called
  count_t evalCount = 0;                                                //!< counter for the number of times the algUpdateFunction was called
  count_t JacobianCount = 0;                                    //!< counter for the number of calls to the Jacobian function
  count_t rootCount = 0;                                                //!< counter for the number of roots
  count_t busCount = 0;                                                 //!< counter for the number of buses
  count_t linkCount = 0;           //!<counter for the number of links
  gridDyn_time probeStepTime = 1e-3;                                  //!< initial step size
  double powerAdjustThreshold = 0.01;                   //!< tolerance on the power adjust step
  gridDyn_time powerFlowStartTime = kNullVal;                 //!< power flow start time  if nullval then it computes based on start time;
  struct tolerances tols;                                               //!< structure of the tolerances


  offset_ordering default_ordering = offset_ordering::mixed;    //!< the default_ordering scheme for state variables
  std::string powerFlowFile;                                    //!<the power flow output file if any
  std::vector < std::shared_ptr < solverInterface >> solverInterfaces;          //!< vector of solver data
  std::vector<const double *> extraStateInformation;				//!< a vector of additional state information for solveMode pairings
  std::vector<const double *> extraDerivInformation;			//!< a vector of additional derivative Information for solverMode pairings
  std::vector<gridObject *>singleStepObjects;  //!<objects which require a state update after time step
  std::vector<gridBus *> slkBusses;                             //!< vector of slack buses to aid in powerFlow adjust
  std::queue<gridDynAction> actionQueue;                //!< queue for actions for GridDyn to execute
  std::vector < std::shared_ptr < continuationSequence >> continList;  //!< set of continuation sequences to run
public:
  /** @ constructor to set the name
  @param[in] objName the name of the simulation*/
  explicit gridDynSimulation (const std::string &objName = "gridDynSim_#");

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  /** @brief set a particular instantiation of the simulation object to be the master for various purposes
   this function along with getInstance is used by external libraries to get particular information about the simulation
  without needing to store a copy of the simulation pointer
  @param[in] gds a pointer to the simulation intended to be the master*/
  static void setInstance (gridDynSimulation *gds);

  /** @brief get the master instance of a gridDynSimulation
  @return a pointer to the master gridDynSimulation object*/
  static gridDynSimulation* getInstance (void);

  // simulation
  /** @brief define an enumeration for the network check level*/
  enum class network_check_type : char
  {
    full,       //!< a full network check
    simplified        //!< a simplified version of the network check
  };

  /** @brief check the simulation network
   function does checks for connectivity and make sure each subnet has a slack bus attached to it
  @param[in] checkType  the type of network check to perform
  @return in indicating success (0) or failure (non-zero)
  */
  int checkNetwork (network_check_type checkType);        //function to do a check on the network and potentially reorder a few things and make sure it is solvable

  /** @brief perform a power flow calculation
@return in indicating success (0) or failure (non-zero)*/
  int powerflow ();



  /** @brief perform a sensitivity analysis
   this function will likely be changing as the sensitivity analysis is more developed
  @return in indicating success (0) or failure (non-zero)*/
  void pFlowSensitivityAnalysis ();

  /** @brief perform a continuation power flow analysis
  @param[in] contName the name of the continuation analysis to perform
  @detail this function will likely be changing as the continuation analysis develops further
  @return int indicating success (0) or failure (non-zero)*/
  void continuationPowerFlow (const std::string &contName);

  /** @brief function to get the system constraints
    constraints allowable are >0,  <0, >=0,  <=0
  this is not used very often
  @param[out] consData  the array to place the model constraint data
  @param[in] sMode the solver mode corresponding to array locations in consData
  */
  void getConstraints (double consData[], const solverMode &sMode) override;

  /** @brief check if the system has any constraints
  *@return true if there is constraints false if not
  */
  bool hasConstraints () const
  {
    return ((opFlags[has_constraints]) || (controlFlags[voltage_constraints_flag]));
  }

  /**@brief run the simulation until the specified time
  @param[in] t_end  the simulation time to stop defaults to the time given in system parameters
  @return int indicating success (0) or failure (non-zero)*/
  int run (gridDyn_time t_end = kNullVal) override;

  /**@brief initialize the simulation for power flow at the specified time
  @param[in] time0 the time of the initialization default to 0
  @return int indicating success (0) or failure (non-zero)*/
 int pFlowInitialize (gridDyn_time time0 = kNullVal);

  /**@brief step the simulation until the next event or stop point
  @param[in] t_end the maximum time to stop
  @param[out] timeActual the actual time that was achieved
  @return int indicating success (0) or failure (non-zero)*/
  int step (gridDyn_time t_end, gridDyn_time &timeActual);

  /**@brief step the simulation until the next event or stop point
  @param[out] timeActual the actual time that was achieved
  @return int indicating success (0) or failure (non-zero)*/
  int step (gridDyn_time &timeActual)
  {
    return step (currentTime + stepTime,timeActual);
  }

  /**@brief step the simulation until the next event or stop point
  @return int indicating success (0) or failure (non-zero)*/
  int step () override;

  /**@brief run powerflow in event driven mode,  evaluate the power flow at every given event or iteration time
  @param[in] t_end the stopping time for the simulation
  @param[in] t_step  the step size (the maximum time between powerflow evaluation is t_step
  @return int indicating success (0) or failure (non-zero)*/
  virtual int eventDrivenPowerflow (gridDyn_time t_end = kNullVal, gridDyn_time t_step = kNullVal);

  /** @brief execute a specific command
  *@param[in] cmd  the command to execute
  @return the return code from the execution (typically 0 upon success)
  */
  virtual int execute (const std::string &cmd);

  /** @brief execute a specific command from a griddynAction
  *@param[in] cmd  the command to execute as an action
  @return the return code from the execution (typically 0 upon success)
  */
  virtual int execute (const gridDynAction &cmd);

  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual std::string getString (const std::string &param) const override;
  virtual void setFlag (const std::string &flag, bool val = true) override;

  /** @brief get a vector of the states
  @param[in]  sMode the solverMode to get the states for
  @return a vector containing the states
  */
  std::vector<double> getState (const solverMode &sMode = cLocalSolverMode) const;
  double getState (index_t offset) const override;

  /** @brief get a particular state
  @param[in] offset the offset of the state to grab
  @param[in] sMode the solverMode corresponding to the state to grab
  @return the value of a particular state
  */
  double getState (index_t offset, const solverMode &sMode) const;

  //saving and loading data

  //function used in initialization
  /**@brief initialize the simulation for dynamic simulation at the specified time
  @param[in] tStart the time of the initialization default to 0
  @return int indicating success (0) or failure (non-zero)*/
  int dynInitialize (gridDyn_time tStart = kNullVal);   //code can detect this default param and use a previously specified start time
  void alert (gridCoreObject *object, int code) override;

  /** @brief function to count the number of MPI objects required for this simulation
  @param[in] printInfo if set to true the information is printed to the console
  @return the number of MPI objects
  */
  int countMpiObjects (bool printInfo = false) const;

  /** @brief get the number of non-zeros in the most recent Jacobian calculation
  @param[in] sMode the solverMode to get the number of non-zeros for
  @return the number of non-zero elements in the Jacobian
  */
  count_t nonZeros (const solverMode &sMode) const;

  /** @brief compute the network residuals
    computes a set of function for the power system such $r(\hat{x},\hat{x'})=f(x,x)- f(\hat{x},\hat{x}')$
  so that r approaches 0 as the $x$ == $\hat{x}
  @param[in] ttime  the simulation time of the evaluation
  @param[in] state  the state information to evaluation
  @param[in] dstate_dt  the time derivative of the state
  @param[out] resid the storage location for the residual function
  @param[in] sMode the solverMode to solve for
  @return integer indicating success (0) or failure (non-zero)
  */
  int residualFunction (double ttime, const double state[],const double dstate_dt[], double resid[], const solverMode &sMode);

  /** @brief compute the derivatives for all differential states
  @param[in] ttime  the simulation time of the evaluation
  @param[in] state  the state information to evaluation
  @param[out] dstate_dt  the time derivative of the state
  @param[in] sMode the solverMode to solve for
  @return integer indicating success (0) or failure (non-zero)
  */
  int derivativeFunction (double ttime, const double state[], double dstate_dt[], const solverMode &sMode);

  /** @brief compute an update to all algebraic states
   compute $x=f(\hat{x})$
  @param[in] ttime  the simulation time of the evaluation
  @param[in] state  the state information to evaluation
  @param[out] update  the updated state information
  @param[in] sMode the solverMode to solve for
  @param[in] alpha a multiplication factor for updates that are expected to be iterative
  @return integer indicating success (0) or failure (non-zero)
  */
  int algUpdateFunction (double ttime, const double state[], double update[], const solverMode &sMode, double alpha);

  /** @brief compute the Jacobian of the residuals
    computes $\frac{\partial r}{\partial x}$ for all components of the residual
  @param[in] ttime  the simulation time of the evaluation
  @param[in] state  the state information to evaluation
  @param[in] dstate_dt  the time derivative of the state
  @param[out] ad the matrixData object to store the Jacobian information into
  @param[in] cj the constant of integration for use in Jacobian elements using derivatives
  @param[in] sMode the solverMode to solve for
  @return integer indicating success (0) or failure (non-zero)
  */
  int jacobianFunction (double ttime, const double state[], const double dstate_dt[], matrixData<double> &ad, double cj, const solverMode &sMode);

  /** @brief compute any root values
    computes the roots for any root finding functions used in the system
  @param[in] ttime  the simulation time of the evaluation
  @param[in] state  the state information to evaluation
  @param[in] dstate_dt  the time derivative of the state
  @param[out] roots the storage location for the roots
  @param[in] sMode the solverMode to solve for
  @return integer indicating success (0) or failure (non-zero)
  */
  int rootFindingFunction (double ttime, const double state[], const double dstate_dt[], double roots[], const solverMode &sMode);


  /** @brief solve for the algebraic components of a system for use with the ode solvers
  @param[in] ttime  the simulation time of the evaluation
  @param[in] diffstate  the current derivative information
  @param[in] deriv the current derivative information
  @param[in] sMode the solverMode to solve related to the differential state information
  @return integer indicating success (0) or failure (non-zero)
  */
  int dynAlgebraicSolve(double ttime, const double diffstate[],const double deriv[], const solverMode &sMode);

  //solverMode and solverInterface search functions

  /** @brief get the solverMode dependent on a particular index of the solver data structure
  @param[in] index the index into the solverInterface storage array
  @return the solverMode named by the string or a blank one if none can be found
  */
  const solverMode &getSolverMode (index_t index) const;

  /** @brief get the solverMode named by a string
  @param[in] name the name of the solverInterface to get the solverMode for
  @return the solverMode named by the string or a blank one if none can be found
  */
  solverMode getSolverMode (const std::string &name);

  /** @brief get the solverInterface referenced by a particular index into the solverInterface array
@param[in] index the index into the solverInterface storage array
@return a shared pointer to a solverInterface
*/
  const std::shared_ptr<solverInterface> getSolverInterface (index_t index) const;

  /** @brief get a shared pointer to a solverInterface object
  @param[in] sMode the solver mode to get the residual information for
  @return a shared pointer to an solverInterface object
  */
  std::shared_ptr<const solverInterface> getSolverInterface (const solverMode &sMode) const;

  /** @brief get a shared pointer to a solverInterface object
   non-const version that can create new solverInterface objects if necessary
  @param[in] sMode the solver mode to get the residual information for
  @return a shared pointer to an solverInterface object
  */
  std::shared_ptr<solverInterface> getSolverInterface (const solverMode &sMode);

  /** @brief get the solverInterface referenced by name
  @param[in] name string representing the solverInterface name,  can be customized name or a particular type
  @return a shared pointer to a solverInterface
  */
  std::shared_ptr<solverInterface> getSolverInterface (const std::string &name);

  using gridSimulation::add;  //use the add functions from gridSimulation

  /** @brief  add a solverInterface object to the solverDat storage array
  @param[in] sd the solverInterface to add to the storage array
*/
  void add (std::shared_ptr<solverInterface> sd);

  /** @brief  add an action to the queue
  @param[in] actionString a string containing the action to add
  */
  void add (std::string actionString);

  /** @brief  add an action to the run queue
  @param[in] newAction the action to add to the queue
  */
  void add (gridDynAction &newAction);

  /** @brief enumeration of the solution modes of operation*/
  enum class solution_modes_t
  {
    powerflow_mode,        //!< mode for power flow solutions
    dae_mode,                           //!< mode for DAE solutions
    algebraic_mode,         //!< mode for algebraic solutions
    differential_mode,        //!< mode for differential equation solutions
  };

  /** @brief set the default solver for particular solution types to a specific solver
  @param[in] mode  the solution mode to set the solver For
  @param[in] sMode the solverMode corresponding to a particular solver
  */
  void setDefaultMode (solution_modes_t mode, const solverMode &sMode);

  /** @brief check if the simulation object has dynamic models
   basically checks if there are any differential states in the default solution mode
  */
  bool hasDynamics () const;

  /** @brief set a numeric parameter in a particular solver
   finds a solver by name then calls the solver set function
  @param[in] name  the name of the solver
  @param[in] field the field to set on the solver
  @param[in] val the value to the set the property to
  */
  virtual void solverSet (const std::string &name, const std::string &field, double val);

  /** @brief set a string parameter in a particular solver
   finds a solver by name then calls the solver set function
  @param[in] name  the name of the solver
  @param[in] field the field to set on the solver
  @param[in] val the value to the set the property to
  */
  virtual void solverSet (const std::string &name, const std::string &field, const std::string &val);

  /** @brief get the current solverMode from the simulation
  @param[in] sMode  input solverMode to check
  @return if sMode is valid it returns that if not it finds the current active mode and returns a reference to that
  */
  const solverMode &getCurrentMode (const solverMode &sMode = cEmptySolverMode) const;

  /** @brief makes sure the solverInterface object is ready to run solutions
  @param[in] solverInterfaces the solverData to check if it is ready
  */
  void getSolverReady (std::shared_ptr<solverInterface> &solverInterfaces);
  /** @brief load a stateData object with extra state information if necessary
  @param[in] sD the stateData object to load
  @param[in] sMode the solverMode of the state Data object
  */
  void fillExtraStateData (stateData *sD, const solverMode &sMode) const;
protected:
  /** @brief makes sure the specified mode has the correct offsets
  @param[in] sMode the solverMode of the offsets to check
  */
  void checkOffsets (const solverMode &sMode);

  /** @brief get and update a solverInterface object
   the difference here is that the object may not exist, in which case it is created and loaded with recent information
  from the models
  @param[in] sMode the solverMode to get the data for
  @return a shared pointer to the solverInterface object
  */
  std::shared_ptr<solverInterface> updateSolver (const solverMode &sMode);

  /** @brief get a pointer to a solverMode based on a string
  @param[in] type the string representing the solverMode, this can be a particular type of solverMode or the name of a solver
  @return the solverMode named by the string or a blank one if none can be found
  */
  const solverMode * getSolverModePtr (const std::string &type) const;

  /** @brief get a pointer to a solverMode dependent on a particular index of the solver data structure
  @param[in] index the index into the solverInterface storage array
  @return the solverMode named by the string or a blank one if none can be found
  */
  const solverMode * getSolverModePtr (index_t index) const;

  /** @brief reinitialize the power flow solver
  @param[in] sMode the solver Mode to reinitialize
  @param[in] change the adjustment mode
  */
  void reInitpFlow (const solverMode &sMode, change_code change = change_code::no_change);

  /** @brief perform a load balance operation on the power system
  @param[in] prevPower the previous total power output from slack bus generators
  @param[in] prevSlkGen the specifics of the power output from each slackbus
  @return true if any changes were made
  */
  virtual bool loadBalance (double prevPower,const std::vector<double> &prevSlkGen);

  /** @brief update the offsets associated with a particular mode
  @param[in] sMode the solver Mode to update the offsets for
  */
  virtual void updateOffsets (const solverMode &sMode);

  /** @brief make the simulator ready to perform a particular simulation
  @param[in] desiredState the desired state
  @param[in] sMode the solverMode to make the operations in
  @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  int makeReady (gridState_t desiredState, const solverMode &sMode);

  /** @brief set the maximum number of non-zeros in the Jacobian
  @param[in] sMode the solver mode to set the max number of non-zeros in the Jacobian
  @param[in] ssize the size to set
  */
  void setMaxNonZeros (const solverMode &sMode, count_t ssize);

  /** @brief reinitialize the dynamic simulation
  @param[in] sMode the solver mode to reinitialize
  @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  int reInitDyn (const solverMode &sMode);

  /** @brief execute a DAE simulation until the given stop time
  @param[in] tStop the stop time for the simulation
  @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  virtual int dynamicDAE (double tStop);

  /** @brief execute a partitioned dynamic simulation
  @param[in] tStop the stop time for the simulation
  @param[in] tStep the step interval (defaults to the step size parameter stored in the simulation
 @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  virtual int dynamicPartitioned (double tStop, double tStep = kNullVal);

  /** @brief execute a decoupled dynamic simulation
  @param[in] tStop the stop time for the simulation
  @param[in] tStep the step interval (defaults to the step size parameter stored in the simulation
 @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  virtual int dynamicDecoupled (double tStop, double tStep = kNullVal);

  /** @brief ensure that the simulation has consistent initial conditions for starting a dynamic simulation
  @param[in] sMode the solver mode for which to generate the initial conditions
  @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  int generateDaeDynamicInitialConditions (const solverMode &sMode);


  /** @brief generate a convergent partitioned solution
  @param[in] sModeAlg the solver mode of the algebraic solver
  @param[in] sModeDiff  the solver mode of the differential solver
 @return FUNCTION_EXECUTION_SUCCESS(0) if successful negative number if not
  */
  int generatePartitionedDynamicInitialConditions (const solverMode &sModeAlg, const solverMode &sModeDiff);



  virtual void setupOffsets (const solverMode &sMode, offset_ordering offsetOrdering);

  /** @brief function to help with IDA solving steps
  */
  void handleEarlySolverReturn (int retval, double timeReturn, std::shared_ptr<solverInterface> &dynData);

  /** @brief reset the dynamic simulation
   function checks for various conditions that cause specific things in the solver or simulation to be reset
  the nature of the reset can be driven by the reset_code given as an argument or internal flags from alerts or other mechanisms
  @param[in] sMode the solverMode to operate on
  @param[in] change the optional change code that the reset should function to, otherwise automatically detected
  @return true if the check did something, false if nothing has changed
  */
  bool dynamicCheckAndReset (const solverMode &sMode, change_code change = change_code::no_change);

  int handleStateChange (const solverMode &sMode);
  void handleRootChange (const solverMode &sMode, std::shared_ptr<solverInterface> &dynData);

  int checkAlgebraicRoots (std::shared_ptr<solverInterface> &dynData);
  /** @brief checks events for events needing to run and runs them then checks if a reset is needed if so it does so
  @param[in] cTime the time to run the events
  @param[in] sMode the solverMode to run
  @return true if the reset Function was run and did something
  */
  bool checkEventsForDynamicReset (double cTime, const solverMode &sMode);


private:
  void setupDynamicDAE ();
  void setupDynamicPartitioned ();

  int dynamicDAEStartupConditions (std::shared_ptr<solverInterface> &dynData, const solverMode &sMode);
  int dynamicPartitionedStartupConditions (std::shared_ptr<solverInterface> &dynDataDiff, std::shared_ptr<solverInterface> &dynDataAlg, const solverMode &sModeDiff, const solverMode &sModeAlg);
  int runDynamicSolverStep (std::shared_ptr<solverInterface> &dynDataDiff, double nextStop, double &timeReturn);

  static gridDynSimulation* s_instance;        //!< static variable to set the master simulation instance
};




#endif

