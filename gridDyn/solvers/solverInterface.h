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

#ifndef _SOLVER_INTERFACE_H_
#define _SOLVER_INTERFACE_H_

#include "gridObjectsHelperClasses.h"

#include <vector>
#include <memory>


enum class solver_print_level
{
  s_debug_print = 2, s_error_log = 1, s_error_trap = 0,
};

class gridDynSimulation;

#define SOLVER_ROOT_FOUND 2
#define SOLVER_INVALID_STATE_ERROR (-36)
#define SOLVER_INITIAL_SETUP_ERROR (-38)
/** @brief class defining the data related to a specific solver
 the solverInterface class is the base class for solvers for the griddyn power systems program
a particular solverInterface class will contain the inferace and calls necessary to implement a particular solver methodology
*/
class solverInterface
{
public:
  /** @brief enumeration of solver call modes*/
  enum class step_mode
  {
    normal,                                                   //!< normal operation
    single_step,                                      //!< single step operation
  };
  /** @brief enumeration of initiaL condition call modes*/
  enum class ic_modes
  {
    fixed_masked_and_deriv,                            //!< fixed_algebraic and differential state derivatives
    fixed_diff,                                      //!< differential states are fixed
  };
  /** @brief enumeration of initiaL condition call modes*/
  enum class sparse_reinit_modes
  {
    resize,                                  //!< destroy and completely reinit the sparse calculations
    refactor,                                //!< refactor the sparse matrix
  };
  std::string name;                        //!< nickname for the solver
  std::vector<int> rootsfound;            //!< mask vector for which roots were found
  bool printResid = false;                 //!< flag telling the interface to print the residual values to the screen (used for debugging)
protected:
	
  std::string lastErrorString = "";             //!< string containing the last error
  int lastErrorCode = 0;                        //!< the last error Code

  // solver outputs

  std::vector<index_t> maskElements;                                           //!< vector of constant states in any problem
  std::string solverLogFile;                                                          //!< file name and location of log file reference
  solver_print_level printLevel = solver_print_level::s_error_trap;            //!< print_level for solver
  count_t rootCount = 0;                                                                        //!< the number of root finding functions
  count_t solverCallCount = 0;                                                          //!< the number of times the solver has been called
  count_t jacCallCount = 0;                                                                     //!< the number of times the Jacobian function has been called
  count_t funcCallCount = 0;											//!< the number of times the function evaluation has been called
  count_t rootCallCount = 0;
  count_t max_iterations = 10000;                                    //!< the maximum number of iterations in the solver loop
  solverMode mode;                                                        //!< to the solverMode
  double tolerance = 1e-8;												//!<the default solver tolerance
  bool dense = false;													//!< if the solver should use a dense or sparse version
  bool constantJacobian = false;										//!< if the solver should just keep a constant Jacobian
  bool useMask = false;                                                                         //!< if the solver should use a mask to filter out specific states
  bool parallel = false;                                                                        //!< if the solver should use a parallel version
  bool locked = false;                                                                          //!< if the solverMode is locked from further updates
  bool use_omp = false;                                     //!<flag indicating whether to use omp data contructs

  bool allocated = false;                                                                       //!< if the solver has been allocated
  bool initialized = false;                                                 //!< flag indicating if these vectors have been initialized
  void *solverMem = nullptr;                                                            //!< the memory used by a specific solver internnally
  gridDynSimulation *m_gds = nullptr;                                           //!< pointer the gridDynSimulation object used
  count_t svsize = 0;                                                                           //!< the state size
  count_t nnz = 0;                                                                           //!< the actual number of non-zeros in a Jacobian

public:
  /** @brief default constructor*/
  solverInterface ();
  /** @brief alternate constructor
  @param[in] gds  gridDynSimulation to link with
  @param[in] sMode the solverMode associated with the solver
  */
  solverInterface (gridDynSimulation *gds, const solverMode& sMode);

