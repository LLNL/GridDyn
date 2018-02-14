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

#include "propertyBuffer.h"
#include "coreObject.h"
#include "helperObject.h"
#include <algorithm>
namespace griddyn
{
void propertyBuffer::set (const std::string &param, const std::string &val)
{
    properties.emplace_back (param, property_type (val));
}
void propertyBuffer::set (const std::string &param, double val)
{
    properties.emplace_back (param, property_type (val));
}
void propertyBuffer::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    properties.emplace_back (param, property_type (std::make_pair (val, unitType)));
}
void propertyBuffer::set (const std::string &param, int val)
{
    properties.emplace_back (param, property_type (val));
}
void propertyBuffer::setFlag (const std::string &flag, bool val)
{
    properties.emplace_back (flag, property_type (val));
}

void propertyBuffer::remove (const std::string &param)
{
    // Using auto lambda here still seems like magic that this works
    auto checkMatch = [param](auto input) { return (std::get<0> (input) == param); };

    auto strend = std::remove_if (properties.begin (), properties.end (), checkMatch);
    properties.erase (strend, properties.end ());
}

void propertyBuffer::clear () { properties.clear (); }
void propertyBuffer::apply (coreObject *obj) const
{
    for (auto &prop : properties)
    {
        switch (prop.second.which ())
        {
        case 0:
            obj->set (prop.first, boost::get<double> (prop.second));
            break;
        case 1:
            obj->set (prop.first, boost::get<std::pair<double, gridUnits::units_t>> (prop.second).first,
                      boost::get<std::pair<double, gridUnits::units_t>> (prop.second).second);
            break;
        case 2:
            obj->set (prop.first, boost::get<int> (prop.second));
            break;
        case 3:
            obj->setFlag (prop.first, boost::get<bool> (prop.second));
            break;
        case 4:
            obj->set (prop.first, boost::get<std::string> (prop.second));
            break;
        }
    }
}

}  // namespace griddyn