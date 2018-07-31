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

#include "../gridDynDefinitions.hpp"
#include "core/objectOperatorInterface.hpp"
#include "utilities/workQueue.h"
#include <future>
#include <memory>
#include <string>
#include <vector>

// limit violation definitions
#define NO_VIOLATION 0
#define VOLTAGE_OVER_LIMIT_VIOLATION 10
#define VOLTAGE_UNDER_LIMIT_VIOLATION 11
#define MVA_EXCEED_RATING_A 13
#define MVA_EXCEED_RATING_B 14
#define MVA_EXCEED_ERATING 15

#define MINIMUM_ANGLE_EXCEEDED 20
#define MAXIMUM_ANGLE_EXCEEDED 21
#define MINIMUM_CURRENT_EXCEEDED 30
#define MAXIMUM_CURRENT_EXCEEDED 31

#define CONVERGENCE_FAILURE 45

namespace griddyn
{
class gridDynSimulation;

/** class encapsulating the data needed to record a violation */
class violation
{
  public:
    std::string m_objectName;  //!< the  name of the object with the violation
    double level = 0.0;  //!< the value of the parameter exceeding some limit
    double limit = 0.0;  //!< the limit value
    double percentViolation = 100;  //!< the violation percent;
    int contingency_id = 0;  //!< usually added later or ignored
    int violationCode;  //!< a code representing the type of violation
    int severity = 0;  //!< a code indicating the severity of the violation
    violation (const std::string &name = "", int code = 0) : m_objectName (name), violationCode (code) {}
    /** @brief encode the violation to a string
    @return the violation string*/
    std::string to_string () const;
};

class Event;

enum class contingency_mode_t
{
    N_1,
    N_1_1,
    N_2,
    line,
    gen,
    load,
    bus,
    custom,
    unknown
};

class contingency;
/** convert a string to a contingency mode*/
contingency_mode_t getContingencyMode (const std::string &mode);
/** class defining some extra optional info used for building contingency lists */
class extraContingencyInfo
{
  public:
    std::shared_ptr<contingency> baseCont;  //!< pointer to the base contingency
    double cutoff = 0.0;  //!< the threshold level to trigger
    double delta = 0.0;  //!< the change in data
    int stage = 0;  //!< which stage should the contingency execute in
    extraContingencyInfo () = default;
};

/** an object pointing to empty information*/
const extraContingencyInfo emptyExtraInfo{};
/** class that encapsulated the information about a contingency
 */
class contingency : public basicWorkBlock, objectOperatorInterface
{
  private:
    static std::atomic_int contingencyCount;  //!< static variable counting the number of created contingencies
  public:
    std::string name;  //!< contingency name
    int id;  //!< contingency id
    std::atomic<bool> completed{false};  //!< boolean indicator if the contingency was run

    std::vector<violation> Violations;  //!< the resulting violations
    double PI = 0.0;  //!< performance index score
    double lowV = 0.0;  //!< minimum voltage
    std::vector<double> busVoltages;  //!< vector of bus voltages
    std::vector<double> busAngles;  //!< vector of bus Angles
    std::vector<double> Lineflows;  //!< vector of transmission line flows
  protected:
    gridDynSimulation *gds = nullptr;  //!< master simulation object
    std::promise<int> promise_val;  //!< paired with future for asynchronous operation
    std::shared_future<int> future_ret;  //!< the future object to contain the data that will come upon execution
    std::vector<std::vector<std::shared_ptr<Event>>> eventList;  //!< events that describe the contingency
  public:
    /** default constructor*/
    contingency ();
    /** construct from a sim and event*/
    contingency (gridDynSimulation *sim, std::shared_ptr<Event> ge = nullptr);
    /** run the contingency
     */
    virtual void execute () override;

    virtual bool isFinished () const override;

    /** set the contingency root object
    @param[in] gdSim  a gridDynSimulation object that is the basis for the contingencies
    */
    void setContingencyRoot (gridDynSimulation *gdSim);
    /** add an event to a contingency
    @param[in] ge the new Event to add to the contingency
      @param[in] stage  the stage to execute the contingency
    */
    void add (std::shared_ptr<Event> ge, index_t stage = 0);
    /** generate a header string for a csv file including the data field names this
    @details this header line would be general to all similar contingencies
    */
    std::string generateHeader () const;
    /** generate an output line for a csv file containing the contingency result data and any violations
     */
    std::string generateFullOutputLine () const;
    /** generate an output string containing just the contingency and any violations
     *
     */
    std::string generateViolationsOutputLine () const;

    /** reset the contingency to be able to execute again*/
    void reset ();
    /** wait for the contingency to finish executing*/
    void wait () const;

    coreObject *getObject () const override;

    void getObjects (std::vector<coreObject *> &objects) const override;

    void updateObject (coreObject *newObj, object_update_mode mode = object_update_mode::match) override;

    std::shared_ptr<contingency> clone (std::shared_ptr<contingency> con = nullptr) const;
};
// Contingency execution functions
/** @brief build a list of contingencies
@param[in] contMode a string with the type of contingency analysis
@return a vector of contingencies
*/
std::vector<std::shared_ptr<contingency>> buildContingencyList (gridDynSimulation *gds,
                                                                const std::string &contMode,
                                                                const extraContingencyInfo &info = emptyExtraInfo);

/** @brief add a list of contingencies to an existing list of contingencies
 *@param[in] gds the simulation root object
@param[in] cmode a string with the type of contingency analysis
@param[out] contList the list of existing contingencies that
@return the number of contingencies added to the list
*/
size_t buildContingencyList (gridDynSimulation *gds,
                             contingency_mode_t cmode,
                             std::vector<std::shared_ptr<contingency>> &contList,
                             const extraContingencyInfo &info = emptyExtraInfo);

/** @brief perform a contingency analysis
@param[in] contList the list of specific contingencies to test
@param[in] output a string containing the output specs (either fileName or some other string
*/
void runContingencyAnalysis (std::vector<std::shared_ptr<contingency>> &contList, const std::string &output);

}  // namespace griddyn
