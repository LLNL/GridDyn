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

#ifndef ACLINE_H_
#define ACLINE_H_

#include "linkModels/gridLink.h"

#define APPROXIMATION_LEVELS (9)


typedef struct linkComputedInformation
{
  double cosTheta1 = 0.0;                                   //!<  cos theta1
  double cosTheta2 = 0.0;                                   //!<  cos theta2
  double sinTheta1 = 0.0;                                   //!<  sin theta1
  double sinTheta2 = 0.0;                                   //!<  sin theta2
  double Vmx = 0.0;                                                 //!< computed parameter
} linkC;

/** @brief structure containing information on the partial derivatives for the the link
the seqID is also the index of the state data it was calculated from*/
typedef struct linkPartialDerivatives
{
  double dP1dv1 = 0.0;
  double dP2dv1 = 0.0;
  double dQ1dv1 = 0.0;
  double dQ2dv1 = 0.0;
  double dP1dv2 = 0.0;
  double dP2dv2 = 0.0;
  double dQ1dv2 = 0.0;
  double dQ2dv2 = 0.0;
  double dP1dt1 = 0.0;
  double dP2dt1 = 0.0;
  double dQ1dt1 = 0.0;
  double dQ2dt1 = 0.0;
  double dP1dt2 = 0.0;
  double dP2dt2 = 0.0;
  double dQ1dt2 = 0.0;
  double dQ2dt2 = 0.0;
  index_t  seqID = 0;
} linkPart;

/** @brief the class that links multiple nodes(buses) together
*  the base class for objects which connect other obects mainly buses
it implements the basic transmission model

Each link has a disconnect switch at the from bus and the to bus
*/
class acLine : public gridLink
{
public:
protected:
  double minAngle = -kPI / 2.0;                     //!<the minimum angle of the link can handle
  double maxAngle = kPI / 2.0;                      //!<the maximum angle the link can handle--related to rating

  double length = 0.0;                    //!< [km] transmission line length
  double r = 0;                           //!< [pu] per unit resistance
  double x = 0.00000001;                           //!< [pu] per unit reactance
  double mp_B = 0.0;                      //!< [pu] per unit shunt capacitance (jb/2 on each end of the line)
  double mp_G = 0.0;                                  //!< [pu] per unit shunt conductance (g/2 on each end of the line)
  double fault = -1.0;                        //!< fault location along the line keep at <0 for no fault
  double g = 0.0;                         //!< [pu] per unit conductance (calculated parameter)
  double b = 0.0;                         //!< [pu] per unit susceptance (calculated parameter)
  double tap = 1.0;                       //!< tap position, neutral t = 1;
  double tapAngle = 0.0;                  //!< [deg] phase angle for phase shifting transformer

  linkI constLinkInfo;                                                //!< holder for static link bus information
  linkC linkComp;                                       //!< holder for some computed information
  linkC constLinkComp;                          //!< holder for some computed information

  linkF constLinkFlows;                                               //!< holder for previous steady state link flows
  linkPart LinkDeriv;                                                 //!< holder for computed derivative information

  //this is the weirdest C++ syntax for a typedef,  but as far as I can tell I need to do it to create a array of member function pointers still not clear how this actually works
  typedef void (acLine::*glMP)();
  glMP flowCalc[APPROXIMATION_LEVELS];                //!< function pointers to power flow calculations
  glMP derivCalc[APPROXIMATION_LEVELS];                //!< function objects to the derivative calculations

public:
  /** @brief default constructor*/
  explicit acLine (const std::string &objName = "acline_$");
  /** @brief constructor specifying the real and imaginary part of the impedance
  @param[in] rP  the real impedance in pu ohm
  @param[in] xP  the reactance in pu Ohm
  @param[in] objName the name of the link
  */
  acLine (double rP, double xP, const std::string &objName = "acline_$");
  /** @brief virtual destructor*/
  virtual ~acLine ();
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;


  /** @brief get the current tap value
  * @return the tap value
  */
  double getTap () const
  {
    return tap;
  }
  /** @brief get the current tap angle value
  * @return the tap angle value
  */
  double getTapAngle () const
  {
    return tapAngle;
  }

