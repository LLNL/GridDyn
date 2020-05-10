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

#ifndef GRIDCORELIST_H_
#define GRIDCORELIST_H_
// include the autogenerated config file

#include "coreObject.h"

#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace griddyn {
using namespace boost::multi_index;
using boost::multi_index_container;

struct name {
};
struct id {
};
struct uid {
};
// define a multi-index container based on the object id, which should be unique, and the name which also should be
// unique, and the user id,
// which isn't necessarily unique.
using objectIndex = multi_index_container<
    coreObject*,
    indexed_by<
        ordered_unique<tag<id>, const_mem_fun<coreObject, id_type_t, &coreObject::getID>>,
        ordered_unique<tag<name>,
                       const_mem_fun<coreObject, const std::string&, &coreObject::getName>>,
        ordered_non_unique<tag<uid>, const_mem_fun<coreObject, index_t, &coreObject::getUserID>>>>;

/** @brief list class that facilitates adding or searching for pointers to griddyn by name or id
 * a list class that stores a list of the objects contained in an unordered map to facilitate searching for objects
 */
class coreObjectList {
  private:
    objectIndex m_objects;  //!< the object map structure
  public:
    /** @brief default constructor*/
    coreObjectList() = default;
    /**
     * @brief function to insert an object into the class
     * @param[in] obj the object to insert
     * @param[in] replace an optional indicator telling whether to replace the object or nor
     * @return a bool indicating successful insertion
     */
    bool insert(coreObject* obj, bool replace = false);
    /** @brief remove an object
     * function to remove an object from the container
     * @param[in] obj the object to insert
     * @return a bool indicating successful removal (0 on success, -1 on failure) const
     */
    bool remove(coreObject* obj);
    /** @brief remove an object by name
     * function to remove an object from the container
     * @param[in] obj the object to insert
     * @return a bool indicating successful removal (0 on success, -1 on failure) const
     */
    bool remove(const std::string& objName);

    /** @brief find object by name
     * function to find an object by name
     * @param[in] objName the name of the object to search for
     * @return a pointer to the object if found otherwise nullptr
     */
    coreObject* find(const std::string& objName) const;

    /** @brief find object by id
     * function to find an object by user id code
     * @param[in] objName the name of the object to search for
     * @return a vector of objects with the appropriate searchID
     */
    std::vector<coreObject*> find(index_t searchID) const;

    /** @brief check if an object is already contained
     * function to find an object by id
     * @param[in] obj to check
     * @return a bool indicating if the object is a member or not
     */
    bool isMember(const coreObject* obj) const;

    /**
    * @brief deletes all object pointed to by the list
    calls the condDel
    */
    void deleteAll(coreObject* parent);

    /** @brief get the size of the list

    * @return the size of the list
    */
    count_t size() const { return static_cast<count_t>(m_objects.size()); }

    /**
    * @brief update a single object with a name change or index change
    @param[in] obj the object with the change
    */
    void updateObject(coreObject* obj);
};

}  // namespace griddyn
#endif
