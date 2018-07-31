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

#ifndef INFINITEBUS_H_
#define INFINITEBUS_H_

#include "../gridBus.h"
namespace griddyn
{
/** @brief an infinite bus object with fixed voltage and angle
  this type of bus can set a voltage, angle and frequency on the system and maintain it regardless of all other
considerations on the model it can supply infinite real and reactive power  it also include ramp parameters to
change the voltage and frequency( and hence angle) over time. The class overrides all math operations and does
nothing since there are no calculations or states in an infinite bus except to update the parameters for the
scheduled change in voltage and frequency and hence angle, objects attached to it are ignored
*/
class infiniteBus : public gridBus
{
  protected:
    parameter_t dvdt = 0;  //!< [puV/s] ramp rate for voltage
    parameter_t dfdt = 0;  //!< [puHz/s] ramp rate for frequency
  public:
    /** @brief default constructor
     *@param[in] objName  the name of the infinite bus object
     */
    explicit infiniteBus (const std::string &objName = "infbus_$");
    /** @brief default constructor
     *@param[in] startVoltage  the initial voltage of the bus
     *@param[in] startAngle  the initial angle of the bus
     *@param[in] objName  the name of the infinite bus object
     */
    infiniteBus (double startVoltage, double startAngle, const std::string &objName = "infbus_$");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    // add components
    void timestep (coreTime time, const IOdata &inputs, const solverMode &sMode) override;
    void
    setState (coreTime time, const double state[], const double dstate_dt[], const solverMode &sMode) override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual bool checkCapable () override;

    virtual double getVoltage (const double state[], const solverMode &sMode) const override;
    virtual double getAngle (const double state[], const solverMode &sMode) const override;
    virtual double getVoltage (const stateData &sD, const solverMode &sMode) const override;
    virtual double getAngle (const stateData &sD, const solverMode &sMode) const override;
    virtual double getFreq (const stateData &sD, const solverMode &sMode) const override;

  protected:
    /** update the Voltage and Angle based on time using the defined ramp rates*/
    void updateVoltageAngle (coreTime time);
};

}  // namespace griddyn
#endif
