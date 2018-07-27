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

#ifndef PARAMETER_OPERATOR_H_
#define PARAMETER_OPERATOR_H_
#pragma once
// headers
//#include "gridDynSimulation.h"

#include "core/coreDefinitions.hpp"
#include "core/helperObject.h"

#include "core/objectOperatorInterface.hpp"
#include <memory>

namespace griddyn
{
class gridComponent;
/** a class defining a parameter to change as part of a sequence or other operation
 */
class parameterOperator : public helperObject, public objectOperatorInterface
{
  protected:
    std::string m_field;  //!< field to trigger
    gridComponent *comp = nullptr;  //!< the object to operator on
    index_t parameterIndex = kNullLocation;  //!< the parameter index to use if so inclined
  public:
    parameterOperator ();
    parameterOperator (gridComponent *target, const std::string &field);

    virtual void setTarget (gridComponent *target, const std::string &field = "");

    virtual void updateObject (coreObject *target, object_update_mode mode = object_update_mode::direct) override;
    virtual void setParameter (double val);
    virtual double getParameter () const;
    virtual coreObject *getObject () const override;
    virtual void getObjects (std::vector<coreObject *> &objects) const override;
    bool isDirect () const { return (parameterIndex != kNullLocation); }

  protected:
    void checkField ();
};

/** construct a parameter operator object from a field and root object*/
std::unique_ptr<parameterOperator> make_parameterOperator (const std::string &param, gridComponent *rootObject);

/** helper class defining a set of parameters for various operations*/
class parameterSet
{
  private:
    std::vector<std::unique_ptr<parameterOperator>> params;

  public:
    parameterSet () = default;
    index_t add (const std::string &paramString, gridComponent *rootObject);
    parameterOperator *operator[] (index_t index);
};

}  // namespace griddyn
#endif