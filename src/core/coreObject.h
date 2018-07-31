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

#include "coreDefinitions.hpp"
#include "utilities/units.h"
// common libraries in all code
// library for printf debug statements

#include <atomic>
#include <memory>

// disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning(disable : 4592)
#endif
#endif

namespace griddyn
{
enum class paramStringType
{
    all,
    localstr,
    localnum,
    str,
    numeric,
    localflags,
    flags
};

typedef void gridPositionInfo;

class helperObject;

/** @brief      base class for a building simulation objects
 Base class for all main simulation objects class includes common properties for all objects such as name, updates
and some common functionality that unifies all objects that are part of the simulation including object ownership,
updates, set and get functions, search features, alert and logging functions
**/
class coreObject
{
  private:
    // this is used much more frequently than any other so it gets its own boolean at the beginning of the object
    bool enabled = true;  //!< enabled indicator
    bool updates_enabled = false;  //!< indicator that updates are enabled
  protected:
    bool extra_bool = false;  //!< an extra flag for derived classes to use
    bool extra_bool2 = false;  //!< a second extra flag for derived classes to use
  private:
    index_t id = 0;  //!< a user defined id for the object
    coreObject *parent = nullptr;  //!< a pointer to the parent object
  protected:  // variables that are used regularly by child class objects
    coreTime prevTime = negTime;  //!<[s]the last state time of the object
    coreTime nextUpdateTime = maxTime;  //!<[s] the next scheduled update
    coreTime lastUpdateTime = negTime;  //!<[s] the last update time
    coreTime updatePeriod = maxTime;  //!<[s]the update period
    coreTime updateDelay =
      timeZero;  //!<[s]the requested delay between updateA and updateB--requested is key here not guaranteed
  private:
    // these shouldn't generate false shareing as one is static
    static std::atomic<id_type_t> s_obcnt;  //!< the global object counter
    std::atomic<count_t> m_refCount;  //!< counter for the number of owning objects;
  public:
    index_t locIndex = kNullLocation;  //!< a lookup index for the object to reference parent location in storage
                                       //!< arrays for use by containing objects no operational dependencies
  private:
    id_type_t m_oid;  //!< a unique index for the object
    std::string name;  //!< the text name of the object
  public:
    /** @brief default constructor
    @param[in] objName the name of the object[optiona] default to "object_#"
    the name can be followed by a few symbols see # appends the id, $ appends the userid, and @ appends the
    locIndex
    */
    explicit coreObject (const std::string &objName = "object_#");

    // don't allow copy constructors and equal operator as they would introduce all sorts of other complicated
    // issues in the system
    coreObject (const coreObject &) = delete;
    void operator= (const coreObject &obj) = delete;
    /** @brief default destructor  so it can be overridden*/
    virtual ~coreObject ();
    /**
     * @brief clones an object to another object or makes a new on
     * @param[in] obj the object to clone to or leave nullptr for a new object.
     */
    virtual coreObject *clone (coreObject *obj = nullptr) const;

    /** @brief update a name potentially containing specific codes
     names ending in # have the '#' replaced with the oid
    names ending in $ have the '$' replaced with the userid
    name ending in @ have the '@' replaced with the locIndex
    */
    void updateName ();

    /**
     * @brief forwards an alert from object *object up the chain or processes it.
     * @param[in] object the object that generated the alert.
     * @param[in] code an alert code
     */
    virtual void alert (coreObject *object, int code);

    /**
     * @brief forwards a log message from object *object up the chain or processes it.
     * @param[in] object the object that generated the message.
     * @param[in] level the level of the log message
     * @param[in] message the log message
     */
    virtual void log (coreObject *object, print_level level, const std::string &message);

    /**
     * @brief increment the reference counter for the object
     */
    void addOwningReference ();
    /**
    * @brief search for an object with the name indicated by &object.
    * @param[in] object the name of the object to search for.
    @return nullptr if the object is not found otherwise the object
    */
    virtual coreObject *find (const std::string &object) const;
    /**
    * @brief retrieve a subObject of type typeName and index num.
    * @param[in] typeName a string indicating which type of object to retrieve
    * @param[in] num the index of the object to retrieve  (index is 0 based)
    @return nullptr if the object is not found otherwise the object
    */
    virtual coreObject *getSubObject (const std::string &typeName, index_t num) const;
    /**
    * @brief locate a subObject of type typeName and searchID.
    * @param[in] typeName a string indicating which type of object to retrieve
    * @param[in] searchID the id of the object to search for
    @return nullptr if the object is not found otherwise the object
    */
    virtual coreObject *findByUserID (const std::string &typeName, index_t searchID) const;
    /**
     * @brief adds an object to another object for instance adding a load to the bus.
     * @param[in] obj the object to add
     * throws an exception if the object is invalid or cannot be added
     */
    virtual void add (coreObject *obj);

