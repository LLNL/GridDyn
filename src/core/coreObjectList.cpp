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

#include "coreObjectList.h"
namespace griddyn {
bool coreObjectList::insert(coreObject* obj, bool replace)
{
    auto inp = m_objects.insert(obj);
    if (inp.second) {
        return true;
    }
    if (replace) {
        m_objects.replace(inp.first, obj);
        return true;
    }
    return false;
}
coreObject* coreObjectList::find(const std::string& objName) const
{
    auto fp = m_objects.get<name>().find(objName);
    if (fp != m_objects.get<name>().end()) {
        return (*fp);
    }
    return nullptr;
}

std::vector<coreObject*> coreObjectList::find(index_t searchID) const
{
    auto fp = m_objects.get<uid>().lower_bound(searchID);
    auto fp2 = m_objects.get<uid>().upper_bound(searchID);
    std::vector<coreObject*> out;
    while (fp != fp2) {
        if ((*fp)->getUserID() == searchID) {
            out.push_back(*fp);
        }
        ++fp;
    }
    return out;
}

bool coreObjectList::remove(coreObject* obj)
{
    auto fp = m_objects.get<id>().find(obj->getID());
    if (fp != m_objects.get<id>().end()) {
        m_objects.erase(fp);
        return true;
    }
    return false;
}

bool coreObjectList::remove(const std::string& objName)
{
    auto fp = m_objects.get<name>().find(objName);
    if (fp != m_objects.get<name>().end()) {
        // I don't know why I have to do this find on the id index
        // Not understanding these multindex objects well enough I guess
        auto fp2 = m_objects.get<id>().find((*fp)->getID());
        m_objects.erase(fp2);

        return true;
    }
    return false;
}

bool coreObjectList::isMember(const coreObject* obj) const
{
    auto fp = m_objects.get<id>().find(obj->getID());
    return (fp != m_objects.get<id>().end());
}

void coreObjectList::deleteAll(coreObject* parent)
{
    for (auto& it : m_objects) {
        removeReference(it, parent);
    }
}

void coreObjectList::updateObject(coreObject* obj)
{
    auto fp = m_objects.get<id>().find(obj->getID());
    m_objects.replace(fp, obj);
}

}  // namespace griddyn
