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

#include "helperObject.h"
#include "coreExceptions.h"
#include "utilities/dataDictionary.h"
#include "utilities/stringOps.h"
#include "utilities/string_viewOps.h"
namespace griddyn
{
// start at 100 since there are some objects that use low numbers as a check for interface number and the id as
// secondary
std::atomic<std::uint64_t> helperObject::s_obcnt (101);

helperObject::helperObject () noexcept : m_oid (s_obcnt++) {}
helperObject::~helperObject () = default;

helperObject::helperObject (std::string objectName) : m_oid (s_obcnt++), um_name (std::move (objectName)) {}
static dataDictionary<std::uint64_t, std::string> descriptionDictionary;

void helperObject::set (const std::string &param, const std::string &val)
{
    if ((param == "name") || (param == "id"))
    {
        setName (val);
    }
    else if (param == "description")
    {
        setDescription (val);
    }
    else if ((param == "flags") || (param == "flag"))
    {
        setMultipleFlags (this, val);
    }
    else
    {
        throw (unrecognizedParameter (param));
    }
}

void helperObject::set (const std::string &param, double val) { setFlag (param, (val > 0.1)); }
void helperObject::setDescription (const std::string &description)
{
    descriptionDictionary.update (m_oid, description);
}

std::string helperObject::getDescription () const { return descriptionDictionary.query (m_oid); }
void helperObject::setFlag (const std::string &flag, bool /*val*/) { throw (unrecognizedParameter (flag)); }
bool helperObject::getFlag (const std::string &flag) const { throw (unrecognizedParameter (flag)); }
double helperObject::get (const std::string &param) const { return getFlag (param) ? 1.0 : 0.0; }
void helperObject::nameUpdate () {}
void helperObject::makeNewOID() { m_oid = ++s_obcnt; }
coreObject *helperObject::getOwner () const { return nullptr; }
void setMultipleFlags (helperObject *obj, const std::string &flags)
{
    auto lcflags = convertToLowerCase (flags);
    auto flgs = utilities::string_viewOps::split (lcflags);
    utilities::string_viewOps::trim (flgs);
    for (const auto &flag : flgs)
    {
		if (flag.empty())
		{
			continue;
		}
        if (flag.front() != '-')
        {
            obj->setFlag (flag.to_string (), true);
        }
        else
        {
            obj->setFlag (flag.substr (1, utilities::string_view::npos).to_string (), false);
        }
    }
}

}  // namespace griddyn
