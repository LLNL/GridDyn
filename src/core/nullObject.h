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

#include "coreObject.h"

namespace griddyn
{
/** @brief      a nullObject class
pretty much does nothing it absorbs logs and alerts (its parent is itself)
**/
class nullObject final : public coreObject
{
  public:
    /** @brief default constructor
    takes a code used as the oid for special id codes below 100
    */
    explicit nullObject(std::uint64_t nullCode = 500) noexcept;
    /** @brief nullObject constructor which takes a string*/
    explicit nullObject(const std::string &objName);

    virtual coreObject *clone(coreObject *obj = nullptr) const override;
    /**
     * alert just sinks all alerts and does nothing
     */
    virtual void alert(coreObject *object, int code) override;

    /**
     * log just absorbs all log messages and does nothing
     */
    virtual void log(coreObject *object, print_level level, const std::string &message) override;

    /** return a nullptr*/
    virtual coreObject *find(const std::string &object) const override;

    /**
    returns a nullptr
    */
    virtual coreObject *findByUserID(const std::string &typeName, index_t searchID) const override;

    /** @brief set the parent
    @details nullObjects do not allow the parents to be set*/
    virtual void setParent(coreObject *parentObj) override;
};
}  // namespace griddyn
