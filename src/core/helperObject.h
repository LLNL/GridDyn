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

#include <atomic>
#include <string>

// disable a funny warning (bug in visual studio 2015)
#ifdef _MSC_VER
#if _MSC_VER >= 1900
#pragma warning(disable : 4592)
#endif
#endif

namespace griddyn
{
class coreObject;
/** @brief      base class for helper objects
 Base class for all helper objects the main purpose is to deal with names and
 give a common interface for the various helper objects in GridDyn
**/
class helperObject
{
  private:
    static std::atomic<uint64_t> s_obcnt;  //!< static counter for a global object counter
    std::uint64_t m_oid;  //!< the identifier for the object
    std::string um_name;  //!< the text name of the object

  public:
    /** @brief default constructor*/
    helperObject () noexcept;

    explicit helperObject (std::string objectName);

    // don't allow copy constructors and assignment operator as they would introduce all sorts of other complicated
    // issues in the system
    helperObject (const helperObject &) = delete;
    void operator= (helperObject &obj) = delete;
    /** @brief default destructor  so it can be overridden*/
    virtual ~helperObject ();

    virtual void set (const std::string &param, const std::string &val);
    /**
     * @brief sets a numeric parameter of an object
     * @param[in] param the name of the parameter to change
     * @param[in] val the value of the parameter to set
     */
    virtual void set (const std::string &param, double val);
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
    virtual double get (const std::string &param) const;
    /**
     * helper function wrapper to return an int (instead of a double) from the get function
     * @param[in] param the name of the parameter to get
     * @return val the value of the parameter
     */
    inline int getInt (const std::string &param) const { return static_cast<int> (get (param)); }

    /** @brief set the name*/
    void setName (const std::string &newName)
    {
        um_name = newName;
        nameUpdate ();
    }

    /** @brief get the name of the object*/
    const std::string &getName () const noexcept { return um_name; }
    /** set the description of the object */
    void setDescription (const std::string &description);
    /** get the description of the object*/
    std::string getDescription () const;

    /**
     * @brief returns the oid of the object which is supposed to be a unique identifier
     */
    std::uint64_t getID () const noexcept { return m_oid; }
    /**
     * @brief updates the OID with a new number-useful in a few circumstances to ensure the id is higher than
     * another object
     */
    void makeNewOID ();

    /**
    @brief get an expected or actual owner of the helperObject
    @return the actual or targeted owner of the helperObject
    */
    virtual coreObject *getOwner () const;

  protected:
    /** a notification functions primary implemented by derived class
    @details the function is called when the object name is updated
    */
    virtual void nameUpdate ();
};

/** allow multiple flags to be set at once
@details the flags parameter can be a list of flags
"flag1, flag2, -flag3"
a negative sign in front of the flag indicates the flag should be turned off
@param[in] obj the helper object to set
@param[in] flags the list of flags to set
*/
void setMultipleFlags (helperObject *obj, const std::string &flags);
}  // namespace griddyn
