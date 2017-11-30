/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

 /** @file
 @brief main file defining the C API to GridDyn
 @details the function defined in this file make up the C api for gridDyn
 */

/** @defgroup GridDyn_api the group of function defining the C api to gridDyn
*  @{
*/
#ifndef GRIDDYN_EXPORT_C_H_
#define GRIDDYN_EXPORT_C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/*
Export GRIDDYN API functions on Windows and under GCC.
If custom linking is desired then the GRIDDYN_Export must be
defined before including this file. For instance,
it may be set to __declspec(dllimport).
*/
#if !defined(GRIDDYN_Export) 
#if defined _WIN32 || defined __CYGWIN__
#if defined(BUILD_DLL)
#define GRIDDYN_Export __declspec(dllexport)
#else // BUILD_DLL
#define GRIDDYN_Export __declspec(dllimport)
#endif //BUILD_DLL
#else // defined _WIN32 || defined __CYGWIN__ 
// Alwasy defined on Linux
#define GRIDDYN_Export __attribute__ ((visibility ("default")))
#endif // defined _WIN32 || defined __CYGWIN__
#endif ////defined(GRIDDYN_Export)


/** typedef a gridDynObject to a void * to represent an object in GridDyn*/
typedef void *gridDynObject;

/** typedef a gridDynSimReference to a void * to represent a simulation object*/
typedef void *gridDynSimReference;

/** typedef a gridDynSingleQuery to a void * to represent a query for a single data point*/
typedef void *gridDynSingleQuery;

/** typedef a gridDynVectorQuery to a void * to represent a collector for multiple data points*/
typedef void *gridDynVectorQuery;

/** typedef a gridDynEvent to a void * to represent an event in GridDyn*/
typedef void *gridDynEvent;

/** typedef * solverKey to a void * to represent a solverMode object for use in a number of functions*/
typedef void *solverKey;

/** defined error codes*/
/** the function executed successfully*/
#define EXECUTION_SUCCESS (0)
/** the call used an invalid object*/
#define INVALID_OBJECT (-24)
/** an invalid parameter value was used in the call*/
#define INVALID_PARAMETER_VALUE (-25)
/** the parameter requested was unknown*/
#define UNKNOWN_PARAMETER (-26)
/** the add failed*/
#define ADD_FAILURE (-27)
/** the remove operation failed*/
#define REMOVE_FAILURE (-28)
/** the field failed to result in a functional query object*/
#define QUERY_LOAD_FAILURE (-33)
/** the file operation failed*/
#define FILE_LOAD_FAILURE (-36)
/** the solver had an error*/
#define SOLVE_ERROR (-43)
/** the object was not in the appropriate initialization state*/
#define OBJECT_NOT_INITIALIZED (-54)
/** the call was not valid*/
#define INVALID_FUNCTION_CALL (-60)

// definitions for GRIDDYN_STATUS
#ifndef GRIDDYN_PENDING 
#define GRIDDYN_PENDING (25)
#endif

#define GRIDDYN_COMPLETE (30)

// the object hiearchary interface and object properties interface

/** create an object in GridDyn
@details the function creates a an owning pointer to the object, as long as this owning pointer exists
the object exists,  if the object is added to the hiearchacy and remains in operation the object will continue to
exist even if the gridDynObject representing it is freed
@param[in] componentType  the component type of the object to create (bus, link, load, etc)
@param[in] objectType  the specific type of the component to create
@return a gridDynObject that represents the newly created object
*/
GRIDDYN_Export gridDynObject gridDynObject_create (const char *componentType, const char *objectType);

/** clone an object from an existing object
@details a new object is constructed that is a clone of the original object
@param[in] obj the original object
@return a gridDynObject that represents the newly created object
*/
GRIDDYN_Export gridDynObject gridDynObject_clone(const gridDynObject obj);

/** free the object,  which may result in object destruction if it is not being used elsewhere
@param[in] obj the object to free
*/
GRIDDYN_Export void gridDynObject_free (gridDynObject obj);

/** add an object to another in the hiearchy
@param[in] parentObject the object to which another is being added
@param[in] objectToAdd  the object being added
@return 0 if successful, non-zero otherwise
*/
GRIDDYN_Export int gridDynObject_add (gridDynObject parentObject, gridDynObject objectToAdd);

