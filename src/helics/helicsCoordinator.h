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

#include "helics/application_api/application_api.h"
#include "core/coreExceptions.h"
#include "utilities/stringConversion.h"
#include "helicsSupport.h"
#include <mutex>
#include <complex>
#include <type_traits>
#include <boost/variant.hpp>
#include <map>

namespace griddyn
{
namespace helicsLib
{
//defining a boost variant for the different data types possible
using defV = boost::variant<std::string, double, int64_t, std::complex<double>>;

/** simple structure containing the publication information*/
class PubInfo
{
public:
	helicsValueType type;
	gridUnits::units_t unitType = gridUnits::defUnit;
	helics::publication_id_t id=helics::invalid_id_value;
	std::string name;
	
};

/** simple structure containing the subscription information*/
class SubInfo
{
public:
	helicsValueType type;
	bool isValid = false;
	gridUnits::units_t unitType = gridUnits::defUnit;
	helics::subscription_id_t id = helics::invalid_id_value;
	std::string name;
	defV defaults;
};


/** class to manage the linkages from HELICS to the GridDyn objects*/
class helicsCoordinator : public coreObject
{
private:
	std::vector<PubInfo> pubs_;  //!< list of all the publication
	std::vector<SubInfo> subs_;	//!< container for all the subscription information
	helics::FederateInfo info_;	//!< container with the federate information
	std::string connectionInfo;	//!< string with the connection info
	std::shared_ptr<helics::ValueFederate> vFed_;	//!< pointer to the federate
	std::shared_ptr<helics::MessageFederate> mFed_; //!< pointer to a message Federate
	std::map<std::string, int32_t> subMap_;  //!< map of all the subscription names
	std::map<std::string, int32_t> pubMap_;  //!< map of all the publication names
public:

	explicit helicsCoordinator(const std::string &fedName = "");

	/** register the information as part of a federate in HELICS
	@return a shared pointer to the federate object itself
	*/
	std::shared_ptr<helics::Federate> RegisterAsFederate();

	void setValue(int32_t index, const std::string &val);
	void setValue(int32_t index, const std::vector<double> &val);
	void setValue(int32_t index, std::complex<double> val);

	template<class ValueType>
	void setValue(int32_t index, ValueType val)
	{
		if ((index >= 0) && (index < static_cast<int32_t>(pubs_.size())))
		{
			auto &pub = pubs_[index];
			switch (pub.type)
			{
			case helicsValueType::helicsString:
				vFed_->publish(pub.id, std::to_string(val));
				break;
			case helicsValueType::helicsDouble:
				vFed_->publish(pub.id, static_cast<double>(val));
				break;
			case helicsValueType::helicsInteger:
				vFed_->publish(pub.id, static_cast<int64_t>(val));
				break;
			case helicsValueType::helicsComplex:
				vFed_->publish(pub.id, std::complex<double>(static_cast<double>(val), 0.0));
				break;
			case helicsValueType::helicsVector:
				vFed_->publish(pub.id, std::vector<double>{static_cast<double>(val)});
				break;
			default:
				throw(invalidParameterValue());
				break;
			}
			return;
		}
		throw(invalidParameterValue());
	}
	

	template<class ValueType>
	ValueType getValueAs(int32_t index)
	{
		if ((index >= 0) && (index < static_cast<int32_t>(subs_.size())))
		{
			auto &sub = subs_[index];
			if (!sub.isValid)
			{
				if (vFed_->isUpdated(sub.id))
				{
					sub.isValid = true;
				}
				else
				{
					return static_cast<ValueType>(kNullVal);
				}
			}
				switch (sub.type)
				{
				case helicsValueType::helicsString:
					return numeric_conversion<ValueType>(vFed_->getValue<std::string>(sub.id), ValueType(0));
				case helicsValueType::helicsDouble:
					return static_cast<ValueType>(vFed_->getValue<double>(sub.id));
				case helicsValueType::helicsInteger:
					return static_cast<ValueType>(vFed_->getValue<int64_t>(sub.id));
				case helicsValueType::helicsComplex:
					return static_cast<ValueType>(std::abs(vFed_->getValue<std::complex<double>>(sub.id)));
				case helicsValueType::helicsVector:
				{
					auto V = vFed_->getValue<std::vector<double>>(sub.id);
					return (!V.empty() ? static_cast<ValueType>(V.front()) : kNullVal);
				}
				default:
					throw(invalidParameterValue());
				}
			
		}
		throw(invalidParameterValue());
	}

	
	void addHelper(std::shared_ptr<helperObject> ho) override;
	void set(const std::string &param, const std::string &val) override;
	void set(const std::string &param, double val, gridUnits::units_t unitType=gridUnits::defUnit) override;
	void setFlag(const std::string &flag, bool val) override;
	/** add a publication to the helics federate
	@param[in] pubName the name of the value to publish
	@param[in] type  the type of value one of helicsValueType
	@param[in] unitType the units of the publication
	@return an identifier value for the publication
	*/
	int32_t addPublication(const std::string &pubName, helicsValueType type, gridUnits::units_t unitType=gridUnits::defUnit);
	/** update a publication
	@param[in] index the identifier for the publication
	@param[in] pubName the name of the value to publish
	@param[in] type  the type of value one of helicsValueType
	@param[in] unitType the units of the publication
	*/
	void updatePublication(int32_t index, const std::string &pubName, helicsValueType type, gridUnits::units_t unitType = gridUnits::defUnit);
	/** add a subscription to the helics federate
	@param[in] subName the name of the value to subscribe to
	@param[in] type  the type of value one of helicsValueType
	@param[in] unitType the units of the publication
	@return an identifier value for the publication
	*/
	int32_t addSubscription(const std::string &subName, helicsValueType type, gridUnits::units_t unitType = gridUnits::defUnit);
	/** update a subscription
	@param[in] index the identifier for the publication
	@param[in] pubName the name of the value to publish
	@param[in] type  the type of value one of helicsValueType
	@param[in] unitType the units of the publication
	*/
	void updateSubscription(int32_t index, const std::string &pubName, helicsValueType type, gridUnits::units_t unitType = gridUnits::defUnit);

	void setDefault(int32_t index, double val);
	void setDefault(int32_t index, const std::string &val);
	void setDefault(int32_t index, std::complex<double> val);
	void setDefault(int32_t index, int64_t val);
	private:
	void sendDefault(const SubInfo &sub);
	public:
	/** lookup an indentifier for a previously declared subscription
	@return the subscription index or -1 if not existant*/
	int32_t getSubscriptionIndex(const std::string &name) const;

	/** lookup an indentifier for a previously declared publication
	@return the publication index or -1 if not existant*/
	int32_t getPublicationIndex(const std::string &name) const;

	/** finalize the federate and disconnect from Helics*/
	void finalize();

	/** check whether a value has been updated*/
	bool isUpdated(int32_t index) const;
};

/** declaring some template overloads */
template<>
std::string helicsCoordinator::getValueAs(int32_t index);
template<>
std::vector<double> helicsCoordinator::getValueAs(int32_t index);
template<>
std::complex<double> helicsCoordinator::getValueAs(int32_t index);


}//namespace fmi
}//namespace griddyn
#endif

