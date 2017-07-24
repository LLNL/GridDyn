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

#ifndef HELPER_TEMPLATES_H_
#define HELPER_TEMPLATES_H_
#include <memory>
#include <type_traits>

namespace griddyn
{
/**
* @brief template helper function for a cascading clone of an object
@tparam targetObjectType child class of the original object
@tparam parentObjectType class of the parent object
* @param[in] originalObject of class targetObjectType to be cloned
* @param[in] obj pointer of an object to clone to or a null pointer if a new object needs to be created
* @return pointer to the cloned object
*/
template <class targetObjectType, class parentObjectType, typename... Args>
std::shared_ptr<targetObjectType>
cloneBase (const targetObjectType *originalObject, std::shared_ptr<parentObjectType> obj, Args... args)
{
    static_assert (std::is_base_of<parentObjectType, targetObjectType>::value,
                   "classes targetObjectType and parentObjectType must have parent child relationship");

    std::shared_ptr<targetObjectType> clonedObject;
    if (!obj)
    {
        clonedObject = std::make_shared<targetObjectType> ();
    }
    else
    {
        clonedObject = std::dynamic_pointer_cast<targetObjectType> (obj);
        if (!clonedObject)
        {
            // if we can't cast the pointer clone at the next lower level
            originalObject->parentObjectType::clone (obj, args...);
            return nullptr;
        }
    }
    // clone everything in the parent object and above
    originalObject->parentObjectType::clone (clonedObject, args...);
    return clonedObject;
}

/**
* @brief template helper function for a cascading clone of an object through multiple derived classes
@tparam targetObjectType child class of the original object
@tparam B class of the parent object
* @param[in] originalObject of class targetObjectType to be cloned
* @param[in] obj pointer of an object to clone to or a null pointer if a new object needs to be created
* @return pointer to the cloned object or nullptr if the given object is not of the correct type
*/
template <class targetObjectType, class parentObjectType, class baseObjectType, typename... Args>
std::shared_ptr<targetObjectType>
cloneBaseStack (const targetObjectType *originalObject, std::shared_ptr<baseObjectType> obj, Args... args)
{
    static_assert (std::is_base_of<parentObjectType, targetObjectType>::value,
                   "classes A and parentObjectType must have parent child relationship");
    static_assert (std::is_base_of<baseObjectType, parentObjectType>::value,
                   "classes parentObjectType and baseObjectType must have parent child relationship");
    std::shared_ptr<targetObjectType> clonedObject;
    if (!obj)
    {
        clonedObject = std::make_shared<targetObjectType> ();
    }
    else
    {
        clonedObject = std::dynamic_pointer_cast<targetObjectType> (obj);
        if (!clonedObject)
        {
            // if we can't cast the pointer clone at the next lower level
            originalObject->parentObjectType::clone (obj, args...);
            return nullptr;
        }
    }
    // clone everything in the parent object and above
    originalObject->parentObjectType::clone (clonedObject, args...);
    return clonedObject;
}
}  // namespace griddyn
#endif