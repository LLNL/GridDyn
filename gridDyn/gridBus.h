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

#ifndef GRIDBUS_H_
#define GRIDBUS_H_

// headers
#include "gridObjects.h"

#include <queue>

// forward classes
class gridLink;
class gridLoad;
class gridDynGenerator;

#define GOOD_SOLUTION (0)
#define QLIMIT_VIOLATION (1)
#define VLIMIT_VIOLATION (2)
#define PLIMIT_VIOLATION (3)

/** @brief helper class for gridBus to store the power information
*/
class busPowers
{
public:
	double linkP = 0.0;						 //!< [puMW]    reactive power coming from Links
	 double linkQ = 0.0;                                //!< [pu]    reactive power coming from Links
	 double loadP = 0.0;					//!< [puMW] real power coming from the loads
	 double loadQ = 0.0;                    //!< [puMW]  reactive  power coming from Loads
	 double genP = 0.0;						//!< [puMW] real power from the generators
    double  genQ = 0.0;                     //!< [puMW]  reactive power from the generators
  index_t seqID=0;						//!< the sequence id of the latest state from which the powers are computed
  busPowers ()
  {
  }
  void reset ();
  bool needsUpdate (const stateData *sD) const;
  double sumP () const
  {
    return (linkP + loadP + genP);
  }
  double sumQ () const
  {
    return (linkQ + loadQ + genQ);
  }

};

/** @brief basic node in a power systems
  This Bus class is primarily a base Class for other types of buses, but it can be instantiated it provides the basic management of subObjects
and defines the interface.  It has no states and fixes the voltage and angle at whatever it gets set to

*/
class gridBus : public gridPrimary
{
public:
  /** @brief flags for the buses*/

  static std::atomic<count_t> busCount; //!<  counter for the buses
  static const int low_voltage_check_flag = object_flag1;
  //afix is a fixed angle bus for power flow
  /* @brief enumeration to define potential busTypes for power flow*/
  enum class busType
  {
    PQ = 0, afix = 1, PV = 2,SLK = 3
  };
  /* @brief enumeration to define potential busTypes for dynamic calculations*/
  enum class dynBusType
  {
    normal = 0,fixAngle = 1,fixVoltage = 2,dynSLK = 3
  };
  int Network = 0;                               //!<  the network a bus belongs to for labeling purposes
  int zone = 1;                                  //!< zone control for some reporting purposes for labeling purposes
protected:
  std::vector<gridLoad *> attachedLoads;                                             //!<  list of all the loads
  std::vector<gridLink *> attachedLinks;                                             //!< list of the attached links
  std::vector<gridDynGenerator *> attachedGens;                              //!< list of the attached generators
  std::vector<std::shared_ptr<gridSecondary>> objectHolder;  //!< storage location for shared_ptrs to secondary objects

  double angle = 0.0;                                   //!< [rad]     voltage angle
  double voltage = 1.0;                                 //!< [puV]    per unit voltage magnitude
  double baseVoltage = 120;                             //!< [kV]    base voltage level
  busType type = busType::PQ;                                   //!< [busType] bus type: PV, PQ, or slack/swing
  dynBusType dynType = dynBusType::normal;                     //!< dynamic bus type normal, fixAngle, fixVoltage, dynSLK same types as for power flow but for dynamic simulations

  busPowers S;                                //!< storage for the power computation from the various sources;
  double Vtol = -1.0;                         //!<[pu] voltage tolerance value <0 implies automatic setting from global levels
  double Atol = -1.0;                         //!<[rad] angle tolerance  value <0 implies automatic setting from global levels
  double freq = 1.0;       //!<[puHz] estimated actual frequency
  gridDyn_time lowVtime = -kBigNum;	//!< the last time a low voltage alert was triggered
  IOdata outputs;   //!< the current output values
  IOlocs outLocs;   //!< the current output locations
public:
  /** @brief default constructor*/
  explicit gridBus (const std::string &objName = "bus_$");
  /** @brief alternate constructor to specify voltage and angle
  @param[in] voltage  the initial voltage
  @param[in] angle the initial angle
  */
  gridBus (double voltage, double angle, const std::string &objName = "bus_$");
  /** @brief destructor*/
  virtual ~gridBus ();

  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  // add components
  virtual void add (gridCoreObject *obj) override;
  /** @brief  add a gridLoad object*/
  virtual void add (gridLoad *pl);
  /** @brief  add a gridGenerator object*/
  virtual void add (gridDynGenerator *gen);
  /** @brief  add a gridLink object*/
  virtual void add (gridLink *lnk);

