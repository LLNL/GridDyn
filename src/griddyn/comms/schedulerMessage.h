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

#pragma once
#include "commMessage.h"

#include <cereal/types/vector.hpp>

#define BASE_SCHEDULER_MESSAGE_NUMBER 800
namespace griddyn
{
namespace comms
{
class schedulerMessagePayload : public CommPayload
{
  public:
    enum scheduler_message_type_t : std::uint32_t
    {
        CLEAR_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 3,
        SHUTDOWN = BASE_SCHEDULER_MESSAGE_NUMBER + 4,
        STARTUP = BASE_SCHEDULER_MESSAGE_NUMBER + 5,
        ADD_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 6,
        UPDATE_TARGETS = BASE_SCHEDULER_MESSAGE_NUMBER + 7,
        UPDATE_RESERVES = BASE_SCHEDULER_MESSAGE_NUMBER + 8,
        UPDATE_REGULATION_RESERVE = BASE_SCHEDULER_MESSAGE_NUMBER + 9,
        USE_RESERVE = BASE_SCHEDULER_MESSAGE_NUMBER + 10,
        UPDATE_REGULATION_TARGET = BASE_SCHEDULER_MESSAGE_NUMBER + 11,
        REGISTER_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 12,
        REGISTER_AGC_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 13,
        REGISTER_RESERVE_DISPATCHER = BASE_SCHEDULER_MESSAGE_NUMBER + 14,
        REGISTER_CONTROLLER = BASE_SCHEDULER_MESSAGE_NUMBER + 15,
    };

    schedulerMessagePayload () = default;

    schedulerMessagePayload (std::vector<double> time, std::vector<double> target);

    void loadMessage (std::vector<double> time, std::vector<double> target);

    std::vector<double> m_time;
    std::vector<double> m_target;

    virtual std::string to_string (uint32_t type, uint32_t code) const override;
    virtual void from_string (uint32_t type, uint32_t code, const std::string &fromString, size_t offset) override;

  private:
    friend class cereal::access;
    template <class Archive>
    void serialize (Archive &ar)
    {
        ar (cereal::base_class<CommPayload> (this), m_time, m_target);
    }

    std::string makeTargetString (size_t cnt) const;
};

}  // namespace comms
}  // namespace griddyn

CEREAL_REGISTER_TYPE (griddyn::comms::schedulerMessagePayload)