  void disable () override;
  /** @brief allow the real power flow to be fixed by adjusting the properties of one bus or another
   performs the calculations necessary to get the power at the mterminal to be a certain value
  @param[in] power  the desired real power flow as measured by mterminal
  @param[in] mterminal  the measrure terminal-either a terminal number (1 or higher) or a busID,  1 by default
  @param[in] fixedTerminal -the terminal that doesn't change (terminal number or busID) if 0 both are changed or 1 is selected based on busTypes
  @param[in] unitType -- the units related to power
  @return 0 for success, some other number for failure
  */
  virtual int fixRealPower (double power, index_t  mterminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) override;
  /** @brief allow the power flow to be fixed by adjusting the properties of one bus or another
   performs the calculations necessary to get the power at the mterminal to be a certain value
  @param[in] rPower  the desired real power flow as measured by mterminal
  @param[in] rPower  the desired reactive power flow as measured by mterminal
  @param[in] mterminal  the measrure terminal-either a terminal number (1 or higher) or a busID,  1 by default
  @param[in] fixedTerminal -the terminal that doesn't change (terminal number or busID) if 0 both are changed or 1 is selected based on busTypes
  @param[in] unitType -- the units related to power
  @return 0 for success, some other number for failure
  */
  virtual int fixPower (double rPower, double qPower, index_t  mterminal, index_t  fixedTerminal = 0, gridUnits::units_t unitType = gridUnits::defUnit) override;

  /** @brief check for any violations of link limits or other factors based on power flow results
   checks things like the maximum angle,  power flow /current limits based on ratings and a few other things
  @param[out] Violation_vector --a list of all the violations any new violations get added to the result
  */
  virtual void pFlowCheck (std::vector<violation> &Violation_vector) override;
  virtual void pFlowObjectInitializeB() override;
  virtual void updateLocalCache () override;
  virtual void updateLocalCache (const stateData *sD, const solverMode &sMode) override;

  virtual void timestep (gridDyn_time ttime, const solverMode &sMode) override;
  /** @brief do a quick update  (may be deprecated)
  * @return the power transfer
  */
  virtual double quickupdateP () override;

  using gridLink::getAngle;
  virtual double getAngle (const double state[], const solverMode &sMode) const override;

  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  /** @brief check if two buses should be merged and the line effects ignored
  */
  virtual void checkMerge () override
  {
  }

  //for computing all the Jacobian elements at once