  // remove components
  virtual void remove (gridCoreObject *obj) override;
  /** @brief  remove a gridLoad object*/
  virtual void remove (gridLoad *pl);
  /** @brief  remove a gridDynGenerator object*/
  virtual void remove (gridDynGenerator *gen);
  /** @brief  remove a gridLink object*/
  virtual void remove (gridLink *lnk);
  //deal with control alerts
  virtual void alert (gridCoreObject *, int code) override;

  // initializeB
  virtual void setOffsets (const solverOffsets &newOffsets, const solverMode &sMode) override;
  virtual void setOffset (index_t offset, const solverMode &sMode) override;
  virtual void loadSizes (const solverMode &sMode, bool dynOnly) override;

  virtual void setRootOffset (index_t Roffset, const solverMode &sMode) override;
protected:
  virtual void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void pFlowObjectInitializeB () override;
public:
  virtual change_code powerFlowAdjust (unsigned long flags, check_level_t level) override;      //only applicable in pFlow
  /** @brief  adjust the power levels of the contained adjustable secondary objects
  @param[in] adjustment the amount of the adjustment requested*/
  virtual void powerAdjust (double adjustment);
  virtual void reset (reset_levels level = reset_levels::minimal) override;
  //initializeB dynamics
protected:
  virtual void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  virtual void dynObjectInitializeB (IOdata &outputSet) override;
public:
  virtual void disable () override;
  /** @brief  disconnect the bus*/
  virtual void disconnect () override;
  /** @brief  reconnect the bus
  @param[in] mapBus  a bus to pick of startup parameters from*/
  virtual void reconnect (gridBus *mapBus);
  virtual void reconnect () override;
  // parameter set functions
  virtual void setAll (const std::string &type, std::string param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype = paramStringType::all) const override;
  virtual void setFlag (const std::string &flag, bool val) override;
  virtual void set (const std::string &param,  const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  // parameter get functions
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  // solver functions
  virtual void preEx (const stateData *sD, const solverMode &sMode) override;
  virtual void jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void algebraicUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha) override;
  virtual void voltageUpdate (const stateData *sD, double update[], const solverMode &sMode, double alpha);
  virtual void guess (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode) override;

  virtual void converge (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode, converge_mode = converge_mode::high_error_only, double tol = 0.01) override;

  /** @brief  return the last error in the real power*/
  virtual double lastError () const;

  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode) override;