  /** @brief destructor*/
  virtual ~solverInterface ();
  /** @brief make a copy of the solver interface
  @param[in] si a shared ptr to an existing interface that data should be copied to
  @param[in] fullCopy set to true to initialize and copy over all data to the new object
  @return a shared ptr to the clones solverInterface
  */
  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si=nullptr, bool fullCopy=false) const;
  /** @brief get a pointer to the state data
  @return a pointer to a double array with the state data
  */
  virtual double * state_data ();

  /** @brief get a pointer to the state time derivative information
  @return a pointer to a double array with the state time derivative information
  */
  virtual double * deriv_data ();

  /** @brief get a pointer to the type data
  @return a pointer to a double array containing the type data
  */
  virtual double * type_data ();

  /** @brief get a pointer to the const state data
  @return a pointer to a const double array with the state data
  */
  virtual const double * state_data() const;

  /** @brief get a pointer to the const state time derivative information
  @return a pointer to a const double array with the state time derivative information
  */
  virtual const double * deriv_data() const;

  /** @brief get a pointer to the const type data
  @return a pointer to a const double array containing the type data
  */
  virtual const double * type_data() const;

  /** @brief allocate the memory for the solver
  @param[in] size  the size of the state vector
  @param[in] numroots  the number of root functions in the solution
  @return the function status
  */
  virtual int allocate (count_t size, count_t numroots = 0);

  /** @brief initialize the solver to time t0
  @param[in] t0  the time for the initialization
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int initialize (double t0);

  /** @brief reinitialize the sparse components
  @param[in] mode the reinitialization mode
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int sparseReInit (sparse_reinit_modes mode);

  /** @brief load the constraints*/
  virtual void setConstraints ();

  /** @brief perform an initial condition calculation
  @param[in] t0  the time for the initialization
  @param[in]  tstep0  the size of the first desired step
  @parma[in] mode  the step mode
  @param[in] constraints  flag indicating that constraints should be used
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int calcIC (double t0, double tstep0, ic_modes mode, bool constraints);
  /** @brief get the current solution
   usually called after a call to CalcIC to get the calculated conditions
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int getCurrentData ();
  /** @brief get the locations of any found roots
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int getRoots ();
  /** @brief update the number of roots to find
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int setRootFinding (index_t numRoots);
  
	/** @brief get a parameter from the solver
  @param[in] param  a string with the desired name of the parameter or result
  @return the value of the requested parameter
  */
  virtual double get (const std::string & param) const;
  /** @brief set a string parameter in the solver
  @param[in] param  a string with the desired name of the parameter
  @param[in] val the value of the property to set
  @return a value indicating success  PARAMETER_FOUND if param was a valid parameter,  PARAMETER_NOT_FOUND if invalid,  INVALID_PARAMETER_VALUE if bad value
  */
  virtual int set (const std::string &param, const std::string &val);
  
	/** @brief set a numerical parameter on a solver
  @param[in] param  a string with the desired name of the parameter
  @param[in] val the value of the property to set
  @return a value indicating success  PARAMETER_FOUND if param was a valid parameter,  PARAMETER_NOT_FOUND if invalid,  INVALID_PARAMETER_VALUE if bad value
  */
  virtual int set (const std::string &param, double val);
  
	/** get the name of the solver*/
	const std::string &getName() const
	{
		return name;
	}
	/** set the name of the solver*/
	void setName(std::string newName)
	{
		name = newName;
	}
	/** @brief perform the solver calculations
  @param[in] tStop  the requested return time   not that useful for algebraic solvers
  @param[out]  tReturn  the actual return time
  @parma[in] mode  the step mode
  @return the function success status  FUNCTION_EXECUTION_SUCCESS on success
  */
  virtual int solve (double tStop, double & tReturn, step_mode stepMode = step_mode::normal);
  /** @brief resize the storage array for the Jacobian
  @param[in] size  the number of elements to potentially store
  */
  virtual void setMaxNonZeros (count_t size);
  /** @brief check if the solverInterface has been initialized
  @return true if initialized false if not
  */
  bool isInitialized () const
  {
    return initialized;
  }
  /** @brief helper function to log specific solver stats
  @param[in] logLevel  the level of logging to display
  @param[in] iconly  flag indicating that the logging should be for the initial condition calculation only
  */
  virtual void logSolverStats (int logLevel, bool iconly = false) const;
  /** @brief helper function to log error weight information
  @param[in] logLevel  the level of logging to display
  */
  virtual void logErrorWeights (int logLevel) const;

  /** @brief get the dedicated memory space of the solver
  @return a void pointer to the memory location of the solver specific memory
  */
  void * getSolverMem () const
  {
    return solverMem;
  }
  /** @brief get the state size
  @return the state size
  */
  count_t size () const
  {
    return svsize;
  }

  /** @brief get the actual number of non-zeros in the Jacobian
  @return the state size
  */
  count_t nonZeros () const
  {
    return nnz;
  }

  const solverMode &getSolverMode () const
  {
    return mode;
  }

  void lock ()
  {
    locked = true;
  }

  void setIndex (index_t newIndex)
  {
    mode.offsetIndex = newIndex;
  }
  /** @brief print out all the state values
  @param[in] getNames use the actual state names vs some coding
  */
  void printStates (bool getNames = false);
  /** @brief input the simulation data to attach to
  @param[in] gds the gridDynSimulationObject to attach to
  @param[in] sMode the solverMode associated with the solver
  */
  virtual void setSimulationData (gridDynSimulation *gds, const solverMode& sMode);
  /** @brief input the simulation data to attach to
  @param[in] gds the gridDynSimulationObject to attach to
  */
  virtual void setSimulationData (gridDynSimulation *gds);

  /** @brief input the solverMode associated with the solver
  @param[in] sMode the solverMode to attach to
  */
  virtual void setSimulationData (const solverMode& sMode);

  /** @brief check the output of actual solver calls for proper results
  @param[in] flagvalue ???
  @param[in] funcname  the name of the function that we are checking
  @param[in] opt  ????
  @param[in] printError  boolean flag indicating whether to print a message on error or not
  */
  virtual int check_flag (void *flagvalue, const std::string &funcname, int opt, bool printError = true) const;

  /** @brief load up masks to the states
    masks isolate specific values and don't let the solver alter them  for newton based solvers this implies overriding specific information in the Jacobian calculations and the residual calculations
  @param[in] msk  the indices of the state elements to fix
  */
  void setMaskElements (std::vector<index_t> msk);

  /** @brief add an index to the mask
    masks isolate specific values and don't let the solver alter them  for newton based solvers this implies overriding specific information in the Jacobian calculations and the residual calculations
  @param[in] newMaskElement the index of the values to mask
  */
  void addMaskElement (index_t newMaskElement);

  /** @brief add several new elements to a mask
    masks isolate specific values and don't let the solver alter them  for newton based solvers this implies overriding specific information in the Jacobian calculations and the residual calculations
  @param[in] newMsk  a vector of indices to add to an existing mask
  */
  void addMaskElements (std::vector<index_t> newMsk);

  void logMessage (int errorCode, std::string message);

  int getLastError () const
  {
    return lastErrorCode;
  }
  std::string getLastErrorString () const
  {
    return lastErrorString;
  }
};

