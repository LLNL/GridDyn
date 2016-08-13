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

#ifndef GRIDLINK_H_
#define GRIDLINK_H_

#include "gridObjects.h"
#include <queue>
class gridBus;  //tempory class definition

/** @brief structure containing information on the states of the joining buses
 included the voltages, angles and some common calculations used in calculating the flows
\f[
Vmx=\frac{v1*v2}{tap}
\f]
the seqID is also the index of the state data it was calculated from*/
typedef struct linkBusInformation
{
  double v1 = 0.0;                                                  //!< [p.u.] voltage at bus1
  double v2 = 0.0;                                                  //!< [p.u.] voltage at bus2
  double theta1 = 0.0;                                      //!<  angle at bus1
  double theta2 = 0.0;                                      //!<  angle at bus2
  index_t  seqID = 0;                                                                           //!< the current sequence id of the local data
} linkI;

/** @brief structure containing information on the flows for the link
        the seqID is also the index of the state data it was calculated from*/
typedef struct linkPowerTransferInformation
{
  double P1 = 0.0;                         //!< [p.u.] power transferred from bus 1
  double P2 = 0.0;                         //!< [p.u.] power transferred from bus 2
  double Q1 = 0.0;                         //!< [p.u.] reactive power transferred from bus 1
  double Q2 = 0.0;                         //!< [p.u.] reactive power transferred from bus 2
  index_t  seqID = 0;
} linkF;

/** @brief the basic class that links multiple nodes(buses) together
*  the base class for objects which connect other obects mainly buses
it implements a trivial transport model Q=0, P1=Pset P2=-(Pset-LossFraction*Pset)

Each link has a disconnect switch at the from bus and the to bus
*/
class gridLink : public gridPrimary
{
public:
  static count_t linkCount;               //!<static variable counting the number of created lines used for automatic user ID creation
  //it can be edited as it does not impact link operations just for user convenience
  /** @brief define some basic flag locations for gridLink*/
  enum gridLink_flags
  {
    switch1_open_flag = object_flag1, //!<  switch for the from bus
    switch2_open_flag = object_flag2, //!< switch for the to bus
    fixed_target_power = object_flag3,  //!< flag indicating if the power flow was fixed
  };
  int zone = 1;  //!< publically accessible loss zone indicator not used internally
protected:
  double ratingA = -kBigNum;                                  //!< the long term rating of the link
  double ratingB = -kBigNum;                                  //!< the short term rating of the link
  double Erating = -kBigNum;                                  //!<the emergency rating of the link

  gridBus *B1 = nullptr;                //!< the bus on the from side
  gridBus *B2 = nullptr;                //!< the bus on the to side
  double Pset = 0;                //!< the scheduled power of the link
  double lossFraction = 0;                      //!< the fraction of power transferred that is lossed

  index_t curcuitNum = 1;       //!< helper field for multicurcuit links
  linkI linkInfo;                               //!< holder for the latest bus information
  linkF linkFlows;              //!< holder latest computed power flow information
public:
  /** @brief default constructor*/
  gridLink (const std::string &objName = "link_$");

  /** @brief virtual destructor*/
  virtual ~gridLink ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void disable () override;
  /** @brief reconnect the link*/
  virtual void disconnect () override;

