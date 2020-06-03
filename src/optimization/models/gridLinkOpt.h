/*
 * Copyright (c) 2014-2020, Lawrence Livermore National Security
 * See the top-level NOTICE for additional details. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

// headers
#include "../gridOptObjects.h"
// forward classes

namespace griddyn {
class gridLoadOpt;
class gridGenOpt;
class gridBusOpt;

class Link;

class gridLinkOpt: public gridOptObject {
  public:
    enum bus_flags {

    };

  protected:
    gridBusOpt* B1;
    gridBusOpt* B2;

    Link* link = nullptr;
    double rampUpLimit;
    double rampDownLimit;

  public:
    gridLinkOpt(const std::string& objName = "");
    gridLinkOpt(coreObject* obj, const std::string& objName = "");
    ~gridLinkOpt();

    virtual coreObject* clone(coreObject* obj = nullptr) const override;
    // add components
    virtual void add(coreObject* obj) override;

    // remove components
    virtual void remove(coreObject* obj) override;

    virtual void dynObjectInitializeA(std::uint32_t flags) override;
    virtual void loadSizes(const optimMode& oMode) override;

    virtual void setValues(const optimData& of, const optimMode& oMode) override;
    // for saving the state
    virtual void guessState(double time, double val[], const optimMode& oMode) override;
    virtual void getTols(double tols[], const optimMode& oMode) override;
    virtual void getVariableType(double sdata[], const optimMode& oMode) override;

    virtual void valueBounds(double time,
                             double upLimit[],
                             double lowerLimit[],
                             const optimMode& oMode) override;

    virtual void
        linearObj(const optimData& of, vectData<double>& linObj, const optimMode& oMode) override;
    virtual void quadraticObj(const optimData& of,
                              vectData<double>& linObj,
                              vectData<double>& quadObj,
                              const optimMode& oMode) override;

    virtual double objValue(const optimData& of, const optimMode& oMode) override;
    virtual void gradient(const optimData& of, double deriv[], const optimMode& oMode) override;
    virtual void jacobianElements(const optimData& of,
                                  matrixData<double>& md,
                                  const optimMode& oMode) override;
    virtual void getConstraints(const optimData& of,
                                matrixData<double>& cons,
                                double upperLimit[],
                                double lowerLimit[],
                                const optimMode& oMode) override;
    virtual void
        constraintValue(const optimData& of, double cVals[], const optimMode& oMode) override;
    virtual void constraintJacobianElements(const optimData& of,
                                            matrixData<double>& md,
                                            const optimMode& oMode) override;
    virtual void getObjName(stringVec& objNames,
                            const optimMode& oMode,
                            const std::string& prefix = "") override;

    virtual void disable() override;
    // parameter set functions

    virtual void setOffsets(const optimOffsets& newOffsets, const optimMode& oMode) override;

    virtual void set(const std::string& param, const std::string& val) override;
    virtual void
        set(const std::string& param, double val, units::unit unitType = units::defunit) override;
    // parameter get functions
    virtual double get(const std::string& param,
                       units::unit unitType = units::defunit) const override;

    // find components
    virtual coreObject* find(const std::string& objName) const override;
    virtual coreObject* getSubObject(const std::string& typeName, index_t num) const override;
    virtual coreObject* findByUserID(const std::string& typeName, index_t searchID) const override;

    virtual gridOptObject* getBus(index_t index) const override;
    virtual gridOptObject* getArea(index_t index) const override;

  protected:
};

}  // namespace griddyn