/** remove an object from another in the hiearchy
@param[in] parentObject the object to which another is being added
@param[in] objectToRemove  the object being removed
@return 0 if successful, non-zero otherwise success is defined as the object not being present in the parent
*/
GRIDDYN_Export int gridDynObject_remove (gridDynObject parentObject, gridDynObject objectToRemove);

/** set a string parameter in an object
@param[in] obj the object to set the property of
@param[in] parameter the parameter to set
@param[in] value the desired value of the parameter
@return 0 on success, negative otherwise
*/
GRIDDYN_Export int gridDynObject_setString (gridDynObject obj, const char *parameter, const char *value);

/** set a value parameter in an object
@details the units are either not needed or assumed to be the default units often pu
@param[in] obj the object to set the property of
@param[in] parameter the parameter to set
@param[in] value the desired value of the parameter
@return 0 on success, negative otherwise
*/
GRIDDYN_Export int
gridDynObject_setValue(gridDynObject obj, const char *parameter, double value);

/** set a value parameter in an object
@param[in] obj the object to set the property of
@param[in] parameter the parameter to set
@param[in] value the desired value of the parameter
@param[in] units a description of the units which correspond to the parameter
@return 0 on success, negative otherwise
*/
GRIDDYN_Export int
gridDynObject_setValueUnits (gridDynObject obj, const char *parameter, double value, const char *units);

/** set a string parameter in an object
@param[in] obj the object to set the property of
@param[in] parameter the parameter to set
@param[in] value the desired value of the parameter
@return 0 on success, negative otherwise
*/
GRIDDYN_Export int gridDynObject_setFlag (gridDynObject obj, const char *flag, int val);

/** query the value of a string in an object
@param[in] obj the object to query
@param[in] parameter the name of the parameter to query
@param[out] value the storage location for the string
@param[in] N the max size of the string
@return the size of the string returned in value*/
GRIDDYN_Export int gridDynObject_getString (const gridDynObject obj, const char *parameter, char *value, int N);

/** query the value of an object parameter
@param[in] obj the object to query
@param[in] parameter the name of the parameter to query
@param[out] result the location to store the result
@return 0 if the value is valid or UNKNOWN_PARAMETER Otherwise*/
GRIDDYN_Export int
gridDynObject_getValue (const gridDynObject obj, const char *parameter, double *result);

/** query the value of an object parameter
@param[in] obj the object to query
@param[in] parameter the name of the parameter to query
@param[in] units the desired output units
@param[out] result the location to store the result
@return 0 if the value is valid or UNKNOWN_PARAMETER Otherwise*/
GRIDDYN_Export int
gridDynObject_getValueUnits(const gridDynObject obj, const char *parameter, const char *units, double *result);

/** query the value of an object parameter
@param[in] obj the object to query
@param[in] parameter the name of the parameter to query
@param[out] result the the value of the flag 0 if false otherwise(usually 1) if true
@return 0 if the flag is found UNKNOWN_PARAMETER Otherwise*/
GRIDDYN_Export int gridDynObject_getFlag (const gridDynObject obj, const char *flag, int *result);

/** find an object within another object
@param[in] obj the object as the basis of the search
@param[in] objectToFind a string describing the object
@return a gridDynObject with the found object,  nullptr if not found*/
GRIDDYN_Export gridDynObject gridDynObject_find (const gridDynObject obj, const char *objectToFind);

/** get a subobject of a particular object by type and index
@param[in] obj the object to use as the basis for the search
@param[in] componentType the type of component to search
@param[in] N  the index of the object to retrieve
@return a gridDynObject as indicated by the index or a nullptr
*/
GRIDDYN_Export gridDynObject gridDynObject_getSubObject (const gridDynObject obj, const char *componentType, int N);

/** get a subobject of a particular object by type and index
@param[in] obj the object to use as the basis for the search
@param[in] componentType the type of component to search
@param[in] ID  the user identifier for the object
@return a gridDynObject as indicated by the user ID
*/
GRIDDYN_Export gridDynObject gridDynObject_findByUserId (const gridDynObject obj, const char *componentType, int ID);
/** get the parent of an object
@param[in] obj the object to get the parent of
@return the parent of the object in question or a nullptr if the object is a root object
*/
GRIDDYN_Export gridDynObject gridDynObject_getParent (const gridDynObject obj);
/** get a const char * to a string representing the componentType of the object
@param[in] obj the object to query
@return a pointer to a type string.  The memory is managed internally and does not need to be freed the pointers
are to a fixed set of strings and should not be modified*/
GRIDDYN_Export const char *gridDynObject_getType (const gridDynObject obj);

