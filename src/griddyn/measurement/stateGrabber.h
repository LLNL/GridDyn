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

#ifndef STATE_GRABBER_H_
#define STATE_GRABBER_H_
#pragma once

#include "../events/eventInterface.hpp"
#include "../gridComponent.h"
#include "core/objectOperatorInterface.hpp"
#include <functional>
/** @file
@brief define a state Grabber object to retrieve data from a state information
*/
namespace griddyn
{
using objJacFunction =
  std::function<void(gridComponent *comp, const stateData &sD, matrixData<double> &md, const solverMode &sMode)>;
using objStateGrabberFunction =
  std::function<double(gridComponent *comp, const stateData &sD, const solverMode &sMode)>;

/** define if the grabber can compute the Jacobian information*/
enum class jacobian_mode
{
    none,  //!< no Jacobian computed
    direct,  //!< the result is a state directly
    computed,  //!< the Jacobian needs to be computed
};

/**class for grabbing a subset of fields directly from the state vector for performing certain calculations
 */
class stateGrabber : public objectOperatorInterface
{
  public:
    std::string field;  //!< name of the field to capture

    gridUnits::units_t outputUnits = gridUnits::defUnit;  //!< desired output units
    gridUnits::units_t inputUnits = gridUnits::defUnit;  //!< units of the input
    index_t offset = kInvalidLocation;  //!< the state offset location
    bool loaded = false;  //!< flag indicating the grabber is loaded
    bool cacheUpdateRequired = false;  //!< flag indicating that the cache should be updated before the call
    double gain = 1.0;  //!< multiplier on the input
    double bias = 0.0;  //!<  bias on the input

  protected:
    jacobian_mode jacMode = jacobian_mode::none;  //!< the mode of the Jacobian calculation
    gridComponent *cobj = nullptr;  //!< the target object
    objStateGrabberFunction fptr;  //!< the functional to grab the data
    objJacFunction jacIfptr;  //!< the functional to compute the Jacobian
    index_t prevIndex = kInvalidLocation;  //!< temporary storage of the previous index
  public:
    stateGrabber () = default;
    explicit stateGrabber (coreObject *obj);

    stateGrabber (const std::string &fld, coreObject *obj);
    stateGrabber (index_t noffset, coreObject *obj);
    /** clone the grabber*/
    virtual std::unique_ptr<stateGrabber> clone () const;
    /** clone a grabber to another grabber
    @param[in] sgb a pointer to a grabber to clone to
    */
    virtual void cloneTo (stateGrabber *sgb) const;

    /** update the target field of a grabber
    @param[in] fld the new target string of a grabber
    */
    virtual void updateField (const std::string &fld);
    /** retrieve the target data associated with a grabber
    @param[in] sD the stateData to grab the data from
    @param[in] sMode the solver mode associated with the stateData
    */
    virtual double grabData (const stateData &sD, const solverMode &sMode);
    /** compute the partial derivatives of a grabber
    @param[in] sD the stateData for computing the information
    @param[in] md the  matrix to store the computed Jacobian information into
    @param[in] sMode the solverMode associated with the stateData*/
    virtual void outputPartialDerivatives (const stateData &sD, matrixData<double> &md, const solverMode &sMode);
    virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
    virtual coreObject *getObject () const override;
    virtual void getObjects (std::vector<coreObject *> &objects) const override;
    /** get the Jacobian abilities of a grabber*/
    jacobian_mode getJacobianMode () const { return jacMode; }

  protected:
    /** load bus specific grabber info*/
    void busLoadInfo (const std::string &fld);
    /** load link specific grabber info*/
    void linkLoadInfo (const std::string &fld);
    /** load relay specific grabber info*/
    void relayLoadInfo (const std::string &fld);
    /** load gridSecondary specific grabber info*/
    void secondaryLoadInfo (const std::string &fld);
    /** load area specific grabber info*/
    void areaLoadInfo (const std::string &fld);
    /** load generic object info*/
    void objectLoadInfo (const std::string &fld);
};

using fstateobjectPair =
  std::pair<std::function<double(gridComponent *, const stateData &sD, const solverMode &sMode)>,
            gridUnits::units_t>;

/** construct a vector of state grabbers from a specific command string
@param[in] command the string command to generate the grabbers
@param[in] obj the root object to start any searches from
@return a vector of unique_ptrs to stateGrabbers containing all the generated grabbers
*/
std::vector<std::unique_ptr<stateGrabber>> makeStateGrabbers (const std::string &command, coreObject *obj);

/**
class with an additional capability of a totally custom function grabber call
*/
class customStateGrabber : public stateGrabber
{
  public:
    customStateGrabber () = default;
    explicit customStateGrabber (gridComponent *comp);
    virtual std::unique_ptr<stateGrabber> clone () const override;
    virtual void cloneTo (stateGrabber *sgb) const override;
    /** set the custom grabber function
    @param[in] nfptr the custom function for grabbing a state value
    */
    void setGrabberFunction (objStateGrabberFunction nfptr);
    /** set the custom Jacobian function related to a state Grabber
    @param[in] nJfptr the custom function for generating Jacobian information for a stateGrabber
    */
    void setGrabberJacFunction (objJacFunction nJfptr);
};

/** function operation on a state grabber*/
class stateFunctionGrabber : public stateGrabber
{
  public:
  protected:
    std::shared_ptr<stateGrabber> bgrabber;  //!< the grabber that gets the data that the function operates on
    std::string function_name;  //!< the name of the function
    std::function<double(double val)> opptr;  //!< function object

  public:
    stateFunctionGrabber () = default;
    stateFunctionGrabber (std::shared_ptr<stateGrabber> ggb, std::string func);
    virtual std::unique_ptr<stateGrabber> clone () const override;
    virtual void cloneTo (stateGrabber *sgb) const override;
    virtual double grabData (const stateData &sD, const solverMode &sMode) override;
    virtual void
    outputPartialDerivatives (const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
    virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
    virtual coreObject *getObject () const override;
    virtual void updateField (const std::string &fld) override;
};

/** a state grabber with operation or two argument functions*/
class stateOpGrabber : public stateGrabber
{
  protected:
    std::shared_ptr<stateGrabber> bgrabber1;  //!< grabber 1 as the first argument
    std::shared_ptr<stateGrabber> bgrabber2;  //!< grabber 2 as the second argument
    std::string op_name;  //!< the name of the operation
    std::function<double(double val1, double val2)> opptr;  //!< function pointer for a two argument function

  public:
    /** default constructor*/
    stateOpGrabber () = default;
    /** construct from two state grabbers and a operation*/
    stateOpGrabber (std::shared_ptr<stateGrabber> ggb1, std::shared_ptr<stateGrabber> ggb2, std::string op);
    virtual std::unique_ptr<stateGrabber> clone () const override;
    virtual void cloneTo (stateGrabber *sgb) const override;
    virtual double grabData (const stateData &sD, const solverMode &sMode) override;
    virtual void
    outputPartialDerivatives (const stateData &sD, matrixData<double> &md, const solverMode &sMode) override;
    virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
    /** overload for updating an object to a specific number of the underlying stateGrabbers
    @param[in] obj the new targetObject
    @param[in] num the index of the underlying state grabber to update
    */
    void updateObject (coreObject *obj, int num);
    virtual coreObject *getObject () const override;
    virtual void updateField (const std::string &opName) override;
};

}  // namespace griddyn

#endif
