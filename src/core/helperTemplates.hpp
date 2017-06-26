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
* @brief template helper function for the getParameter String function
@tparam A child class of the original object
@tparam B class of the parent object
* @param[in] originalObject of class A to be cloned
* @param[in] obj pointer of an object to clone to or a null pointer if a new object needs to be created
* @return pointer to the cloned object
*/
template <class A, class B, typename... Args>
std::shared_ptr<A> cloneBase (const A *originalObject, std::shared_ptr<B> obj, Args... args)
{
    static_assert (std::is_base_of<B, A>::value, "classes A and B must have parent child relationship");

    std::shared_ptr<A> clonedObject;
    if (!obj)
    {
        clonedObject = std::make_shared<A> ();
    }
    else
    {
        clonedObject = std::dynamic_pointer_cast<A> (obj);
        if (!clonedObject)
        {
            // if we can't cast the pointer clone at the next lower level
            originalObject->B::clone (obj, args...);
            return nullptr;
        }
    }
    // clone everything in the parent object and above
    originalObject->B::clone (clonedObject, args...);
    return clonedObject;
}

template <class A, class B, class C, typename... Args>
std::shared_ptr<A> cloneBaseStack (const A *originalObject, std::shared_ptr<C> obj, Args... args)
{
    static_assert (std::is_base_of<B, A>::value, "classes A and B must have parent child relationship");
    static_assert (std::is_base_of<C, B>::value, "classes B and C must have parent child relationship");
    std::shared_ptr<A> clonedObject;
    if (!obj)
    {
        clonedObject = std::make_shared<A> ();
    }
    else
    {
        clonedObject = std::dynamic_pointer_cast<A> (obj);
        if (!clonedObject)
        {
            // if we can't cast the pointer clone at the next lower level
            originalObject->B::clone (obj, args...);
            return nullptr;
        }
    }
    // clone everything in the parent object and above
    originalObject->B::clone (clonedObject, args...);
    return clonedObject;
}
}  // namespace griddyn
#endif