// functions for the gridDyn Simulation
/** create a new simulation object
@param[in] type the type of simulation to create (can be "" for the standard type
@param[in] name  the name of the simulation to ceate
@return a reference to the simulation object
*/
GRIDDYN_Export gridDynSimReference gridDynSimulation_create (const char *type, const char *name);

/** free the simulation memory
@param[in] sim the simulation runner reference object
*/
GRIDDYN_Export void gridDynSimulation_free (gridDynSimReference sim);

/** free the simulation memory
@param[in] sim the simulation runner reference object
@param[in] initializationString
@return error code if not successful
*/
GRIDDYN_Export int
gridDynSimulation_initializeFromString (gridDynSimReference sim, const char *initializationString);

/** free the simulation memory
@param[in] sim the simulation runner reference object
@param[in] argc  the argument count from command line arguments
@param[in] argv the argument values from the command line arguments
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int
gridDynSimulation_initializeFromArgs (gridDynSimReference sim, int argc, char *argv[], int ignoreUnrecognized);

/** load a simulation file or add a file to the existing simulation
@param[in] sim the simulation runner reference object
@param[in] fileName the name of the file to load
@param[in] fileType the type of the file set to null string to enable automatic detection based on the extension
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int
gridDynSimulation_loadfile (gridDynSimReference sim, const char *fileName, const char *fileType);

/** add a command to the GridDyn Command queue
@details this is one of the main interfaces to get GridDyn to do all sorts of things and change how it does it
this includes scripting and execution order once a command queue is set up the run command will execute the command
queue
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_addCommand (gridDynSimReference sim, const char *command);

/** initialize the simulation so it is ready to execute a power flow
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_powerflowInitialize (gridDynSimReference sim);

/** run a power flow calculation on the current simulation
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_powerflow (gridDynSimReference sim);

/** initialize the simulation so it is ready to execute a dynamic simulation
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_dynamicInitialize (gridDynSimReference sim);

/** reset the simulation to time 0
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_reset (gridDynSimReference sim);

/** get the currernt simulation time
@param sim the simulation runner reference object
@return the simulation time in seconds
*/
GRIDDYN_Export double gridDynSimulation_getCurrentTime(gridDynSimReference sim);
/** run the command queue of the simulation
@details if the command queue is empty it will try to run a dynamic simulation if the models are capable of that
otherwise it runs a power flow and stops
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_run (gridDynSimReference sim);

/** run the simulation to a particular time
@param[in] sim the simulation runner reference object
@param[in] runToTime the time to execute the simulator to
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_runTo (gridDynSimReference sim, double runToTime);

/** step the simulator one event step
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_Step (gridDynSimReference sim);

/** run the command queue of the simulation asynchronously
@details if the command queue is empty it will try to run a dynamic simulation if the models are capable of that
otherwise it runs a power flow and stops this function will return immediately
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_runAsync(gridDynSimReference sim);

/** run the simulation to a particular time but return and run aynchronously
@param[in] sim the simulation runner reference object
@param[in] runToTime the time to execute the simulator to
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_runToAsync(gridDynSimReference sim, double runToTime);

/** step the simulator one event step
@param[in] sim the simulation runner reference object
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_StepAsync(gridDynSimReference sim);
/** query the status of a simulation
@param[in] sim the simulation runner reference object
@return will return GRIDDYN_PENDING if an Async operation is ongoing, otherwise will return the execution state
*/
GRIDDYN_Export int gridDynSimulation_getStatus(gridDynSimReference sim);
/** get an object reference for the simulation object that can be used in the gridDynDbject functions
@param[in] sim the simulation runner reference object
@return a gridDynObject that can be used in the gridDynObject_* functions
*/
GRIDDYN_Export gridDynObject getSimulationObject (gridDynSimReference sim);

// mathematical function for using the simulation as a function evaluator
/** get a solverKey value that can be used for the solverKey input in the other math functions
@param[in] sim the simulation runner reference object
@param[in] solverType  the type or name of a solver used in GridDyn
@return a solverKey object used in functions that require a solverKey
*/
GRIDDYN_Export solverKey gridDynSimulation_getSolverKey (gridDynSimReference sim, const char *solverType);

