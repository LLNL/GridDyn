/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef GRIDCOREOBJECT_H_

#define GRIDCOREOBJECT_H_

#include "basicDefs.h"
#include "gridDynVectorTypes.h"
#include "units.h"

//common libraries in all code
//library for printf debug statements

#include <memory>
#include <vector>
#include <atomic>


//disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning( disable : 4592)
#endif
#endif


enum class paramStringType
{
  all, localstr,localnum,string,numeric,localflags,flags
};

typedef void gridPositionInfo;

/** @brief      base class for a majority of GridDyn Objects
 Base class for all gridDyn objects class includes common properties for all objects such as name, updates
and some common functionality that unifies all objects that are part of the simulation including object ownership, updates, set and get functions, search features,
alert and logging functions
**/
class gridCoreObject
{
public:
  std::string description;             //!< storage for a description of the object meant for user, no operational impact
  index_t locIndex = kNullLocation;           //!< a lookup index for the object to reference parent location in storage arrays for use by containing objects no operational dependencies
  index_t locIndex2 = kNullLocation;           //!< a second lookup index for the object to reference parent location in storage arrays for use by containing objects no operational dependencies
  //this is used much more frequently than any other so it gets its own bool for ease of use

  bool enabled = true;           //!< enabled indicator TODO: PT move to a protected instead of public

private:
  static std::atomic<count_t> s_obcnt;       //!< the global object counter
  count_t m_oid;       //!< a unique index for the object
  gridCoreObject *owner = nullptr;      //!<a pointer to the owner object
protected:
  std::string name;       //!< the text name of the object
  index_t id;              //!< a user defined id for the object
  gridCoreObject *parent = nullptr;      //!< a pointer to the parent object
  gridDyn_time prevTime = negTime;       //!<[s]the last state time of the object
  gridDyn_time updatePeriod = maxTime;      //!<[s]the update period
  gridDyn_time nextUpdateTime = maxTime;     //!<[s] the next scheduled update
  gridDyn_time lastUpdateTime = negTime;      //!<[s] the last update time
  gridDyn_time m_bDelay  = -1.0;         //!<[s]the requested delay between updateA and updateB--requested is key here not guaranteed
  

  std::shared_ptr<gridPositionInfo> pos;  //!< pointer to as of yet undefined position information structure.
public:
  /** @brief default constructor*/
  explicit gridCoreObject (const std::string &name = "object_#");

  //don't allow copy constructors and equal operator as they would introduce all sorts of other complicated issues in the system
  gridCoreObject (const gridCoreObject&) = delete;
  void operator= (gridCoreObject &obj) = delete;
  /** @brief default destructor  so it can be overridden*/
  virtual ~gridCoreObject ();
  /**
  * @brief clones an object to another object or makes a new on
  * @param[in] obj the object to clone to or leave nullptr for a new object.
  */
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const;

  /** @brief update a name potentially containing specific codes
   names ending in # have the '#' replaced with the oid
  names ending in $ have the name replaced with the userid
  */
  void updateName ();

  /**
   * @brief forwards an alert from object *object up the chain or processes it.
   * @param[in] object the object that generated the alert.
   * @param[in] code an alert code
   */
  virtual void alert (gridCoreObject *object, int code);

  /**
  * @brief forwards a log message from object *object up the chain or processes it.
  * @param[in] object the object that generated the message.
  * @param[in] level the level of the log message
  * @param[in] message the log message
  */
  virtual void log (gridCoreObject *object, print_level level, const std::string &message);
  /**
  * @brief sets the object time used primarily for shifting the clock to a different basis
  * @param[in] time the time to set the object clock to.
  */
  virtual void setTime (gridDyn_time time);
  /**
  * @brief changes ownership of the object
    @details so the owner can't be changed without the current owners permission
  permission is assumed if the currentOwner location is known

  * @param[in] currentOwner the currentOwner of the object
  @param[in]  newOwner  the desired newOwner of the object
  @return indicator of success  true if successful
  */
  bool setOwner (gridCoreObject *currentOwner, gridCoreObject *newOwner);
  /**
  * @brief search for an object with the name indicated by &object.
  * @param[in] object the name of the object to search for.
  @return nullptr if the object is not found otherwise the object
  */
  virtual gridCoreObject* find (const std::string &object) const;
  /**
  * @brief retrieve a subObject of type typeName and index num.
  * @param[in] typeName a string indicating which type of object to retrieve
  * @param[in] num the index of the object to retrieve  (index is 0 based)
  @return nullptr if the object is not found otherwise the object
  */
  virtual gridCoreObject* getSubObject (const std::string & typeName, index_t num) const;
  /**
  * @brief locate a subObject of type typeName and searchID.
  * @param[in] typeName a string indicating which type of object to retrieve
  * @param[in] searchID the id of the object to search for
  @return nullptr if the object is not found otherwise the object
  */
  virtual gridCoreObject * findByUserID (const std::string & typeName, index_t searchID) const;
  /**
  * @brief adds an object to another object for instance adding a load to the bus.
  * @param[in] obj the object to add
  * throws and exception if the object is invalid or cannot be added
  */
  virtual void add (gridCoreObject * obj);

  /**
  * @brief adds a shared ptr object.
  * @param[in] obj the object to add
  * throws and exception if the object is invalid or cannot be added
  */
  virtual void addsp (std::shared_ptr<gridCoreObject> obj);

