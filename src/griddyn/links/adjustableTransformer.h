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

#ifndef ADJUSTABLE_LINKS_H_
#define ADJUSTABLE_LINKS_H_

#include "acLine.h"

namespace griddyn
{
namespace links
{
/** @brief extends the link class to include adjustments to the tap and tapAngle
*  principally a model for adjustable transformers such as ULTC or regulators or other types of transformers with
adjustments.
it implements three types of control in power flow voltage control of local or remote buses,  local reactive power
flow, and local real power flow the real power flow is the only one that adjusts the angle.  available control
include stepped and continuous

*/
class adjustableTransformer : public acLine
{
  public:
    /** @brief  enumeration of the available control types
     */
    enum class control_mode_t
    {
        manual_control = 0,  //!< no automatic adjustments
        voltage_control = 1,  //!< automatic control based on voltage
        MW_control = 2,  //!< automatic control based on real power flow at a specific terminal
        MVar_control = 3  //!< automatic control based on real power flow at a specific terminal
    };
    /** @brief  flags for
     */
    enum adjustable_flags
    {
        continuous_flag = object_flag5,  //!< flag indicating continuous adjustments
        use_target_mode = object_flag6,  //!< flag indicating target mode
        at_limit = object_flag7,  //!< flag indicating the adjustments are at their limit
        no_pFlow_adjustments = object_flag8,  //!< flag turning off all automatic adjustments
        use_lookup_table = object_flag9,  //!< flag indicating use of an impedance lookup table
    };

  protected:
    control_mode_t cMode = control_mode_t::manual_control;  //!< control Mode
    parameter_t stepSize = 0.01;  //!< step size of the adjustment for non-continuous adjustments
    parameter_t maxTapAngle = kPI / 4.0;  //!< maximum tap angle
    parameter_t minTapAngle = -kPI / 4.0;  //!< minimum tap angle
    parameter_t minTap = 0.9;  //!< minimum tap setting
    parameter_t maxTap = 1.1;  //!< maximum tap setting
    parameter_t Vtarget = -kBigNum;  //!< target voltage
    parameter_t Vmax = kBigNum;  //!< maximum voltage before changing the tap
    parameter_t Vmin = 0.0;  //!< minimum voltage before changing the tap

    parameter_t Ptarget = -kBigNum;  //!< the target power flow
    parameter_t Pmin = -kBigNum;  //!< the minimum power level before changing the tap
    parameter_t Pmax = kBigNum;  //!< the maximum power before changing the tap

    parameter_t Qtarget = -kBigNum;  //!< the target reactive power flow
    parameter_t Qmax = kBigNum;  //!< the minimum power level before changing the tap
    parameter_t Qmin = -kBigNum;  //!< the maximum power before changing the tap
    // double Tm;				//!< time constant
    // double Td;				//!< time constant
    double direction = 1;  //!< variable storing whether the directional derivate of the tap changes with respect
                           //!< to voltage or power is positive or negative
    parameter_t tapMaxChangeRate = kBigNum;  //!< maximum rate at which the tap can change
    parameter_t sample_rate = 4.0;  //!< [s]the rate at which the measurements are sampled
    gridBus *controlBus = nullptr;  //!< the control bus to monitor voltage

