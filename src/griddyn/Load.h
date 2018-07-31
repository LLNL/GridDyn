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

#include "gridSecondary.h"

namespace griddyn
{
class gridBus;

/** primary load class defines the load component is a very simple constant power load
 */
class Load : public gridSecondary
{
  public:
    /** flags used in the load objec*/
    enum load_flags
    {
        use_power_factor_flag = object_flag1,  //!< flag instructing the load to use a power factor to compute Q
    };
    static std::atomic<count_t> loadCount;  //!< counter for automatic load id's
  protected:
    double P = 0.0;  //!< [pu] real component of the load (constant Power)
    double Q = 0.0;  //!< [pu] imaginary component of the load (constant Power)
    parameter_t pfq = 0.0;  //!< power factor multiply  sqrt((1-pf*pf)/pf*pf)
  public:
    /** constructor which takes the object name*/
    explicit Load (const std::string &objName = "load_$");
    /** alternate constructor taking p and q values in addition to the name
    @param[in] rP the real power consumption of the load
    @param[in] rQ the reactive power consumption of the load
    @objName the name of the load object*/
    Load (double rP, double rQ, const std::string &objName = "load_$");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;

    virtual void getParameterStrings (stringVec &pstr, paramStringType pstype) const override;

    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    virtual void setFlag (const std::string &flag, bool val = true) override;

    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

    /** set the real output power with the specified units
    @param[in] level the real power output setting
    @param[in] unitType the units on the real power
      */
    virtual void setLoad (double level, gridUnits::units_t unitType = gridUnits::defUnit);
    /** set the real and reactive output power with the specified units
    @param[in] Plevel the real power output setting
    @param[in] Qlevel the reactive power output setting
    @param[in] unitType the units on the real power
    */
    virtual void setLoad (double Plevel, double Qlevel, gridUnits::units_t unitType = gridUnits::defUnit);

    virtual double
    getRealPower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    virtual double
    getReactivePower (const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
    /** get the real output power that based on the given voltage
    @param[in] V the bus voltage
    @return the real power consumed by the load*/
    virtual double getRealPower (double V) const;
    /** get the reactive output power that based on the given voltage
    @param[in] V the bus voltage
    @return the reactive power consumed by the load*/
    virtual double getReactivePower (double V) const;
    virtual double getRealPower () const override;
    virtual double getReactivePower () const override;  // for saving the state

    count_t outputDependencyCount (index_t num, const solverMode &sMode) const override;

  protected:
    // little helper functions to do some calculations
    /** update the power factor ratio*/
    void updatepfq ();
    /** check if the pfq is valid*/
    void checkpfq ();
    // getters for the actual property values
    /** get the real power output*/
    double getP () const { return P; }
    /** get the reactive power output*/
    double getQ () const { return Q; }

    // setters for the actual load values
    /** set the real power consumption*/
    void setP (double newP);
    /** set the reactive power consumption*/
    void setQ (double newQ);
    /** check if there was a change in the load around a fault and if so notify the bus*/
    void checkFaultChange ();

  private:
    /** due some additional work required by the constructor*/
    void constructionHelper ();
};

}  // namespace griddyn
