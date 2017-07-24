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

#include "helicsCoordinator.h"
#include "helics/application_api/application_api.h"

#include "utilities/stringConversion.h"
#include <algorithm>
#include <utilities/mapOps.hpp>

namespace griddyn
{
namespace helicsLib
{
helicsCoordinator::helicsCoordinator (const std::string &fedName) : coreObject ("helics"), info_ (fedName) {}

std::shared_ptr<helics::Federate> helicsCoordinator::RegisterAsFederate ()
{
    if (info_.name.empty ())
    {
        info_.name = getParent ()->getName ();
    }

    vFed_ = std::make_shared<helics::ValueFederate> (info_);

    for (auto &p : pubs_)
    {
        if (p.unitType != gridUnits::defUnit)
        {
            p.id =
              vFed_->registerGlobalPublication (p.name, to_string (p.type), gridUnits::to_string (p.unitType));
        }
        else
        {
            p.id = vFed_->registerGlobalPublication (p.name, to_string (p.type));
        }
    }
    for (auto &s : subs_)
    {
        if (s.unitType != gridUnits::defUnit)
        {
            s.id =
              vFed_->registerOptionalSubscription (s.name, to_string (s.type), gridUnits::to_string (s.unitType));
        }
        else
        {
            s.id = vFed_->registerOptionalSubscription (s.name, to_string (s.type));
        }
    }
    vFed_->enterInitializationState ();
    return vFed_;
}

void helicsCoordinator::setFlag (const std::string &flag, bool val)
{
    if (flag == "source_only")
    {
        info_.sourceOnly = val;
    }
    else if (flag == "uninterruptible")
    {
        info_.uninterruptible = val;
    }
    else if (flag == "observer")
    {
        info_.observer = val;
    }
    else if (flag == "time_agnostic")
    {
        info_.timeAgnostic = val;
    }
    else
    {
        coreObject::setFlag (flag, val);
    }
}
void helicsCoordinator::set (const std::string &param, const std::string &val)
{
    if ((param == "corename") || (param == "core_name"))
    {
        info_.coreName = val;
    }
    else if ((param == "initstring") || (param == "init") || (param == "core_init"))
    {
        info_.coreInitString = val;
    }
    else if ((param == "coretype") || (param == "core_type"))
    {
        info_.coreType = val;
    }
    else if ((param == "name") || (param == "fed_name"))
    {
        info_.name = val;
    }
    else if ((param == "broker") || (param == "connection_info"))
    {
        connectionInfo = val;
    }
    else
    {
        coreObject::set (param, val);
    }
}

void helicsCoordinator::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    if (param == "timedelta")
    {
        info_.timeDelta = val;
    }
    else if (param == "lookahead")
    {
        info_.lookAhead = val;
    }
    else if (param == "impactwindow")
    {
        info_.impactWindow = val;
    }
    else
    {
        coreObject::set (param, val, unitType);
    }
}

void helicsCoordinator::setValue (int32_t index, const std::string &val)
{
    if ((index >= 0) && (index < static_cast<int> (pubs_.size ())))
    {
        auto pub = pubs_[index];
        if (pub.type == helicsValueType::helicsString)
        {
            vFed_->publish (pub.id, val);
            return;
        }
    }
    throw (invalidParameterValue ());
}

void helicsCoordinator::setValue (int32_t index, const std::vector<double> &val)
{
    if ((index >= 0) && (index < static_cast<int> (pubs_.size ())))
    {
        auto pub = pubs_[index];
        if (pub.type == helicsValueType::helicsVector)
        {
            vFed_->publish (pub.id, val);
            return;
        }
        else if (pub.type == helicsValueType::helicsComplex)
        {
            if (val.size () == 2)
            {
                vFed_->publish (pub.id, std::complex<double> (val[0], val[1]));
                return;
            }
        }
    }
    throw (invalidParameterValue ());
}
void helicsCoordinator::setValue (int32_t index, std::complex<double> val)
{
    if ((index >= 0) && (index < static_cast<int> (pubs_.size ())))
    {
        auto pub = pubs_[index];
        switch (pub.type)
        {
        case helicsValueType::helicsDouble:
            vFed_->publish (pub.id, std::abs (val));
            return;
        case helicsValueType::helicsString:
            vFed_->publish (pub.id, helicsComplexString (val));
            return;
        case helicsValueType::helicsComplex:
            vFed_->publish (pub.id, val);
            return;
        case helicsValueType::helicsVector:
            vFed_->publish (pub.id, std::vector<double>{val.real (), val.imag ()});
            return;
        case helicsValueType::helicsInteger:
            vFed_->publish (pub.id, static_cast<int64_t> (std::abs (val)));
            return;
        default:
            throw (invalidParameterValue ());
        }
    }
    throw (invalidParameterValue ());
}

bool helicsCoordinator::isUpdated (int32_t index) const
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        return (vFed_) ? vFed_->isUpdated (subs_[index].id) : false;
    }
    return false;
}