  virtual void ioPartialDerivatives (index_t  busId, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;

  virtual void outputPartialDerivatives (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (index_t  busId, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;

  virtual double getMaxTransfer () const override;
  //virtual void busResidual(index_t busId, const stateData *sD, double *Fp, double *Fq, const solverMode &sMode);
  virtual void setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;

  virtual change_code rootCheck (const stateData *sD, const solverMode &sMode, check_level_t level) override;

protected:
  void setAdmit ();
  // virtual void basePowerComp ();
  /** @brief calculations for fault conditions
  */
  void faultCalc ();
  /** @brief full ac calculations
  */
  void fullCalc ();
  /** @brief assumes r is small and can be ignored
  */
  void simplifiedCalc ();
  /** @brief assumes there is no coupling between P -theta and V-Q
  */
  void decoupledCalc ();
  /** @brief assumes the r is small and decoupled
  */
  void simplifiedDecoupledCalc ();
  /** @brief assumes the angle is small
  */
  void smallAngleCalc ();
  /** @brief assumes the angle is small and r is small
  */
  void smallAngleSimplifiedCalc ();
  /** @brief small angle decoupled
  */
  void smallAngleDecoupledCalc ();
  /** @brief linearizes computations around previous operating point
  */
  void linearCalc ();
  /** @brief computes the values for nonConnected line
  */
  void swOpenCalc ();
  /** @brief assumes r is small and the angle is small and the voltage is constant for real power computation and angle is constant for reactive power computation
  so there is no cross coupling between V and theta*/
  void fastDecoupledCalc ();
  /** @brief  fault Derivative Calculations
  */
  void faultDeriv ();
  /** @brief  full ac calculations for partial derivatives
  */
  void fullDeriv ();
  /** @brief assumes r is small and can be ignored for partial derivatives
  */
  void simplifiedDeriv ();
  /** @brief assumes there is no coupling between P -theta and V-Q for partial derivatives
  */
  void decoupledDeriv ();
  /** @brief assumes the r is small and decoupled
  */
  void simplifiedDecoupledDeriv ();
  /** @brief assumes the angle is small
  */
  void smallAngleDeriv ();
  /** @brief small angle decoupled
  */
  void smallAngleDecoupledDeriv ();
  /** @brief assumes the angle is small and r is small
  */
  void smallAngleSimplifiedDeriv ();
  /** @brief linearizes computations around previous operating point
  */
  void linearDeriv ();
  /** @brief assumes r is small and the angle is small and the voltage is constant for real power computation and angle is constant for reactive power computation
  so there is no cross coupling between V and theta for partial derivatives*/
  void fastDecoupledDeriv ();
  /** @brief switch open derivatives*/
  void swOpenDeriv ();
  /** @brief load information into the linkInfo structure*/
  void loadLinkInfo ();
  /** @brief load information into the linkInfo structure
  @param[in] sD  the state Data
  @param[in] sMode the corresponding solver Mode*/
  void loadLinkInfo (const stateData *sD, const solverMode &sMode);
  /** @brief load the approximation functions in the bizarely defined array above*/
  void loadApproxFunctions ();

  void switchChange (int switchNum) override;
};
/** @brief find the matching link in a different tree
  searches a cloned object tree to find the corresponding link
@param[in] bus  the link to search for
@param[in] src  the existing parent object
@param[in] sec  the desired parent object tree
@return a pointer to a link on the second tree that matches the calling link based on name and location
*/
gridLink * getMatchingLink (gridLink *lnk, gridPrimary *src, gridPrimary *sec);

/** @brief extends the link class to include adjustments to the tap and tapAngle
*  principally a model for adjustable transformers such as ULTC or regulators or other types of transformers with
adjustments.
it implements three types of control in power flow voltage control of local or remote buses,  local reactive power flow, and local real power flow
the real power flow is the only one that adjusts the angle.  available control include stepped and continuous

*/
class adjustableTransformer : public acLine
{
public:
  /** @brief  enumeration of the available control types
  */
  enum class control_mode_t
  {
    manual_control = 0,             //!< no automatic adjustments
    voltage_control = 1,                //!< automatic control based on voltage
    MW_control = 2,                 //!< automatic control based on real power flow at a specific terminal
    MVar_control = 3                 //!<automatic control based on real power flow at a specific terminal
  };
  /** @brief  flags for
  */
  enum adjustable_flags
  {
    continuous_flag = object_flag5,              //!< flag indicating continuous adjustments
    use_target_mode = object_flag6,              //!< flag indicating target mode
    at_limit = object_flag7,                                //!< flag indicating the adjustments are at their limit
    no_pFlow_adjustments = object_flag8,              //!< flag turning off all automatic adjustments
    use_lookup_table = object_flag9,               //!< flag indicating use of an impedance lookup table
  };

protected:
  control_mode_t cMode = control_mode_t::manual_control;        //!< control Mode
  double stepSize = 0.01;        //!< step size of the adjustment for non-continuous adjustments
  double maxTapAngle = kPI / 4.0;        //!< maximum tap angle
  double minTapAngle = -kPI / 4.0;     //!< minimum tap angle
  double minTap = 0.9;        //!< minimum tap setting
  double maxTap = 1.1;        //!< maximum tap setting
  double Vtarget = -kBigNum;       //!< target voltage
  double Vmax = kBigNum;        //!< maximum voltage before changing the tap
  double Vmin = 0;        //!< minimum voltage before changing the tap

  double Ptarget = -kBigNum;        //!< the target power flow
  double Pmin = -kBigNum;        //!< the minimum power level before changing the tap
  double Pmax = kBigNum;        //!< the maximum power before changing the tap

  double Qtarget = -kBigNum;        //!< the target reactive power flow
  double Qmax = kBigNum;        //!< the minimum power level before changing the tap
  double Qmin = -kBigNum;        //!< the maximum power before changing the tap
  // double Tm;				//!< time constant
  // double Td;				//!< time constant
  double direction = 1;               //!< variable storing whether the directional derivate of the tap changes with respect to voltage or power is positive or negative
  double tapMaxChangeRate;                    //!<maximum rate at which the tap can change
  double sample_rate;                 //!< the rate at which the measurements are sampled
  gridBus *controlBus = nullptr;        //!< the control bus to monitor voltage

  double tap0;              //!< baseline tap position used for continuous tap settings
  double tapAngle0;        //!< baseline tapAngle position used for continuous tap settings
  double stepDelay = 30;        //!< step control for adjusting the quantity or the time constant for continuous system
  double mp_Tm = 0.05;                //!< time constant for continuous tap settings
  double dTapdt = 0;       //!< rate of change of the tap
  double dTapAdt = 0;       //!< rate of change of the tapAngle
private:
  int controlNum = -1;        //!< the control bus and number setting are not fully determined until initialization so this stores information from the startup phase
  std::string controlName;       //!< the control bus and number setting are not fully determined until initialization so this stores information from the startup phase
  count_t adjCount;
  count_t oCount;
  double prevAdjust;
  double prevValue;
  //double baseValue;
public:
  explicit adjustableTransformer (const std::string &objName = "adjTX_$");
  /** @brief default constructor
  @param[in] rP  resistance of the link
  @param[in] xP  reactance of the link forwarded to the gridLink constructor
  */
  adjustableTransformer (double rP, double xP, const std::string &objName = "adjTX_$");
  //!< @brief destructor
  ~adjustableTransformer ()
  {
  }
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
  void set (const std::string &param, const std::string &val) override;
  void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  //adjuster specific functions
  /**@ brief set the control bus to a specified bus pointer
  @param[in] cBus  the specified control Bus*/
  void setControlBus (gridBus *cBus);
  /**@ brief set the control bus to a specified bus number
  @param[in] busnumber-- this can be 1 or 2 for already attached buses or the user id of a bus in which cases the parent of the link is searched for the bus*/
  void setControlBus (index_t  busnumber = 2);


  change_code powerFlowAdjust (unsigned long flags, check_level_t level) override;
  void reset (reset_levels level) override;

  void updateLocalCache () override;
  void updateLocalCache (const stateData *sD, const solverMode &sMode) override;

  virtual IOdata getOutputs (index_t  busId, const stateData *sD, const solverMode &sMode) override;

  void jacobianElements (const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;
  //for computing all the Jacobian elements at once
  virtual void ioPartialDerivatives (index_t  busId, const stateData *sD, matrixData<double> &ad, const IOlocs &argLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (index_t  busId, const stateData *sD, matrixData<double> &ad, const solverMode &sMode) override;

  void residual (const stateData *sD, double resid[], const solverMode &sMode) override;
  void setState (gridDyn_time ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  void guess (gridDyn_time ttime, double state[], double dstate_dt[], const solverMode &sMode) override;
  void loadSizes (const solverMode &sMode, bool dynOnly) override;
protected:
  void pFlowObjectInitializeA (gridDyn_time time0, unsigned long flags) override;
  void dynObjectInitializeA (gridDyn_time time0, unsigned long flags) override;

public:
  void rootTest (const stateData *sD, double roots[], const solverMode &sMode) override;
  void rootTrigger (gridDyn_time ttime, const std::vector<int> &rootMask, const solverMode &sMode) override;
  virtual void followNetwork (int network, std::queue<gridBus *> &bstk) override;
  virtual void getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;
protected:
  /** @brief compute the Jacobian elements based on the MW control
  @param[in] sD  the statedata of the current state of the system
  @param[out] ad the matrixData object to store the Jacobian information
  @param[in]  the solverMode corresponding to the stateData
  */
  void MWJac (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  /** @brief compute the Jacobian elements based on the MVar control
  @param[in] sD  the statedata of the current state of the system
  @param[out] ad the matrixData object to store the Jacobian information
  @param[in]  the solverMode corresponding to the stateData
  */
  void MVarJac (const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  /** @brief compute the partial derivatives of the power flows based on the tap angle
  @param[in] busId the id of the calling bus either 1 or 2 or a busID of one of the attached buses
  @param[in] sD  the statedata of the current state of the system
  @param[out] ad the matrixData object to store the Jacobian information
  @param[in]  the solverMode corresponding to the stateData
  */
  void tapAnglePartial (index_t  busId, const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  /** @brief compute the partial derivatives of the power flows based on the tap setting
  @param[in] busId the id of the calling bus either 1 or 2 or a busID of one of the attached buses
  @param[in] sD  the statedata of the current state of the system
  @param[out] ad the matrixData object to store the Jacobian information
  @param[in]  the solverMode corresponding to the stateData
  */
  void tapPartial (index_t busId, const stateData *sD, matrixData<double> &ad, const solverMode &sMode);
  /** @brief do any stepped adjustments  based on voltage control from the power flow calculations
  @return change_code::no_change if nothing was done,  PARAMETER_ADJUSTMENT if the tap changer was stepped
  */
  change_code voltageControlAdjust ();
  /** @brief do any stepped adjustments  based on MW control from the power flow calculations
  @return change_code::no_change if nothing was done,  PARAMETER_ADJUSTMENT if the tap changer was stepped
  */
  change_code MWControlAdjust ();
  /** @brief do any stepped adjustments  based on MVAR control from the power flow calculations
  @return change_code::no_change if nothing was done,  PARAMETER_ADJUSTMENT if the tap changer was stepped
  */
  change_code MVarControlAdjust ();
};

#endif

