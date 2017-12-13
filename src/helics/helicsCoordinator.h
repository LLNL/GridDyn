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

#ifndef _HELICS_COORDINATOR_H_
#define _HELICS_COORDINATOR_H_

#include "core/coreObject.h"

#include "core/coreExceptions.h"
#include "helics/helics.hpp"
#include "helicsSupport.h"
#include "utilities/stringConversion.h"
#include <complex>
#include <map>
#include <mutex>
#include <type_traits>
#include <boost/variant.hpp>

namespace griddyn
{
namespace helicsLib
{
/** simple structure containing the publication information*/
class PubInfo
{
  public:
    helics::helicsType_t type;
    gridUnits::units_t unitType = gridUnits::defUnit;
    std::string name;
};

/** simple structure containing the subscription information*/
class SubInfo
{
  public:
    bool isValid = false;
    gridUnits::units_t unitType = gridUnits::defUnit;
    std::string name;
    helics::defV defaults;
};

/** class to manage the linkages from HELICS to the GridDyn objects*/
class helicsCoordinator : public coreObject
{
  private:
    std::vector<helics::Publication> pubs_;  //!< list of all the publication
    std::vector<helics::Subscription> subs_;  //!< container for all the subscription information
    std::vector<helics::Endpoint> epts;
    std::vector<SubInfo> subI;
    std::vector<PubInfo> pubI;
    helics::FederateInfo info_;  //!< container with the federate information
    std::string connectionInfo;  //!< string with the connection info
    std::shared_ptr<helics::ValueFederate> vFed_;  //!< pointer to the federate
    std::shared_ptr<helics::MessageFederate> mFed_;  //!< pointer to a message Federate
    std::map<std::string, int32_t> subMap_;  //!< map of all the subscription names
    std::map<std::string, int32_t> pubMap_;  //!< map of all the publication names
  public:
    explicit helicsCoordinator (const std::string &fedName = "");

    /** register the information as part of a federate in HELICS
    @return a shared pointer to the federate object itself
    */
    std::shared_ptr<helics::Federate> RegisterAsFederate ();

    template <class ValueType>
    void setValue (int32_t index, const ValueType &val)
    {
        if (isValidIndex (index, pubs_))
        {
            auto &pub = pubs_[index];
            pub.publish(val);
        }
        throw (invalidParameterValue ());
    }

    template <class ValueType>
    ValueType getValueAs (int32_t index)
    {
        if (isValidIndex (index, subs_))
        {
            auto &sub = subs_[index];
            return sub.getValue<ValueType> ();
        }
        throw (invalidParameterValue ());
    }

    void addHelper (std::shared_ptr<helperObject> ho) override;
    void set (const std::string &param, const std::string &val) override;
    void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    void setFlag (const std::string &flag, bool val) override;
    /** add a publication to the helics federate
    @param[in] pubName the name of the value to publish
    @param[in] type  the type of value one of helicsValueType
    @param[in] unitType the units of the publication
    @return an identifier value for the publication
    */
    int32_t addPublication (const std::string &pubName,
                            helics::helicsType_t type,
                            gridUnits::units_t unitType = gridUnits::defUnit);
    /** update a publication
    @param[in] index the identifier for the publication
    @param[in] pubName the name of the value to publish
    @param[in] type  the type of value one of helicsValueType
    @param[in] unitType the units of the publication
    */
    void updatePublication (int32_t index,
                            const std::string &pubName,
                            helics::helicsType_t type,
                            gridUnits::units_t unitType = gridUnits::defUnit);
    /** add a subscription to the helics federate
    @param[in] pubName the name of the value to subscribe to
    @param[in] unitType the units of the publication
    @return an identifier value for the publication
    */
    int32_t addSubscription (const std::string &pubName,
                             gridUnits::units_t unitType = gridUnits::defUnit);
    /** update a subscription
    @param[in] index the identifier for the subscription
    @param[in] pubName the name of the value to subscribe to
    @param[in] unitType the units of the publication
    */
    void updateSubscription (int32_t index,
                             const std::string &subName,
                             gridUnits::units_t unitType = gridUnits::defUnit);

    template <class ValueType>
    void setDefault (int32_t index, const ValueType &val)
    {
        if (isValidIndex (index, subs_))
        {
            auto &sub = subs_[index];
            sub.setDefault (val);
        }
        else if (isValidIndex (index, subI))
        {
            auto &sub = subI[index];
            sub.defaults = val;
        }
        throw (invalidParameterValue ());
    }

  public:
    /** lookup an identifier for a previously declared subscription
    @return the subscription index or -1 if not existent*/
    int32_t getSubscriptionIndex (const std::string &name) const;

    /** lookup an identifier for a previously declared publication
    @return the publication index or -1 if not existent*/
    int32_t getPublicationIndex (const std::string &name) const;

    /** finalize the federate and disconnect from Helics*/
    void finalize ();

    /** check whether a value has been updated*/
    bool isUpdated (int32_t index) const;
};

}  // namespace helics
}  // namespace griddyn
#endif
