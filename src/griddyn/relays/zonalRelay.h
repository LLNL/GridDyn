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
#ifndef ZONAL_RELAY_H_
#define ZONAL_RELAY_H_

#include "../Relay.h"

namespace griddyn
{
namespace relays
{
/** class building off of Relay to define a zonal relays
 the number of zones is arbitrary and it works by checking the impedances of the associated link and comparing to
specific thresholds. This zonal relays runs off a single impedance number
*/
class zonalRelay : public Relay
{
  public:
    enum zonalrelay_flags
    {
        nondirectional_flag = object_flag10,
    };

  protected:
    count_t m_zones = 2;  //!< the number of zones for the relay
    index_t m_terminal =
      1;  //!< the side of the line to connect 1=from side 2=to side, 3+ for multiterminal devices
    double m_resetMargin = 0.01;  //!<! the reset margin for clearing a fault
    std::vector<double> m_zoneLevels;  //!< the level of impedance to trigger
    std::vector<coreTime> m_zoneDelays;  //!< the delay upon which to act for the relay
    count_t m_condition_level = kInvalidCount;  //!< the level of condition that has been triggered
    int autoName = -1;  //!< storage for indicator of the type of autoname to use
  public:
    explicit zonalRelay (const std::string &objName = "zonalRelay_$");
    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    virtual void setFlag (const std::string &flag, bool val = true) override;
    virtual void set (const std::string &param, const std::string &val) override;

    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
    virtual void dynObjectInitializeA (coreTime time0, std::uint32_t flags) override;

  protected:
    virtual void
    actionTaken (index_t ActionNum, index_t conditionNum, change_code actionReturn, coreTime actionTime) override;
    virtual void conditionTriggered (index_t conditionNum, coreTime triggerTime) override;
    virtual void conditionCleared (index_t conditionNum, coreTime triggerTime) override;
    virtual void receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message) override;
    /** function to automatically generate the comm system names
    @param[in] code  a code value representing the method of generating the name
    @return the generated name
    */
    std::string generateAutoName (int code);

    virtual std::string generateCommName () override;
};
}  // namespace relays
}  // namespace griddyn
#endif