/** free a solver key
@param[in] key the solver key to be freed that is no longer needed
*/
GRIDDYN_Export void gridDynSolverKey_free (solverKey key);

/** get the simulation state size
@param[in] sim the simulation runner reference object
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return  the state size of the system if <=0 an error was encountered
*/
GRIDDYN_Export int gridDynSimulation_stateSize (gridDynSimReference sim, solverKey key);

/** get the number of buses in the simulation
@param[in] sim the simulation runner reference object
@return  the number of buses in the simulation
*/
GRIDDYN_Export int gridDynSimulation_busCount(gridDynSimReference sim);
/** get the number of lines in the simulation
@param[in] sim the simulation runner reference object
@return  the number of lines in the simulation
*/
GRIDDYN_Export int gridDynSimulation_lineCount(gridDynSimReference sim);
/** get a set of data from the simulation
@param[in] sim the simulation runner reference object
@param datatype a string defining what data to retreive from the simulation
@param[out] data memory for storing the requested results
@param maxSize the size of the memory location specified by voltages
@return  the number of points written
*/
GRIDDYN_Export int gridDynSimulation_getResults(gridDynSimReference sim, const char *datatype, double *data, int maxSize);

/** have the simulation guess at all the state variables
@param[in] sim the simulation runner reference object
@param[in] time the simulation time to guess the state for
@param[out] states mamory to store the states must be at least the size returned by ::gridDynSimulation_stateSize
@param[out] dstate_dt memory to store the guess of the derivatives if the solver key points to a solver mode with
differential variables must be the same size as ::states
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_guessState (gridDynSimReference sim,
                                                 double time,
                                                 double *states,
                                                 double *dstate_dt,
                                                 solverKey key);

/** set the system state,  store state variables into the simulation objects
@param[in] sim the simulation runner reference object
@param[in] time the time relates to the states
@param[in] states the state variables of the system must be of the size returned by ::gridDynSimulation_stateSize
@param[in] dstate_dt the time derivatives of the states can be nullptr if the solverkey references does not have
differential states
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_setState (gridDynSimReference sim,
                                               double time,
                                               const double *states,
                                               const double *dstate_dt,
                                               solverKey key);

/** get the types(algebraic or differential) of all the variables in the simulation voltages and angle variables
are algebraic
@param[in] sim the simulation runner reference object
@param[out] types the types of the system 0 for differential variables 1 for algebraic
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_getStateVariableTypes (gridDynSimReference sim, double *types, solverKey key);

/** compute the residual values for all states in the system
@details the residual function will result in all very small values if the states are self consistent
f(x,x')=0 if everything is correct
@param[in] sim the simulation runner reference object
@param[in] time the simulation to get the residual for
@param[out] resid the memory location to store the results of the calculation
@param[in] states the state data to use in computing the residual
@param[in] dstate_dt the derivative data to use in computing the residual
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_residual (gridDynSimReference sim,
                                               double time,
                                               double *resid,
                                               const double *states,
                                               const double *dstate_dt,
                                               solverKey key);

/** compute the derivatives for all differential variables in the system
@param[in] sim the simulation runner reference object
@param[in] time the time to compute the derivative for
@param[out] deriv the memory storage to store the derivative
@param[in] states the state data to use in the computation
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_derivative (gridDynSimReference sim,
                                                 double time,
                                                 double *deriv,
                                                 const double *states,
                                                 solverKey key);

/** compute an algebraic update for all algebraic variables in a system
@param[in] sim the simulation runner reference object
@param[out] update the memory location to store all the updates to the algebraic variables
@param[in] states the state variables to use in computing the update
@param[in] alpha a update scaling parameter used in some cases where the update must be computed iteratively
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_algebraicUpdate (gridDynSimReference sim,
                                                      double time,
                                                      double *update,
                                                      const double *states,
                                                      double alpha,
                                                      solverKey key);

/** generate a system jacobian matrix
@param[in] sim the simulation runner reference object
@param[in] time the time to use in the jacobian computation
@param[in] states the state variable values to use in the computation
@param[in] dstate_dt the derivative information to use in the computation of the system jacobian
@param[in] cj the constant of differentiation.  this is the number to use when the jacobian depends on the
derivative of state
@param[in] key the index of the solver to use (the value should be retrieved by ::gridDynSimulation_getSolverKey)
@param[in] insert a function pointer to a callback that takes a row, a column and a value to define values on a
sparse matrix
@return 0 if successful an error code otherwise
*/
GRIDDYN_Export int gridDynSimulation_jacobian (gridDynSimReference sim,
                                               double time,
                                               const double *states,
                                               const double *dstate_dt,
                                               double cj,
                                               solverKey key,
                                               void (*insert) (int, int, double));