    double tap0 = 0.0;  //!< baseline tap position used for continuous tap settings
    double tapAngle0 = 0.0;  //!< baseline tapAngle position used for continuous tap settings
    parameter_t stepDelay =
      30;  //!< step control for adjusting the quantity or the time constant for continuous system
    parameter_t mp_Tm = 0.05;  //!< time constant for continuous tap settings
    parameter_t dTapdt = 0;  //!< rate of change of the tap
    parameter_t dTapAdt = 0;  //!< rate of change of the tapAngle
  private:
    int controlNum = -1;  //!< the control bus and number setting are not fully determined until initialization so
                          //!< this stores information from the startup phase
    std::string controlName;  //!< the control bus and number setting are not fully determined until initialization
                              //!< so this stores information from the startup phase
    count_t adjCount = 0;
    count_t oCount = 0;
    double prevAdjust = 0.0;
    double prevValue = 0.0;
    // double baseValue;
  public:
    explicit adjustableTransformer (const std::string &objName = "adjTX_$");
    /** @brief default constructor
    @param[in] rP  resistance of the link
    @param[in] xP  reactance of the link forwarded to the Link constructor
    */
    adjustableTransformer (double rP, double xP, const std::string &objName = "adjTX_$");
    //!< @brief destructor
    coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;
    void set (const std::string &param, const std::string &val) override;
    void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
    // adjuster specific functions
    /**@ brief set the control bus to a specified bus pointer
    @param[in] cBus  the specified control Bus*/
    void setControlBus (gridBus *cBus);
    /**@ brief set the control bus to a specified bus number
    @param[in] busNumber-- this can be 1 or 2 for already attached buses or the user id of a bus in which cases the
    parent of the link is searched for the bus*/
    void setControlBus (index_t busNumber = 2);

    change_code powerFlowAdjust (const IOdata &inputs, std::uint32_t flags, check_level_t level) override;
    void reset (reset_levels level) override;

    void updateLocalCache () override;
    void updateLocalCache (const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;

    void jacobianElements (const IOdata &inputs,
                           const stateData &sD,
                           matrixData<double> &md,
                           const IOlocs &inputLocs,
                           const solverMode &sMode) override;
    // for computing all the Jacobian elements at once

    using acLine::ioPartialDerivatives;
    virtual void ioPartialDerivatives (id_type_t busId,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const IOlocs &inputLocs,
                                       const solverMode &sMode) override;
    virtual void outputPartialDerivatives (const IOdata &inputs,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual void outputPartialDerivatives (id_type_t busId,
                                           const stateData &sD,
                                           matrixData<double> &md,
                                           const solverMode &sMode) override;
    virtual count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;

    void residual (const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
    void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;
    void guessState (coreTime time, double state[], double dstate_dt[], const solverMode &sMode) override;
    virtual stateSizes LocalStateSizes (const solverMode &sMode) const override;
    virtual count_t LocalJacobianCount (const solverMode &sMode) const override;

  protected:
    void pFlowObjectInitializeA (coreTime time0, std::uint32_t flags) override;
    void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  public:
    void rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
    void rootTrigger (coreTime time,
                      const IOdata &inputs,
                      const std::vector<int> &rootMask,
                      const solverMode &sMode) override;
    virtual void followNetwork (int network, std::queue<gridBus *> &stk) override;
    virtual void
    getStateName (stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;

  protected:
    /** @brief compute the Jacobian elements based on the MW control
    @param[in] sD  the stateData of the current state of the system
    @param[out] md the matrixData object to store the Jacobian information
    @param[in]  the solverMode corresponding to the stateData
    */
    void MWJac (const stateData &sD, matrixData<double> &md, const solverMode &sMode);
    /** @brief compute the Jacobian elements based on the MVar control
    @param[in] sD  the stateData of the current state of the system
    @param[out] md the matrixData object to store the Jacobian information
    @param[in]  the solverMode corresponding to the stateData
    */
    void MVarJac (const stateData &sD, matrixData<double> &md, const solverMode &sMode);
    /** @brief compute the partial derivatives of the power flows based on the tap angle
    @param[in] busId the id of the calling bus either 1 or 2 or a busID of one of the attached buses
    @param[in] sD  the stateData of the current state of the system
    @param[out] md the matrixData object to store the Jacobian information
    @param[in]  the solverMode corresponding to the stateData
    */
    void tapAnglePartial (index_t busId, const stateData &sD, matrixData<double> &md, const solverMode &sMode);
    /** @brief compute the partial derivatives of the power flows based on the tap setting
    @param[in] busId the id of the calling bus either 1 or 2 or a busID of one of the attached buses
    @param[in] sD  the stateData of the current state of the system
    @param[out] md the matrixData object to store the Jacobian information
    @param[in]  the solverMode corresponding to the stateData
    */
    void tapPartial (index_t busId, const stateData &sD, matrixData<double> &md, const solverMode &sMode);
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

}  // namespace links
}  // namespace griddyn
#endif