template <>
std::string helicsCoordinator::getValueAs (int32_t index)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto &sub = subs_[index];
        if (!sub.isValid)
        {
            if (vFed_->isUpdated (sub.id))
            {
                sub.isValid = true;
            }
            else
            {
                return "";
            }
        }
        switch (sub.type)
        {
        case helicsValueType::helicsString:
            return vFed_->getValue<std::string> (sub.id);
        case helicsValueType::helicsDouble:
            return std::to_string (vFed_->getValue<double> (sub.id));
        case helicsValueType::helicsInteger:
            return std::to_string (vFed_->getValue<int64_t> (sub.id));
        case helicsValueType::helicsComplex:
            return helicsComplexString (vFed_->getValue<std::complex<double>> (sub.id));
        case helicsValueType::helicsVector:
        {
            std::string vals = "[";
            for (auto &v : vFed_->getValue<std::vector<double>> (sub.id))
            {
                vals += std::to_string (v) + ", ";
            }
            if (vals.size () > 2)
            {
                vals.pop_back ();
                vals.pop_back ();
            }
            vals.push_back (']');
            return vals;
        }
        default:
            throw (invalidParameterValue ());
        }
    }
    throw (invalidParameterValue ());
}

template <>
std::vector<double> helicsCoordinator::getValueAs (int32_t index)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto &sub = subs_[index];
        if (!sub.isValid)
        {
            if (vFed_->isUpdated (sub.id))
            {
                sub.isValid = true;
            }
            else
            {
                return {};
            }
        }
        switch (sub.type)
        {
        case helicsValueType::helicsDouble:
            return std::vector<double>{vFed_->getValue<double> (sub.id)};
        case helicsValueType::helicsInteger:
            return std::vector<double>{static_cast<double> (vFed_->getValue<int64_t> (sub.id))};
        case helicsValueType::helicsComplex:
        {
            auto val = vFed_->getValue<std::complex<double>> (sub.id);
            return std::vector<double>{val.real (), val.imag ()};
        }
        case helicsValueType::helicsVector:
            return vFed_->getValue<std::vector<double>> (sub.id);
        case helicsValueType::helicsString:
        {
            auto str = vFed_->getValue<std::string> (sub.id);
            str = stringOps::removeBrackets (str);
            return str2vector<double> (str, kNullVal);
        }
        default:
            throw (invalidParameterValue ());
        }
    }
    throw (invalidParameterValue ());
}

template <>
std::complex<double> helicsCoordinator::getValueAs (int32_t index)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto &sub = subs_[index];
        if (!sub.isValid)
        {
            if (vFed_->isUpdated (sub.id))
            {
                sub.isValid = true;
            }
            else
            {
                return std::complex<double> (kNullVal, 0.0);
            }
        }

        switch (sub.type)
        {
        case helicsValueType::helicsDouble:
            return std::complex<double>{vFed_->getValue<double> (sub.id), 0.0};
        case helicsValueType::helicsInteger:
            return std::complex<double>{static_cast<double> (vFed_->getValue<int64_t> (sub.id)), 0.0};
        case helicsValueType::helicsVector:
        {
            auto val = vFed_->getValue<std::vector<double>> (sub.id);
            if (val.size () == 2)
            {
                return std::complex<double>{val[0], val[1]};
            }
            else
            {
                throw (invalidParameterValue ());
            }
        }
        case helicsValueType::helicsComplex:
            return vFed_->getValue<std::complex<double>> (sub.id);
        case helicsValueType::helicsString:
            return helicsGetComplex (vFed_->getValue<std::string> (sub.id));
        default:
            throw (invalidParameterValue ());
        }
    }
    throw (invalidParameterValue ());
}

int32_t
helicsCoordinator::addPublication (const std::string &pubName, helicsValueType type, gridUnits::units_t unitType)
{
    PubInfo p;
    p.name = pubName;
    p.type = type;
    p.unitType = unitType;
    pubs_.push_back (p);
    auto ind = static_cast<int32_t> (pubs_.size ()) - 1;
    pubMap_.emplace (pubName, ind);
    return ind;
}

int32_t
helicsCoordinator::addSubscription (const std::string &subName, helicsValueType type, gridUnits::units_t unitType)
{
    SubInfo s;
    s.name = subName;
    s.type = type;
    s.unitType = unitType;
    subs_.push_back (s);
    auto ind = static_cast<int32_t> (subs_.size ()) - 1;
    subMap_.emplace (subName, ind);
    return ind;
}

void helicsCoordinator::updatePublication (int32_t index,
                                           const std::string &pubName,
                                           helicsValueType type,
                                           gridUnits::units_t unitType)
{
    if ((index >= 0) && (index < static_cast<int32_t> (pubs_.size ())))
    {
        if (!pubName.empty ())
        {
            pubs_[index].name = pubName;
        }
        pubs_[index].type = type;
        if (unitType != gridUnits::defUnit)
        {
            pubs_[index].unitType = unitType;
        }
    }
}

void helicsCoordinator::updateSubscription (int32_t index,
                                            const std::string &subName,
                                            helicsValueType type,
                                            gridUnits::units_t unitType)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        if (!subName.empty ())
        {
            subs_[index].name = subName;
        }
        subs_[index].type = type;
        if (unitType != gridUnits::defUnit)
        {
            subs_[index].unitType = unitType;
        }
    }
}

