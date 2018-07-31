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

#include "core/coreObject.h"
#include <memory>
#include <type_traits>

namespace griddyn
{
/** define the function type for the deleter*/
using removeFunction_t = void (*) (coreObject *obj);

/** template class for defining a (potentially shared) owning ptr for the coreObject
@details uses a custom deleter to operate on the reference counter inside of the core object
intended to be used when there are multiple owners with independent lives and for direct instantiated objects where
the delete function should not be called
shared pointers of coreObjects are not recommended due to the hierarchal nature of the objects
in a block
*/
template <class X>
class coreOwningPtr
{
  private:
    std::unique_ptr<X, removeFunction_t> ptr;  //!< the reference to the object
  public:
    constexpr coreOwningPtr () noexcept : ptr (nullptr, removeReference) {}
    /*IMPLICIT*/ coreOwningPtr (X *obj) : ptr (obj, removeReference)
    {
        static_assert (std::is_base_of<coreObject, X>::value, "owning ptr type must have a base of coreObject");
        if (obj != nullptr)
        {
            obj->addOwningReference ();
        }
    }
    /*IMPLICIT*/ coreOwningPtr (const coreOwningPtr &optr) : coreOwningPtr (optr.get ()) {}
    template <class Y>
    /*IMPLICIT*/ coreOwningPtr (coreOwningPtr<Y> &&ref) noexcept : ptr (ref.release (), removeReference)
    {
    }

    template <class Y>
    coreOwningPtr &operator= (coreOwningPtr<Y> &&ref) noexcept
    {
        ptr.reset (ref.release ());
        return *this;
    }
    coreOwningPtr &operator= (const coreOwningPtr &optr) = delete;
    coreOwningPtr &operator= (std::nullptr_t /*null*/) noexcept
    {
        ptr = nullptr;
        return *this;
    }
    auto operator-> () const noexcept
    {  // return pointer to class object
        return ptr.operator-> ();
    }

    auto get () const noexcept
    {  // return pointer to object
        return (ptr.get ());
    }

    explicit operator bool () const noexcept
    {  // test for non-null pointer
        return (static_cast<bool> (ptr));
    }
    auto release () { return ptr.release (); }
};

template <typename X, typename... Args>
coreOwningPtr<X> make_owningPtr (Args &&... args)
{
    return coreOwningPtr<X> (new X (std::forward<Args> (args)...));
}

/*
template <class X>
class childObject
{
private:
    X* ptr = nullptr;
    coreObject *parent = nullptr;
public:
    childObject() noexcept{};
    childObject(X* obj,coreObject* parentObj):ptr(obj),parent(parentObj)
    {
        static_assert (std::is_base_of<coreObject, X>::value, "child Object ptr type must have a base of
coreObject");
        if (ptr != nullptr)
        {
            ptr->setParent(parent);
            ptr->addOwningReference();
        }
    };
    childObject(childObject &&ref) noexcept:ptr(ref.ptr)
    {

    }
    ~childObject()
    {
        if (ptr != nullptr)
        {
            removeReference(ptr, parent);
        }

    }
    childObject &operator=(childObject &&ref) noexcept
    {
        ptr=ref.ptr;
        return *this;
    }
    childObject &operator=(std::nullptr_t) noexcept
    {
        if (ptr != nullptr)
        {
            removeReference(ptr, ptr->getParent());
        }
        ptr = nullptr;
        parent=nullptr;
        return *this;
    }
    auto operator->() const noexcept
    {	// return pointer to class object
        return ptr;
    }

    auto get() const noexcept
    {	// return pointer to object
        return (ptr);
    }

    explicit operator bool() const noexcept
    {	// test for non-null pointer
        return (ptr!=nullptr);
    }
}
*/

}  // namespace griddyn