    /** @brief add a helperObject to the object
    @param[in] obj shared_ptr to a helper object
    *throws an exception if the object is invalid or cannot be added
    */
    virtual void addHelper (std::shared_ptr<helperObject> obj);
    /**
     * @brief remove an object from the calling object
     * @param[in] obj the object to remove
     * @return value indicating success or failure 0 success -1 (object not found) -2(removal failure)
     */
    virtual void remove (coreObject *obj);
    /**
     * @brief sets a string parameter of an object
     * @param[in] param the name of the parameter to change
     * @param[in] val the value of the parameter to set
     */
    virtual void set (const std::string &param, const std::string &val);
    /**
     * @brief sets a numeric parameter of an object
     * @param[in] param the name of the parameter to change
     * @param[in] val the value of the parameter to set
     * @param[in] unitType a type indicating the units of the val a defUnit default value
     */
    virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit);
    /** @brief get flags
    @param flag -the name of the flag to be queried
    @param val the value to the set the flag ;
    @return int a value representing whether the set operation was successful or not
    */
    virtual void setFlag (const std::string &flag, bool val = true);
    /** @brief get flags
    @param param the name of the flag to query.
    @return the value of the flag queried
    */
    virtual bool getFlag (const std::string &flag) const;
    /**
     * @brief get a parameter from the object
     * @param[in] param the name of the parameter to get
     * @param[in] unitType a type indicating the units of the val a defUnit default value
     * @return val the value of the parameter returns kNullVal if no property is found
     */
    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const;
    /**
     * helper function wrapper to return an int (instead of a double) from the get function
     * @param[in] param the name of the parameter to get
     * @return val the value of the parameter
     */
    int getInt (const std::string &param) const;

    /**
     * get a string property from an object
     * @param[in] param the name of the parameter to get
     * @return the string property  return "NA" if no property is found
     */
    virtual std::string getString (const std::string &param) const;

    /**
     * @brief get the parameters that can be used with the set/get functions
     * @param[out] pstr a vector of strings to append to
     * @param[in] pstype the type of parameters to get the strings for
     */
    virtual void getParameterStrings (stringVec &pstr, paramStringType pstype = paramStringType::all) const;
    /**
    * @brief update the object at a specific time
  @ details if the object requires and A and B parts this is the A part the B part gets executed at a later time
    * @param[in] time the times to update the object to
    */
    virtual void updateA (coreTime time);
    /**
     * @brief the B update function for update calls with two parts
     */
    virtual coreTime updateB ();
    /**
     * @brief function to enable the object, most objects are enabled by default
     */
    virtual void enable ();
    /**
     * @brief function to disable the object most objects are enabled by default
     */
    virtual void disable ();
    /** check if the object is enabled*/
    virtual bool isEnabled () const;

    /** check if the object is cloneable
    @return true if the object is cloneable*/
    virtual bool isCloneable () const;
    /**
     * @brief returns the oid of the object which is supposed to be a unique identifier
     */
    std::int64_t getID () const noexcept { return m_oid; }
    /**
     * @brief updates the OID with a new number-useful in a few circumstances to ensure the id is higher than
     * another object
     */
    void makeNewOID ();

    /** @brief set the name*/
    void setName (const std::string &newName)
    {
        name = newName;
        nameUpdate ();
    }

    /** @brief get the name of the object*/
    const std::string &getName () const noexcept { return name; }
    /** add a description to the object
    @param[in] description a string describing the object*/
    void setDescription (const std::string &description);
    /** fetch the core object description
    @return a string describing the object may be empty if no description was entered*/
    std::string getDescription () const;
    /** @brief set the parent*/
    virtual void setParent (coreObject *parentObj);
    /** @brief get the parent object*/
    coreObject *getParent () const { return parent; }
    /** get the root object */
    coreObject *getRoot () const
    {  // the id of 0 is used by a special nullObject
        return (parent->id != 0) ? (parent->getRoot ()) : const_cast<coreObject *> (this);
    }
    /** check if the object is a root object*/
    bool isRoot () const { return (parent->id == 0); }
    /** @brief set the user defined identification number for an object
    @param[in] newUserID the new user defined identifier for an object*/
    void setUserID (index_t newUserID)
    {
        id = newUserID;
        idUpdate ();
    }
    /** get the userID
    @return the user defined identification code for the object*/
    index_t getUserID () const noexcept { return id; }
    /** turn on updates for an object
    @param[in] upd_enabled a boolean defining whether to turn updates on(true) or off (false)
    */
    void enable_updates (bool upd_enabled = true)
    {
        updates_enabled = upd_enabled;
        alert (this, UPDATE_REQUIRED);
    }
    /** check if an object has updates
    @return true if updates are enabled
    */
    bool hasUpdates () const { return updates_enabled; }
    /**@brief set the next update Time
    @param[in] newUpdateTime the next time the update should trigger
    */
    virtual void setUpdateTime (double newUpdateTime);
    /** @brief get the next time the system should call the update functions
     */
    coreTime getNextUpdateTime () const noexcept { return nextUpdateTime; }