void helicsCoordinator::addHelper (std::shared_ptr<helperObject> /*ho*/)
{
    // std::lock_guard<std::mutex> hLock(helperProtector);
    // helpers.push_back(std::move(ho));
}

template <class X>
X getDefVal (const defV &v)
{
    (void)(v);
}

template <>
std::string getDefVal (const defV &v)
{
    switch (v.which ())
    {
    case 0:
    default:
        return boost::get<std::string> (v);
    case 1:
        return std::to_string (boost::get<double> (v));
    case 2:
        return std::to_string (boost::get<int64_t> (v));
    case 3:
        return helicsComplexString (boost::get<std::complex<double>> (v));
    }
}

template <>
int64_t getDefVal (const defV &v)
{
    switch (v.which ())
    {
    case 0:
    default:
        return numeric_conversion<int64_t> (boost::get<std::string> (v), 0x0FFF'FFFF);
    case 1:
        return static_cast<int64_t> (boost::get<double> (v));
    case 2:
        return boost::get<int64_t> (v);
    case 3:
        return static_cast<int64_t> (std::abs (boost::get<std::complex<double>> (v)));
    }
}

template <>
double getDefVal (const defV &v)
{
    switch (v.which ())
    {
    case 0:
    default:
        return numeric_conversion<double> (boost::get<std::string> (v), kNullVal);
    case 1:
        return boost::get<double> (v);
    case 2:
        return static_cast<double> (boost::get<int64_t> (v));
    case 3:
        return std::abs (boost::get<std::complex<double>> (v));
    }
}

template <>
std::complex<double> getDefVal (const defV &v)
{
    switch (v.which ())
    {
    case 0:
    default:
        return helicsGetComplex (boost::get<std::string> (v));
    case 1:
        return std::complex<double> (boost::get<double> (v), 0.0);
    case 2:
        return std::complex<double> (static_cast<double> (boost::get<int64_t> (v)), 0.0);
    case 3:
        return boost::get<std::complex<double>> (v);
    }
}

template <>
std::vector<double> getDefVal (const defV &v)
{
    switch (v.which ())
    {
    case 0:
    default:
    {
        auto str = boost::get<std::string> (v);
        str = stringOps::removeBrackets (str);
        return str2vector<double> (str, kNullVal);
    }
    case 1:
        return std::vector<double> (boost::get<double> (v));
    case 2:
        return std::vector<double> (static_cast<double> (boost::get<int64_t> (v)));
    case 3:
    {
        auto vc = boost::get<std::complex<double>> (v);
        return std::vector<double>{vc.real (), vc.imag ()};
    }
    }
}

int32_t helicsCoordinator::getSubscriptionIndex (const std::string &subName) const
{
    return mapFind (subMap_, subName, -1);
}

int32_t helicsCoordinator::getPublicationIndex (const std::string &pubName) const
{
    return mapFind (pubMap_, pubName, -1);
}

void helicsCoordinator::sendDefault (const SubInfo &sub)
{
    switch (sub.type)
    {
    case helicsValueType::helicsString:
        vFed_->setDefaultValue (sub.id, getDefVal<std::string> (sub.defaults));
        break;
    case helicsValueType::helicsComplex:
        vFed_->setDefaultValue (sub.id, getDefVal<std::complex<double>> (sub.defaults));
        break;
    case helicsValueType::helicsDouble:
        vFed_->setDefaultValue (sub.id, getDefVal<double> (sub.defaults));
        break;
    case helicsValueType::helicsVector:
        // vFed_->setDefaultValue(sub.id, getDefVal<double>(sub.defaults));
        break;
    case helicsValueType::helicsInteger:
        vFed_->setDefaultValue (sub.id, getDefVal<int64_t> (sub.defaults));
        break;
    default:
        break;
    }
}

void helicsCoordinator::setDefault (int32_t index, double val)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto sub = subs_[index];
        sub.defaults = val;
        if (sub.id != helics::invalid_id_value)
        {
            sendDefault (sub);
        }
    }
}

void helicsCoordinator::setDefault (int32_t index, const std::string &val)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto sub = subs_[index];
        sub.defaults = val;
        if (sub.id != helics::invalid_id_value)
        {
            sendDefault (sub);
            sub.isValid = true;
        }
    }
}

void helicsCoordinator::setDefault (int32_t index, std::complex<double> val)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto sub = subs_[index];
        sub.defaults = val;
        if (sub.id != helics::invalid_id_value)
        {
            sendDefault (sub);
            sub.isValid = true;
        }
    }
}

void helicsCoordinator::setDefault (int32_t index, int64_t val)
{
    if ((index >= 0) && (index < static_cast<int32_t> (subs_.size ())))
    {
        auto sub = subs_[index];
        sub.defaults = val;
        if (sub.id != helics::invalid_id_value)
        {
            sendDefault (sub);
            sub.isValid = true;
        }
    }
}

void helicsCoordinator::finalize ()
{
    if (vFed_)
    {
        vFed_->finalize ();
    }
    else if (mFed_)
    {
        mFed_->finalize ();
    }
}
}  // namespace helics
}  // namespace griddyn