  /**
  * @brief remove an object from the calling object
  * @param[in] obj the object to remove
  * @return value indicating success or failure 0 success -1 (object not found) -2(removal failure)
  */
  virtual void remove (gridCoreObject * obj);
  /**
  * @brief sets a string parameter of an object
  * @param[in] param the name of the parameter to change
  * @param[in] val the value of the parameter to set
  */
  virtual void set (const std::string &param,  const std::string &val);
  /**
  * @brief sets a numeric parameter of an object
  * @param[in] param the name of the parameter to change
  * @param[in] val the value of the parameter to set
  * @param[in] unitType a type indicating the units of the val a defUnit default value
  */
  virtual void set (const std::string &param, double val,gridUnits::units_t unitType = gridUnits::defUnit);
  /** @brief get flags
  \param flag -the name of the flag to be queried
  \param val the value to the set the flag ;
  \return int a value representing whether the set operation was successful or not
  */
  virtual void setFlag (const std::string &flag, bool val = true);
  /** @brief get flags
  \param param the name of the flag to query.
  \return the value of the flag queried
  */
  virtual bool getFlag (const std::string &param) const;
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
  virtual void updateA (gridDyn_time time);
  /**
  * @brief the B update function for update calls with two parts
  */
  virtual double updateB ();
  /**
  * @brief function to enable the object, most objects are enabled by default
  */
  virtual void enable ();
  /**
  * @brief function to disable the object most objects are enabled by default
  */
  virtual void disable ();

  /**
  * @brief static function to reset the object counter
  @param[in] counterVal the new counterVal
  */
	//TODO:: I am not entirely sure I should allow this function
  //static void setCounter (count_t counterVal)
  //{
  //  s_obcnt = counterVal;
  //}
  /**
  * @brief returns the oid of the object which is supposed to be a unique identifier
  */
  index_t getID () const
  {
    return m_oid;
  }
  /**
  * @brief updates the OID with a new number-useful in a few circumstances to ensure the id is higher than another object
  */
  void makeNewOID ();

  /** @brief loads a position object
   *@details I don't know what a grid Position object looks like yet
  @param[in] npos a gridPositionObject
  */
  void loadPosition (std::shared_ptr<gridPositionInfo> npos);

  /** @brief set the name*/
  virtual void setName (std::string name);

  /** @brief get the name of the object*/
  const std::string &getName () const
  {
    return name;
  }

  /** @brief set the parent*/
  virtual void setParent (gridCoreObject *parentObj);
  /** @brief get the parent object*/
  gridCoreObject * getParent () const
  {
    return parent;
  }

  /** @brief set the parent*/
  virtual void setUserID (index_t newUserID);

  index_t getUserID () const
  {
    return id;
  }
  //@brief set the next update Time
  virtual void setUpdateTime (double newUpdateTime);
  //@brief get the next time the system should call the update functions
  gridDyn_time getNextUpdateTime () const
  {
    return nextUpdateTime;
  }
  
  //@brief return the last time the object had its state set or was updated
  gridDyn_time currentTime () const
  {
    return prevTime;
  }
  friend void condDelete (gridCoreObject *obj, gridCoreObject *Pobject);
  friend bool compareUpdates (gridCoreObject *o1, gridCoreObject *o2);
  friend bool compareNames (gridCoreObject *o1, gridCoreObject *o2);
  friend std::string fullObjectName (gridCoreObject *obj);
};

/**
* @brief function to compare update times for object sorting
* @param[in] o1 the first object to compare
@param[in] o2 the second object to compare
@return true if the update time from o1 comes before the update time in o2
*/
bool compareUpdates (gridCoreObject *o1, gridCoreObject *o2);
/**
* @brief function to compare names of two objects
* @param[in] o1 the first object to compare
* @param[in] o2 the second object to compare
@return true if the first object comes before the second alphabetically
*/
bool compareNames (gridCoreObject *o1, gridCoreObject *o2);
/**
* @brief general deletion function that verifies object ownership
* @param[in] objToDelete the object to potentially delete
* @param[in] parentObject the object that is doing the deletion
*/
void condDelete (gridCoreObject *objToDelete, gridCoreObject *parentObject);
//helper templates

/**
* @brief get an objects full path name including all parent objects( except root)
* @param[in] obj for which to get the full name
@return the full object path
*/
std::string fullObjectName (gridCoreObject *obj);

/**
* @brief convert a string to a print level
* @param[in] level the level name as a string
@throw invalidParameterValue() if level is not recognized
*/
print_level stringToPrintLevel(const std::string &level);

//logging Macros

#define LOG_ERROR(message) log (this,print_level::error,message);
#define LOG_WARNING(message) log (this,print_level::warning,message);

#ifdef LOG_ENABLE
#define LOG_SUMMARY(message) log (this,print_level::summary,message);
#define LOG_NORMAL(message) log (this,print_level::normal,message);

#ifdef DEBUG_LOG_ENABLE
#define LOG_DEBUG(message) log (this,print_level::debug,message);
#else
#define LOG_DEBUG(message)
#endif

#ifdef TRACE_LOG_ENABLE
#define LOG_TRACE(message) log (this,print_level::trace,message);
#else
#define LOG_TRACE(message)
#endif

#else
#define LOG_SUMMARY(message)
#define LOG_NORMAL(message)
#define LOG_DEBUG(message)
#define LOG_TRACE(message)
#endif

#endif