public:
  void setTime (gridDyn_time time) override;
  void timestep (gridDyn_time ttime, const solverMode &sMode) override;

  virtual void setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  /** @brief a faster function to set the voltage and angle of a bus*
  @param[in] Vnew  the new voltage
  @param[in] Anew  the new angle
  */
  virtual void setVoltageAngle (double Vnew, double Anew);
  //for identifying which variables are algebraic vs differential
  virtual void getVariableType (double sdata[], const solverMode &sMode) override;       //only applicable in DAE mode
  virtual void getTols (double tols[], const solverMode &sMode) override;
  // dynamic simulation
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix) const override;
  /** @brief function to propagate a network value to all connected buses
  @param[in] network the new network number
  @param[in,out] bstk  a queue containing the stack of buses that have yet to be checked
  */
  virtual void followNetwork (int network, std::queue<gridBus *> &bstk);
  /** @brief check if the bus is capable of operating
  */
  virtual bool checkCapable ();
  // find components
  /** @brief find a link based on the bus desiring to be connected to
  @param[in] bs  the bus we want to check for a connecting link
  @return  a pointer to a Link that connects the current bus to the bus specified by bs or nullptr if none exists
  */
  gridLink * findLink (gridBus *bs) const;
  gridCoreObject * find (const std::string &objname) const override;
  gridCoreObject * getSubObject (const std::string &typeName, index_t num) const override;
  gridCoreObject * findByUserID (const std::string &typeName, index_t searchID) const override;

  gridLink * getLink (index_t x) const override;
  /**
  *@brief get a pointer for a particular Load
  @param[in] x the index of the load being requested
  @return a pointer to the requested load or nullptr
  **/
  gridLoad * getLoad (index_t x = 0) const;
  /**
  *@brief get a pointer for a particular generator
  @param[in] x the index of the generator being requested
  @return a pointer to the requested generator or nullptr
  **/
  gridDynGenerator * getGen (index_t x = 0) const;
  /** @brief for identifying if there is a direct path from object to another in the system
  * @param target the object to search for
  * @param source the object from back up the path
  * @return whether there is a singular path to the object or not
  **/
  bool directPath (gridObject *target, gridObject *source = nullptr);
  /** @brief for obtaining the complete path to another object
  * @param target the object to search for
  * @param source the object from back up the path
  * @return a vector of objects with the path information  (NOTE: not necessarily the shortest path)
  **/
  std::vector<gridObject *> getDirectPath (gridObject *target, gridObject *source = nullptr);
  /** @brief propagate and fix a power level for a bus
  * @param[in] makeSlack boolean indicating if the bus should be made into a slack bus or not
  * @return value indicating success 0 on success -1 on failure
  **/
  virtual int propogatePower (bool makeSlack = false);
  /** @brief get the bus type for power flow mode
  * @return the type of the bus
  **/
  busType getType () const
  {
    return type;
  }
  /** @brief get the bus type for dynamic calculations
  * @return the type of the bus
  **/
  dynBusType getDynType () const
  {
    return dynType;
  }
  /** @brief get the bus voltage
  * @return the bus voltage
  **/
  double getVoltage () const
  {
    return voltage;
  }
  /** @brief get the bus angle
  * @return the bus angle
  **/
  double getAngle () const
  {
    return angle;
  }
  /** @brief get the bus frequency
  * @return the bus frequency
  **/
  double getFreq () const
  {
    return freq;
  }
  /** @brief get the bus real generation
  * @return the bus real generation
  **/
  double getGenerationReal () const
  {
    return S.genP;
  }
  /** @brief get the bus reactive generation
  * @return the bus reactive generation
  **/
  double getGenerationReactive () const
  {
    return S.genQ;
  }

  virtual double getMaxGenReal () const
  {
    return kBigNum;
  }
  /** @brief get the maximum reactive power generation
  * @return the maximum reactive power generation
  **/
  virtual double getMaxGenReactive () const
  {
    return kBigNum;
  }

  double getLoadReal () const
  {
    return S.loadP;
  }
  /** @brief get the reactive power Load
  * @return the reactive power Load
  **/
  double getLoadReactive () const
  {
    return S.loadQ;
  }
  /** @brief get the real power coming from links
  * @return the real link power
  **/
  double getLinkReal () const
  {
    return S.linkP;
  }
  /** @brief get the reactive power coming from links
  * @return the reactive link power
  **/
  double getLinkReactive () const
  {
    return S.linkQ;
  }
  /** @brief get the available controllable upward adjustments within a time period
  @ details this means power production or load reduction
  @param[in] time  the time period within which to do the adjustments
  * @return the reactive link power
  **/
  virtual double getAdjustableCapacityUp (gridDyn_time time = kBigNum) const;
  /** @brief get the available controllable upward adjustments within a time period
  @ details this means power production or load reduction
  @param[in] time  the time period within which to do the adjustments
  * @return the reactive link power
  **/
  virtual double getAdjustableCapacityDown (gridDyn_time time = kBigNum) const;
  /** @brief the dPdf partial derivative  (may be deprecated in the future)
  * @return the $\frac{\partial P}{\partial f}$
  **/
  virtual double getdPdf () const
  {
    return 0;
  }
  /**@brief boolean indicator if the bus has inertial generators or loads*/
  bool hasInertialAngle () const;
  /** @brief get the tie error (may be deprecated in the future as this should be handled at an area level)
  * @return the tie error
  **/
  virtual double getTieError () const
  {
    return kNullVal;
  }
  /** @brief get the frequency response
  * @return the tie error
  **/
  virtual double getFreqResp () const;
  /** @brief get available regulation
  * @return the available regulation
  **/
  virtual double getRegTotal () const;
  /** @brief get the scheduled power
  * @return the scheduled power
  **/
  virtual double getSched () const;

  virtual IOdata getOutputs (const stateData *sD, const solverMode &sMode) override;
  virtual IOlocs getOutputLocs  (const solverMode &sMode) const override;

  /** @brief get a const ref to the outputs*/
  const IOdata &getOutputsRef () const;
  /** @brief get a const ref to the output Locations
  */
  virtual const IOlocs &getOutputLocsRef () const;

  virtual double getOutput (const stateData *sD, const solverMode &sMode, index_t outNum = 0) const override;
  /** @brief get the voltage
  * @param[in] state the system state
  @param[in] sMode the corresponding solverMode to the state
@return the sbus voltage
  **/
  virtual double getVoltage (const double state[], const solverMode &sMode) const;
  /** @brief get the angle
  * @param[in] state the system state
  @param[in] sMode the corresponding solverMode to the state
  @return the bus angle
  **/
  virtual double getAngle (const double state[], const solverMode &sMode) const;
  /** @brief get the voltage
  * @param[in] sD the system state data
  @param[in] sMode the corresponding solverMode to the state data
  @return the bus voltage
  **/
  virtual double getVoltage (const stateData *sD, const solverMode &sMode) const;
  /** @brief get the angle
  * @param[in] sD the system state data
  @param[in] sMode the corresponding solverMode to the state
  @return the bus angle
  **/
  virtual double getAngle (const stateData *sD, const solverMode &sMode) const;
  /** @brief get the bus frequency
  * @param[in] sD the system state data
  @param[in] sMode the corresponding solverMode to the state
  @return the bus frequency
  **/
  virtual double getFreq (const stateData *sD, const solverMode &sMode) const;

  virtual void rootTest (const stateData *sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger (gridDyn_time ttime, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual change_code rootCheck (const stateData *sD, const solverMode &sMode,  check_level_t level) override;


  friend bool compareBus (gridBus *bus1, gridBus *bus2, bool cmpLink, bool printDiff);
  virtual void updateFlags (bool dynOnly = false) override;
  //for registering and removing power control objects

  //for dealing with buses merged with zero impedance link
  /** @brief  merge a bus with the calling bus*/
  virtual void mergeBus (gridBus *mbus);
  /** @brief  unmerge a bus with the calling bus*/
  virtual void unmergeBus (gridBus *mbus);
  /** @brief  check if all the buses that are merged should be*/
  virtual void checkMerge ();

  /** @brief  register an object for voltage control on a bus*/
  virtual void registerVoltageControl (gridObject *obj);
  /** @brief  remove an object from voltage control on a bus*/
  virtual void removeVoltageControl (gridObject *obj);
  /** @brief  register an object for power control on a bus*/
  virtual void registerPowerControl (gridObject *obj);
  /** @brief  remove an object from power control on a bus*/
  virtual void removePowerControl (gridObject *obj);

protected:
  /** @brief
  @param[in] sD  the statedata to compute the Error for
  @param[in] sMode the solverMode corresponding to the stateData
  @return the error in the power balance equations
  */
  virtual double computeError (const stateData *sD, const solverMode &sMode);
private:
  template<class X>
  friend void addObject (gridBus *bus, X* obj, std::vector<X *> &objVector);
};




/** @brief compare 2 buses
  check a number of bus parameters to see if they match, probably not that useful of function any more ,but it was useful during development
@param[in] bus1  bus1
@param[in] bus2 bus2
@param[in] cmpLink  whether to compare links or not  (deep comparison of links)
@param[in] printDiff  true if the diffs are to be printed
@return true if match
*/
bool compareBus (gridBus *bus1, gridBus *bus2, bool cmpLink = false,bool printDiff = false);

/** @brief find the matching bus in a different tree
  searches a cloned object tree to find the corresponding bus
@param[in] bus  the bus to search for
@param[in] src  the existing parent object
@param[in] sec  the desired parent object tree
@return a pointer to a bus on the sec tree that matches bus based on name and location
*/
gridBus * getMatchingBus (gridBus *bus, const gridPrimary *src, gridPrimary *sec);

#endif