  virtual void reconnect () override;
  //handle end switches
  /** @brief get the switch state
  * @return true if switch is open, false if closed
  */
  virtual bool switchTest () const
  {
    return (opFlags[switch1_open_flag] || opFlags[switch2_open_flag]);
  }
  /** @brief get the switch state
  @param[in] num  the number of the switch 1 for "from" bus 2 for "to" bus
  * @return true if switch is open, false if closed
  */
  virtual bool switchTest (index_t num) const
  {
    return (num == 2) ? opFlags[switch2_open_flag] : opFlags[switch1_open_flag];
  }
  /** @brief set the switch state
  @param[in] num  the number of the switch 1 for "from" bus 2 for "to" bus
  @param[in] mode the mode of the switch true for open  false for closed
  */
  virtual void switchMode (index_t num, bool mode);
  /** @brief get the connectivity
  * @return true if there is a connection between the to and from bus
  */
  virtual bool isConnected () const override;

  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode) override;
  /** @brief allow the real power flow to be fixed by adjusting the properties of one bus or another
   performs the calculations necessary to get the power at the mterminal to be a certain value
  @param[in] power  the desired real power flow as measured by mterminal
  @param[in] mterminal  the measrure terminal-either a terminal number (1 or higher) or a busID,  1 by default
  @param[in] fixedTerminal-the terminal that doesn't change (terminal number or busID) if 0 both are changed or 1 is selected based on busTypes
  @param[in] unitType -- the units related to power
  @return 0 for success, some other number for failure
  */
  virtual int fixRealPower (double power, index_t  mterminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit);
  /** @brief allow the power flow to be fixed by adjusting the properties of one bus or another
   performs the calculations necessary to get the power at the mterminal to be a certain value
  @param[in] rPower  the desired real power flow as measured by mterminal
  @param[in] rPower  the desired reactive power flow as measured by mterminal
  @param[in] mterminal  the measrure terminal-either a terminal number (1 or higher) or a busID,  1 by default
  @param[in] fixedTerminal-the terminal that doesn't change (terminal number or busID) if 0 both are changed or 1 is selected based on busTypes
  @param[in] unitType -- the units related to power
  @return 0 for success, some other number for failure
  */
  virtual int fixPower (double rPower, double qPower, index_t  mterminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit);
  /** @brief propogate a network number to all connected buses
   checks if a link actually connects the two buses in an AC sense, then checks if a bus is already part of the specified network and if not it adds it to the queue
  @param[in] network  the new network number
  @param[in] bstk a FIFO queue of buses to add to the network
  */
  virtual void followNetwork (int network, std::queue<gridBus *> &bstk);
  /** @brief update one of the link terminals with a new bus
  @param[in] bus  the new bus to connect to
  @param[in] busnumber  1 for from,  2 for "to"
  @return 0 for success, some other number for failure
  */
  virtual int updateBus (gridBus *bus,index_t busnumber);
  /** @brief check for any violations of link limits or other factors based on power flow results
   checks things like the maximum angle,  power flow /current limits based on ratings and a few other things
  @param[out] Violation_vector --a list of all the violations any new violations get added to the result
  */
  virtual void pFlowCheck (std::vector<violation> &Violation_vector) override;

  virtual double timestep (double ttime,const solverMode &sMode) override;
  /** @brief do a quick update  (may be deprecated)
  * @return the power transfer
  */
  virtual double quickupdateP ();
  /** @brief get the remaining capacity of the line
  * @return the remaining capacity
  */
  virtual double remainingCapacity () const;
  /** @brief get angle differential of the buses
  * @return the angle difference
  */
  virtual double getAngle () const;
  /** @brief get angle differential of the buses
  @param[in] state  the state of the system
  @param[in] sMode the solverMode the state is based on
  * @return the angle difference
  */
  virtual double getAngle (const double state[], const solverMode &sMode) const;
  /** @brief get the absolute angle of an attached bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the absolute angle
  */
  virtual double getAbsAngle (index_t busId = 0) const;
  /** @brief get the voltage of an attached bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the voltage
  */
  virtual double getVoltage (index_t  busId = 0) const;
  /** @brief get the real perceived impedance at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the perceived impedance
  */
  virtual double getRealImpedance (index_t  busId = 0) const;
  /** @brief get the imaginary part of the perceived impedance at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the perceived imaginary component of the impedance
  */
  virtual double getImagImpedance (index_t  busId = 0) const;
  /** @brief get the magnitude of the perceived impedance at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the magnitude of the perceived impedance, sign indicated flow direction of the real current
  */
  virtual double getTotalImpedance (index_t  busId = 0) const;
  /** @brief get the magnitude of the current at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the magnitude of the current, sign indicated flow direction of the real current
  */
  virtual double getCurrent (index_t  busId = 0) const;
  /** @brief get the real part of the current at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the real part of the current
  */
  virtual double getRealCurrent (index_t  busId = 0) const;
  /** @brief get the imaginary part of the current at a particular bus
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the imaginary part of the current
  */
  virtual double getImagCurrent (index_t  busId = 0) const;
  /** @brief get the real power at a particular bus
   this function is to allow a link to act similarly to a gridSecondary
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the real Power
  */
  virtual double getRealPower (index_t  busId = 0) const;
  /** @brief get the real power at a particular bus
   this function is to allow a link to act similarly to a gridSecondary
  @param[in] busId  either 1 or 2 or the object id of the bus
  * @return the real Power
  */
  virtual double getReactivePower (index_t  busId = 0) const;
  /** @brief get the loss of a line
  * @return the loss in pu
  */
  virtual double getLoss () const;
  /** @brief get reactive power consumed by the link
  * @return the reactive power consumption
  */
  virtual double getReactiveLoss () const;
  /** @brief get the number of terminals
  * @return the number of terminals
  */
  virtual count_t terminalCount () const
  {
    return 2;
  }
  /** @brief get the max transfer capacity
  * @return the max capacity
  */
  virtual double getMaxTransfer () const;

  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  gridBus * getBus (index_t busInd)  const override;

  gridCoreObject * getSubObject (const std::string &typeName, index_t num) const override;


  // initializeB power flow
protected:
  virtual void pFlowObjectInitializeA (double time0, unsigned long flags) override;
public:
  /** @brief check if two buses should be merged and the line effects ignored
  */
  virtual void checkMerge ()
  {
  }
  //initializeB dynamics
protected:
  virtual void dynObjectInitializeA (double time0, unsigned long flags) override;
public:
  //for computing all the jacobian elements at once

  virtual void ioPartialDerivatives (index_t  busId, const stateData *sD, arrayData<double> *ad, const IOlocs &argLocs, const solverMode &sMode);

  virtual void outputPartialDerivatives (const stateData *sD, arrayData<double> *ad, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (index_t  busId, const stateData *sD, arrayData<double> *ad, const solverMode &sMode);

  //virtual void busResidual(index_t busId, const stateData *sD, double *Fp, double *Fq, const solverMode &sMode);
  virtual IOdata getOutputs (const stateData *sD, const solverMode &sMode) override;
  virtual IOdata getOutputs (index_t busId, const stateData *sD, const solverMode &sMode);
  virtual void setState (double ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;

protected:
  /** @brief deal with any effects of a switch change
  @param[in] switchNum  the number of the switch that changed
  */
  virtual void switchChange (int switchNum);
private:
  /**function to do the power transfer computation*/
  void computePowers ();

};
/** @brief find the matching link in a different tree
  searches a cloned object tree to find the corresponding link
@param[in] bus  the link to search for
@param[in] src  the existing parent object
@param[in] sec  the desired parent object tree
@return a pointer to a link on the second tree that matches the calling link based on name and location
*/
gridLink * getMatchingLink (gridLink *lnk, gridPrimary *src, gridPrimary *sec);



/** @brief compare 2 links
  check a number of link parameters to see if they match, probably not that useful of function any more ,but it was useful during development
@param[in] lnk1  the first link to check
@param[in] lnk2  the second link to check
@param[in] cmpBus  whether to compare links or not  (deep comparison of links)
@param[in] printDiff  true if the diffs are to be printed
@return true if match
*/
bool compareLink (gridLink *lnk1, gridLink *lnk2, bool cmpBus = false, bool printDiff = false);
#endif