// functions for querying values
/** create a query object to retrieve a single value repeatedly
@details if just a single value is needed once use ::gridDynObject_getValue
@param[in] obj the object to retrieve the value from (can be nullptr in this case)
@param[in] queryString a string representing a value to query
@return a singleQueryObject
*/
GRIDDYN_Export gridDynSingleQuery gridDynSingleQuery_create (gridDynObject obj, const char *queryString);

/** create a query object to retrieve a set of values repeatedly
@details can be one or more values that can be retrieved to a vector
@param[in] obj the object to retrieve the value from (can be nullptr in this case)
@param[in] queryString a string representing a value to query
@return a vectorQueryObject
*/
GRIDDYN_Export gridDynVectorQuery gridDynVectorQuery_create (gridDynObject obj, const char *queryString);

/** release the memory from a single query object
@param[in] query the query object to free
*/
GRIDDYN_Export void gridDynSingleQuery_free (gridDynSingleQuery query);

/** release the memory from a vector query object
@param[in] query the query object to free
*/
GRIDDYN_Export void gridDynVectorQuery_free (gridDynVectorQuery query);

/** get the result from a single query
@param[in] query the single query to run
@return the value retrieved by the query
*/
GRIDDYN_Export double gridDynSingleQuery_run (gridDynSingleQuery query);

/** get the results from a Vector query
@param[in] query the vector query to run
@param[out] data the memory location to store the data
@param[in] N the maximum size of the memory location
@return the value retrieved by the query
*/
GRIDDYN_Export int gridDynVectorQuery_run (gridDynVectorQuery query, double *data, int N);

/** add additional measurements to a vector query
@param[in] query the vector query object to append measurements
@param[in] obj the object to get the measurement from
@param[in] queryString a string describing the new queries to add
*/
GRIDDYN_Export int
gridDynVectorQuery_append (gridDynVectorQuery query, gridDynObject obj, const char *queryString);
/** change the data a single query gets
@param[in] query the single query object to alter
@param[in] obj the object to get the measurement from
@param[in] queryString a string describing the new queries to add
*/
GRIDDYN_Export int
gridDynSingleQuery_update (gridDynSingleQuery query, gridDynObject obj, const char *queryString);

/** change the data a vector query gets
@param[in] query the vector query object to alter
@param[in] obj the object to get the measurement from
@param[in] queryString a string describing the new queries
*/ GRIDDYN_Export int
gridDynVectorQuery_update (gridDynVectorQuery query, gridDynObject obj, const char *queryString);

// functions for handling events

/** create an event for use on GridDyn
@param[in] eventString the description string for the event
@param[in] obj the target object of the event can be nullptr
@return a gridDynEvent object used in the other functions
*/
GRIDDYN_Export gridDynEvent gridDynEvent_create (const char *eventString, gridDynObject obj);
/** delete the event
@param[in] evnt  the event object to be freed
*/
GRIDDYN_Export void gridDynEvent_free (gridDynEvent evnt);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_trigger (gridDynEvent evnt);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@param[in] sim the simulation object to schedule the event on
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_schedule (gridDynEvent evnt, gridDynSimReference sim);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@param[in] parameter the parameter to change
@param[in] value the new value to set for the parameter
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_setValue (gridDynEvent evnt, const char *parameter, double value);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@param[in] parameter the parameter to change
@param[in] value the new value to set for the parameter
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_setString (gridDynEvent evnt, const char *parameter, const char *value);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@param[in] flag the name of the flag to change
@param[in] val the value to set the flag to (0 for false, otherwise for true)
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_setFlag (gridDynEvent evnt, const char *flag, int val);

/** trigger the action described by the event
@param[in] evnt the event object to perform the action on
@param[in] obj the object for the event to act upon
@return 0 for success or an error code representing an issue
*/
GRIDDYN_Export int gridDynEvent_setTarget (gridDynEvent evnt, gridDynObject obj);

/** @} */ //end of the C-api group

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif  // GRIDDYN_EXPORT_C_H_