/** @brief class implementing a Gauss Seidel solver for algebraic variables in a power system
*/
class basicSolver : public solverInterface
{
private:
  std::vector<double> state; //!< state data/
  std::vector<double> tempState1;  //!< temp state data location 1
  std::vector<double> tempState2;  //!< temp state data location 2
  std::vector<double> type;                     //!< type data
  double alpha = 1.1;                                             //!< convergence gain;
  /** @brief enumeration listing the algorithm types*/
  enum class mode_t
  {
    gauss, gauss_siedel
  };
  mode_t algorithm = mode_t::gauss;  //!< the algorithm to use
  count_t iterations=0;   //!< counter for the number of iterations
public:
  /** @brief default constructor*/
  basicSolver ();
  /** alternate constructor to feed to solverInterface
  @param[in] gds  the gridDynSimulation to link to
  @param[in] sMode the solverMode to solve with
  */
  basicSolver (gridDynSimulation *gds, const solverMode& sMode);

  virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
  double * state_data () override;
  double * deriv_data () override;
  double * type_data () override;

  const double * state_data() const override;
  const double * deriv_data() const override;
  const double * type_data() const override;
  int allocate (count_t size, count_t numroots = 0) override;
  int initialize (double t0) override;

  virtual double get (const std::string & param) const override;
  virtual int set (const std::string &param, const std::string &val) override;
  virtual int set (const std::string &param, double val) override;

  virtual int solve (double tStop, double & tReturn, step_mode stepMode = step_mode::normal) override;
};

/** @brief class implementing a Gauss Seidel solver for algebraic variables in a power system
*/
class basicOdeSolver : public solverInterface
{
private:
	std::vector<double> state; //!< state data/
	std::vector<double> deriv;  //!< temp state data location 1
	std::vector<double> state2;  //!< temp state data location 2
	std::vector<double> type;                     //!< type data
	double deltaT = 0.005;  //!< the default time step
	double solveTime = 0; //!< the last solve Time
public:
	/** @brief default constructor*/
	basicOdeSolver();
	/** alternate constructor to feed to solverInterface
	@param[in] gds  the gridDynSimulation to link to
	@param[in] sMode the solverMode to solve with
	*/
	basicOdeSolver(gridDynSimulation *gds, const solverMode& sMode);

	virtual std::shared_ptr<solverInterface> clone(std::shared_ptr<solverInterface> si = nullptr, bool fullCopy = false) const override;
	double * state_data() override;
	double * deriv_data() override;
	double * type_data() override;

	const double * state_data() const override;
	const double * deriv_data() const override;
	const double * type_data() const override;
	int allocate(count_t size, count_t numroots = 0) override;
	int initialize(double t0) override;

	virtual double get(const std::string & param) const override;
	virtual int set(const std::string &param, const std::string &val) override;
	virtual int set(const std::string &param, double val) override;

	virtual int solve(double tStop, double & tReturn, step_mode stepMode = step_mode::normal) override;
};

/** @brief make a solver from a particular mode
@param[in] gds  the gridDynSimulation to link to
@param[in] sMode the solverMode to construct the solverInterface from
@return a shared_ptr to a solverInterface object
*/
std::shared_ptr<solverInterface> makeSolver (gridDynSimulation *gds, const solverMode &sMode);
/** @brief make a solver from a string
@param[in] type the type of solverInterface to create
@return a shared_ptr to a solverInterface object
*/
std::shared_ptr<solverInterface> makeSolver (const std::string &type);


#endif
