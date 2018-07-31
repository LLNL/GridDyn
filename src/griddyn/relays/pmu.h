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

#ifndef GRIDDYN_PMU_H_
#define GRIDDYN_PMU_H_

#include "sensor.h"

namespace griddyn
{
namespace relays
{
/** @brief class modeling a PMU
 */
class pmu : public sensor
{
  public:
    enum pmu_flags
    {
        transmit_active = object_armed_flag,  //!< flag to indicate that the relay is transmitting
        three_phase_active = three_phase_only,  //!< flag indicating 3 phase values
        three_phase_set = three_phase_capable,  //!< flag indicating that the 3-phase value was user set vs default
        current_active = object_flag12,  //!< flag indicating that the current measurements are active
    };

  protected:
    coreTime transmissionPeriod = 1.0 / 30.0;  //!< the rate of data transmission
    parameter_t Tv = 0.05;  //!< filter time constant for the voltage measurement
    parameter_t Ttheta = 0.05;  //!< filter time constant for the angle measurement
    parameter_t Tcurrent = 0.05;  //!< filter time constant for the current measurement
    parameter_t Trocof = 0.05;  //!< filter time constant for computing the ROCOF
    parameter_t sampleRate = 720.0;  //!< [Hz] the actual sample time
  private:
    coreTime nextTransmitTime = maxTime;  //!< the time of the next transmission
    coreTime lastTransmitTime = negTime;  //!< the time of the last transmission
  public:
    pmu (const std::string &objName = "pmu_$");
    coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void setFlag (const std::string &flag, bool val = true) override;
    virtual void set (const std::string &param, const std::string &val) override;

    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

    virtual void updateA (coreTime time) override;
    virtual coreTime updateB () override;

  private:
    /** create the appropriate output names based on the settings*/
    void generateOutputNames ();
    /** generate the filter blocks and inputs for the sensor object*/
    void createFilterBlocks ();
    /** generate a control message packet with the PMU and send it if there is an existing communicator*/
    void generateAndTransmitMessage () const;
};

}  // namespace relays
}  // namespace griddyn
#endif