    /**@brief return the last time the object had its state set or was updated
     */
    coreTime currentTime () const noexcept { return prevTime; }
    friend void removeReference (coreObject *objToDelete);
    friend void removeReference (coreObject *objToDelete, const coreObject *parent);
    friend bool compareUpdates (const coreObject *o1, const coreObject *o2);
    friend bool compareNames (const coreObject *o1, const coreObject *o2);
    friend bool isSameObject (const coreObject *o1, const coreObject *o2);
    friend bool isSameObject (id_type_t id, const coreObject *o2);
    friend bool isSameObject (const coreObject *o1, id_type_t id);
    friend std::string fullObjectName (const coreObject *obj);
    friend class nullObject;

  protected:
    /** simple function for alerting of a name change*/
    virtual void nameUpdate ();
    /** simple function for alerting of a user id change*/
    virtual void idUpdate ();

  private:
    /** private constructor function to build an object with a specified object id
    @details used in special objects for various purposes
    */
    coreObject (id_type_t coid);
};

/** @brief helper function to set multiple flags on a object separated by ; or ,
@details the flags can be turned off by putting a - in front of the flag
@param[in] obj the object to set the flags on
@param[in] flags  the list of flags to set
*/
void setMultipleFlags (coreObject *obj, const std::string &flags);
/**
* @brief function to compare update times for object sorting
* @param[in] o1 the first object to compare
@param[in] o2 the second object to compare
@return true if the update time from o1 comes before the update time in o2
*/
inline bool compareUpdates (const coreObject *o1, const coreObject *o2)
{
    return (o1->nextUpdateTime < o2->nextUpdateTime);
}
/**
* @brief function to compare names of two objects
* @param[in] o1 the first object to compare
* @param[in] o2 the second object to compare
@return true if the first object comes before the second alphabetically
*/
inline bool compareNames (const coreObject *o1, const coreObject *o2) { return (o1->name < o2->name); }

/**
 * @brief general deletion function that checks the reference count and deletes the object if it is 0;
 * @param[in] objToDelete the object to potentially delete
 */
void removeReference (coreObject *objToDelete);
/**
* @brief general deletion function that checks the reference count and deletes the object if it is 0;
* if it is not deleted it check the parent and removes a reference to the parent in the object;
* @param[in] objToDelete the object to potentially delete
@param[in] parent the parent of the object
*/
void removeReference (coreObject *objToDelete, const coreObject *parent);
/**
* @brief get an objects full path name including all parent objects( except root)
* @param[in] obj for which to get the full name
@return the full object path
*/
std::string fullObjectName (const coreObject *obj);

/**
* @brief function to check if two objects are the same object
* @param[in] o1 the first object to check
* @param[in] o2 the second object to check
@return true if the objects have the object id
*/
inline bool isSameObject (const coreObject *o1, const coreObject *o2)
{
    return ((o2 != nullptr) && (o1 != nullptr) && (o1->m_oid == o2->m_oid));
}
/**
* @brief function to check if two objects are the same object
* @param[in] id the id of the first object to check
* @param[in] o2 the second object to check
@return true if the id's match
*/
inline bool isSameObject (id_type_t id, const coreObject *o2) { return ((o2 != nullptr) && (id == o2->m_oid)); }
/**
* @brief function to check if two objects are the same object
* @param[in] o1 the first object to check
* @param[in] id the identifier of the first object to check
@return true if the id's match
*/
inline bool isSameObject (const coreObject *o1, id_type_t id) { return ((o1 != nullptr) && (id == o1->m_oid)); }

/**
* @brief convert a string to a print level
* @param[in] level the level name as a string
@throw invalidParameterValue() if level is not recognized
*/
print_level stringToPrintLevel (const std::string &level);

// logging Macros

#define LOG_ERROR(message) log (this, print_level::error, message);
#define LOG_WARNING(message) log (this, print_level::warning, message);

#ifdef LOG_ENABLE
#define LOG_SUMMARY(message) log (this, print_level::summary, message);
#define LOG_NORMAL(message) log (this, print_level::normal, message);

#ifdef DEBUG_LOG_ENABLE
#define LOG_DEBUG(message) log (this, print_level::debug, message);
#else
#define LOG_DEBUG(message)
#endif

#ifdef TRACE_LOG_ENABLE
#define LOG_TRACE(message) log (this, print_level::trace, message);
#else
#define LOG_TRACE(message)
#endif

#else
#define LOG_SUMMARY(message)
#define LOG_NORMAL(message)
#define LOG_DEBUG(message)
#define LOG_TRACE(message)
#endif

}  // namespace griddyn
