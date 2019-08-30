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

// headers
#include "../gridOptObjects.h"
// forward classes

namespace griddyn
{
class gridLinkOpt;
class gridLoadOpt;
class gridGenOpt;
class gridBusOpt;
class Generator;

class gridGenOpt : public gridOptObject
{
  public:
    enum optgen_flags
    {
        piecewise_linear_cost = 1,
        limit_override = 2,
    };

  protected:
    Generator *gen = nullptr;
    gridBusOpt *bus;
    double m_heatRate = -kBigNum;
    std::vector<double> Pcoeff;
    std::vector<double> Qcoeff;
    double m_penaltyCost = 0;
    double m_fuelCost = -1;
    double m_Pmax = kBigNum;
    double m_Pmin = kBigNum;
    double m_forecast = -kBigNum;
    double systemBasePower = 100.0;  //!< the base power of the generator
    double mBase = 100.0;  //!< the machine base of the generator
  public:
    gridGenOpt (const std::string &objName = "");
    gridGenOpt (coreObject *obj, const std::string &objName = "");

    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    // add components

    virtual void add (coreObject *obj) override;
    virtual void dynObjectInitializeA (std::uint32_t flags) override;
    virtual void loadSizes (const optimMode &oMode) override;

    virtual void setValues (const optimData &oD, const optimMode &oMode) override;
    // for saving the state
    virtual void guessState (double time, double val[], const optimMode &oMode) override;
    virtual void getTols (double tols[], const optimMode &oMode) override;
    virtual void getVariableType (double sdata[], const optimMode &oMode) override;

    virtual void valueBounds (double time, double upperLimit[], double lowerLimit[], const optimMode &oMode) override;

    virtual void linearObj (const optimData &oD, vectData<double> &linObj, const optimMode &oMode) override;
    virtual void quadraticObj (const optimData &oD,
                               vectData<double> &linObj,
                               vectData<double> &quadObj,
                               const optimMode &oMode) override;

    virtual double objValue (const optimData &oD, const optimMode &oMode) override;
    virtual void gradient (const optimData &oD, double deriv[], const optimMode &oMode) override;
    virtual void jacobianElements (const optimData &oD, matrixData<double> &md, const optimMode &oMode) override;
    virtual void getConstraints (const optimData &oD,
                                 matrixData<double> &cons,
                                 double upperLimit[],
                                 double lowerLimit[],
                                 const optimMode &oMode) override;
    virtual void constraintValue (const optimData &oD, double cVals[], const optimMode &oMode) override;
    virtual void
    constraintJacobianElements (const optimData &oD, matrixData<double> &md, const optimMode &oMode) override;
    virtual void getObjName (stringVec &objNames, const optimMode &oMode, const std::string &prefix = "") override;

    // parameter set functions
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, units::unit unitType = units::defunit) override;
    // parameter get functions
    virtual double get (const std::string &param, units::unit unitType = units::defunit) const override;

    virtual void loadCostCoeff (std::vector<double> const &coeff, int mode);
    // find components

    virtual gridOptObject *getBus (index_t index) const override;
    virtual gridOptObject *getArea (index_t index) const override;

  protected:
};

}  // namespace griddyn
