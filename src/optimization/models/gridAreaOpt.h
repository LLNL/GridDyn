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

#ifndef GRIDAREAOPT_H_
#define GRIDAREAOPT_H_

// headers

#include "../gridOptObjects.h"

#include "core/coreObjectList.h"
// forward classes

namespace griddyn
{
class gridBusOpt;
class gridLinkOpt;
class gridRelayOpt;

class Area;

class gridAreaOpt : public gridOptObject
{
  public:
  protected:
    std::vector<gridAreaOpt *> areaList;
    std::vector<gridBusOpt *> busList;
    std::vector<gridLinkOpt *> linkList;
    std::vector<gridRelayOpt *> relayList;

    std::vector<gridOptObject *> objectList;
    Area *area = nullptr;

    coreObjectList optObList;  // a search index for object names
  public:
    gridAreaOpt (const std::string &objName = "");
    gridAreaOpt (coreObject *obj, const std::string &objName = "");
    ~gridAreaOpt ();

    virtual coreObject *clone (coreObject *obj = nullptr) const override;
    // add components

    void add (coreObject *obj) override;
    void add (gridAreaOpt *pl);
    void add (gridBusOpt *gen);
    void add (gridLinkOpt *lnk);
    void add (gridRelayOpt *rel);

    // remove components
    void remove (coreObject *obj) override;
    void remove (gridAreaOpt *pl);
    void remove (gridBusOpt *gen);
    void remove (gridLinkOpt *lnk);
    void remove (gridRelayOpt *rel);

    virtual void dynObjectInitializeA (std::uint32_t flags) override;
    virtual void loadSizes (const optimMode &oMode) override;
    virtual void setValues (const optimData &oD, const optimMode &oMode) override;
    // for saving the state
    virtual void guessState (double time, double val[], const optimMode &oMode) override;
    virtual void getTols (double tols[], const optimMode &oMode) override;
    virtual void getVariableType (double sdata[], const optimMode &oMode) override;

    virtual void valueBounds (double time, double upLimit[], double lowerLimit[], const optimMode &oMode) override;

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

    virtual void disable () override;
    // parameter set functions
    virtual void setOffsets (const optimOffsets &newOffsets, const optimMode &oMode) override;
    virtual void setOffset (index_t offset, index_t constraintOffset, const optimMode &oMode) override;

    virtual void setAll (const std::string &type,
                         const std::string &param,
                         double val,
                         gridUnits::units_t unitType = gridUnits::defUnit);
    virtual void set (const std::string &param, const std::string &val) override;
    virtual void
    set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
    // parameter get functions
    virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

    virtual bool isMember (coreObject *obj) const;
    // find components

    virtual coreObject *find (const std::string &objName) const override;
    virtual coreObject *getSubObject (const std::string &typeName, index_t num) const override;
    virtual coreObject *findByUserID (const std::string &typeName, index_t searchID) const override;

    virtual gridOptObject *getBus (index_t index) const override;
    virtual gridOptObject *getArea (index_t index) const override;
    virtual gridOptObject *getRelay (index_t index) const override;
    virtual gridOptObject *getLink (index_t index) const override;

  protected:
};

gridAreaOpt *getMatchingArea (gridAreaOpt *area, gridOptObject *src, gridOptObject *sec);

}  // namespace griddyn
#endif
