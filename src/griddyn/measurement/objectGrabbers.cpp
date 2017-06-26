/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "objectGrabbers.h"
#include "Area.h"
#include "Generator.h"
#include "Link.h"
#include "Load.h"
#include "Relay.h"
#include "gridBus.h"
#include "gridSubModel.h"
#include "relays/controlRelay.h"
#include "relays/sensor.h"

#include "solvers/solverMode.hpp"
#include "utilities/mapOps.hpp"
#include "utilities/stringOps.h"
#include <map>

namespace griddyn
{
using namespace gridUnits;

static const fobjectPair nullPair{nullptr, defUnit};
static const fvecPair nullVecPair{nullptr, defUnit};

/** map of all the alternate strings that can be used*/
static const std::map<std::string, std::string> stringTranslate{{"voltage", "v"},
                                                                {"vol", "v"},
                                                                {"link", "linkreal"},
                                                                {"linkp", "linkreal"},
                                                                {"linkepower", "linereal"},
                                                                {"loadq", "loadreactive"},
                                                                {"loadreactivepower", "loadreactive"},
                                                                {"load", "loadreal"},
                                                                {"loadp", "loadreal"},
                                                                {"loadpower", "loadreal"},
                                                                {"reactivegen", "genreactive"},
                                                                {"genq", "genreactive"},
                                                                {"gen", "genreal"},
                                                                {"generation", "genreal"},
                                                                {"genp", "genreal"},
                                                                {"genpower", "genreal"},
                                                                {"realgen", "genreal"},
                                                                {"f", "freq"},
                                                                {"frequency", "freq"},
                                                                {"omega", "freq"},
                                                                {"a", "angle"},
                                                                {"phase", "angle"},
                                                                {"busgenerationreal", "busgenreal"},
                                                                {"busp", "busgenreal"},
                                                                {"buspower", "busgenreal"},
                                                                {"busgen", "busgenreal"},
                                                                {"busload", "busloadreal"},
                                                                {"busloadp", "busloadreal"},
                                                                {"busloadq", "busloadreactive"},
                                                                {"busgenerationreactive", "busgenreactive"},
                                                                {"busq", "busgenreactive"},
                                                                {"linkrealpower", "linkreal"},
                                                                {"linerealpower", "linkreal"},
                                                                {"linkp1", "linkreal"},
                                                                {"linkq", "linkreactive"},
                                                                {"linkreactivepower", "linkreactive"},
                                                                {"linereactivepower", "linkreactive"},
                                                                {"linkrealpower1", "linkreal"},
                                                                {"linerealpower1", "linkreal"},
                                                                {"linkq1", "linkreactive"},
                                                                {"linkreactivepower1", "linkreactive"},
                                                                {"linereal", "linkreal"},
                                                                {"linkreal1", "linkreal"},
                                                                {"linereactive", "linkreactive"},
                                                                {"linkreactive1", "linkreactive"},
                                                                {"linkrealpower2", "linkreal2"},
                                                                {"linkq2", "linkreactive2"},
                                                                {"linkreactivepower2", "linkreactive2"},
                                                                {"linkp2", "linkreal2"},
                                                                {"p", "real"},
                                                                {"power", "real"},
                                                                {"q", "reactive"},
                                                                {"impedance", "z"},
                                                                {"admittance", "y"},
                                                                {"impedance1", "z"},
                                                                {"admittance1", "y"},
                                                                {"z1", "z"},
                                                                {"y1", "y"},
                                                                {"impedance2", "z2"},
                                                                {"admittance2", "y2"},
                                                                {"status", "connected"},
                                                                {"breaker", "switch"},
                                                                {"breaker1", "switch"},
                                                                {"switch1", "switch"},
                                                                {"breaker2", "switch2"},
                                                                {"i", "current"},
                                                                {"i1", "current"},
                                                                {"current1", "current"},
                                                                {"i2", "current2"},
                                                                {"imagcurrent1", "imagcurrent"},
                                                                {"realcurrent1", "realcurrent"},
                                                                {"lossreal", "loss"},
                                                                {"angle1", "angle"},
                                                                {"absangle1", "absangle"},
                                                                {"voltage1", "voltage"},
                                                                {"v1", "voltage"},
                                                                {"v2", "voltage2"},
                                                                {"output0", "output"},
                                                                {"cv", "output"},
                                                                {"o0", "output"},
                                                                {"currentvalue", "output"},
                                                                {"deriv0", "deriv"},
                                                                {"dodt", "deriv"},
                                                                {"dodt0", "deriv"},
                                                                {"doutdt", "deriv"},
                                                                {"doutdt0", "deriv"}

};

static const std::map<std::string, fobjectPair> objectFunctions{
  {"connected",
   {[](coreObject *obj) { return static_cast<double> (static_cast<gridComponent *> (obj)->isConnected ()); },
    defUnit}},
  {"enabled",
   {[](coreObject *obj) { return static_cast<double> (static_cast<gridComponent *> (obj)->isEnabled ()); },
    defUnit}},
  {"armed",
   {[](coreObject *obj) { return static_cast<double> (static_cast<gridComponent *> (obj)->isArmed ()); },
    defUnit}},
  {"output", {[](coreObject *obj) { return static_cast<gridComponent *> (obj)->getOutput (0); }, defUnit}},
  {"deriv",
   {[](coreObject *obj) {
        return static_cast<gridComponent *> (obj)->getDoutdt (noInputs, emptyStateData, cLocalbSolverMode, 0);
    },
    defUnit}}};

static const std::map<std::string, fobjectPair> busFunctions{
  {"v", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getVoltage (); }, puV}},
  {"angle", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getAngle (); }, rad}},
  {"freq", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getFreq (); }, puHz}},
  {"genreal", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getGenerationReal (); }, puMW}},
  {"genreactive", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getGenerationReactive (); }, puMW}},
  {"loadreal", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getLoadReal (); }, puMW}},
  {"loadreactive", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getLoadReactive (); }, puMW}},
  {"linkreal", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getLinkReal (); }, puMW}},
  {"linkreactive", {[](coreObject *obj) { return static_cast<gridBus *> (obj)->getLinkReactive (); }, puMW}},
};

static const std::map<std::string, fobjectPair> areaFunctions{
  {"avgfreq", {[](coreObject *obj) { return static_cast<Area *> (obj)->getAvgFreq (); }, puHz}},
  {"genreal", {[](coreObject *obj) { return static_cast<Area *> (obj)->getGenerationReal (); }, puMW}},
  {"genreactive", {[](coreObject *obj) { return static_cast<Area *> (obj)->getGenerationReactive (); }, puMW}},
  {"loadreal", {[](coreObject *obj) { return static_cast<Area *> (obj)->getLoadReal (); }, puMW}},
  {"loadreactive", {[](coreObject *obj) { return static_cast<Area *> (obj)->getLoadReactive (); }, puMW}},
  {"loss", {[](coreObject *obj) { return static_cast<Area *> (obj)->getLoss (); }, puMW}},
  {"tieflow", {[](coreObject *obj) { return static_cast<Area *> (obj)->getTieFlowReal (); }, puMW}},
};

static const std::map<std::string, fvecPair> areaVecFunctions{
  {"v",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getVoltage (data); }, puV}},
  {"angle",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getAngle (data); }, rad}},
  {"freq",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getFreq (data); }, puHz}},
  {"busgenreal",
   {[](coreObject *obj, std::vector<double> &data) {
        return static_cast<Area *> (obj)->getBusGenerationReal (data);
    },
    puMW}},
  {"busgenreactive",
   {[](coreObject *obj, std::vector<double> &data) {
        return static_cast<Area *> (obj)->getBusGenerationReactive (data);
    },
    puMW}},
  {"busloadreal",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getBusLoadReal (data); },
    puMW}},
  {"busloadreactive",
   {[](coreObject *obj, std::vector<double> &data) {
        return static_cast<Area *> (obj)->getBusLoadReactive (data);
    },
    puMW}},
  {"linkreal",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getLinkRealPower (data); },
    puMW}},
  {"linkreactive",
   {[](coreObject *obj, std::vector<double> &data) {
        return static_cast<Area *> (obj)->getLinkReactivePower (data);
    },
    puMW}},
  {"linkloss",
   {[](coreObject *obj, std::vector<double> &data) { return static_cast<Area *> (obj)->getLinkLoss (data); },
    puMW}},

};

static const std::map<std::string, descVecFunc> areaVecDescFunctions{
  {"v", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"angle", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"freq", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"busgenreal", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"busgenreactive",
   [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"busloadreal", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"busloadreactive",
   [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getBusName (desc); }},
  {"linkreal", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getLinkName (desc); }},
  {"linkreactive", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getLinkName (desc); }},
  {"linkloss", [](coreObject *obj, stringVec &desc) { return static_cast<Area *> (obj)->getLinkName (desc); }},

};

static const std::map<std::string, fobjectPair> loadFunctions{
  {"real", {[](coreObject *obj) { return static_cast<Load *> (obj)->getRealPower (); }, puMW}},
  {"reactive", {[](coreObject *obj) { return static_cast<Load *> (obj)->getReactivePower (); }, puMW}},
  {"loadreal", {[](coreObject *obj) { return static_cast<Load *> (obj)->getRealPower (); }, puMW}},
  {"loadreactive", {[](coreObject *obj) { return static_cast<Load *> (obj)->getReactivePower (); }, puMW}},
};

static const std::map<std::string, fobjectPair> genFunctions{
  {"real", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getRealPower (); }, puMW}},
  {"reactive", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getReactivePower (); }, puMW}},
  {"genreal", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getRealPower (); }, puMW}},
  {"genreactive", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getReactivePower (); }, puMW}},
  {"pset", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getPset (); }, puMW}},
  {"adjup", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getAdjustableCapacityUp (); }, puMW}},
  {"adjdown",
   {[](coreObject *obj) { return static_cast<Generator *> (obj)->getAdjustableCapacityDown (); }, puMW}},
  {"pmax", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getPmax (); }, puMW}},
  {"pmin", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getPmin (); }, puMW}},
  {"qmax", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getQmax (); }, puMW}},
  {"qmin", {[](coreObject *obj) { return static_cast<Generator *> (obj)->getQmin (); }, puMW}},
  {"freq",
   {[](coreObject *obj) { return static_cast<Generator *> (obj)->getFreq (emptyStateData, cLocalSolverMode); },
    puHz}},
  {"angle",
   {[](coreObject *obj) { return static_cast<Generator *> (obj)->getAngle (emptyStateData, cLocalSolverMode); },
    rad}},
};

static const std::map<std::string, fobjectPair> linkFunctions{
  {"real", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealPower (1); }, puMW}},
  {"reactive", {[](coreObject *obj) { return static_cast<Link *> (obj)->getReactivePower (1); }, puMW}},
  {"linkreal", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealPower (1); }, puMW}},
  {"linkreactive", {[](coreObject *obj) { return static_cast<Link *> (obj)->getReactivePower (1); }, puMW}},
  {"z", {[](coreObject *obj) { return static_cast<Link *> (obj)->getTotalImpedance (1); }, puOhm}},
  {"y", {[](coreObject *obj) { return 1.0 / (static_cast<Link *> (obj)->getTotalImpedance (1)); }, puOhm}},
  {"r", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealImpedance (1); }, puOhm}},
  {"x", {[](coreObject *obj) { return 1.0 / (static_cast<Link *> (obj)->getImagImpedance (1)); }, puOhm}},
  {"current", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getCurrent (1)); }, puA}},
  {"realcurrent", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getRealCurrent (1)); }, puA}},
  {"imagcurrent", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getImagCurrent (1)); }, puA}},
  {"voltage", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getVoltage (1)); }, puV}},
  {"absangle", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getAbsAngle (1)); }, rad}},
  // functions for to side
  {"real2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealPower (2); }, puMW}},
  {"reactive2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getReactivePower (2); }, puMW}},
  {"linkreal2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealPower (2); }, puMW}},
  {"linkreactive2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getReactivePower (2); }, puMW}},
  {"z2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getTotalImpedance (2); }, puOhm}},
  {"y2", {[](coreObject *obj) { return 1.0 / (static_cast<Link *> (obj)->getTotalImpedance (2)); }, puOhm}},
  {"r2", {[](coreObject *obj) { return static_cast<Link *> (obj)->getRealImpedance (2); }, puOhm}},
  {"x2", {[](coreObject *obj) { return 1.0 / (static_cast<Link *> (obj)->getImagImpedance (2)); }, puOhm}},
  {"current2", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getCurrent (2)); }, puA}},
  {"realcurrent2", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getRealCurrent (2)); }, puA}},
  {"imagcurrent2", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getImagCurrent (2)); }, puA}},
  {"voltage2", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getVoltage (2)); }, puV}},
  {"absangle2", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getAbsAngle (2)); }, rad}},

  // non numbered fields
  {"angle", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getAngle ()); }, rad}},
  {"loss", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getLoss ()); }, puMW}},
  {"lossreactive", {[](coreObject *obj) { return (static_cast<Link *> (obj)->getReactiveLoss ()); }, puMW}},
  {"attached",
   {[](coreObject *obj) {
        return static_cast<double> (((!static_cast<Link *> (obj)->checkFlag (Link::switch1_open_flag)) ||
                                     (!static_cast<Link *> (obj)->checkFlag (Link::switch2_open_flag))) &&
                                    (static_cast<Link *> (obj)->isEnabled ()));
    },
    defUnit}},
};

fobjectPair getObjectFunction (const gridComponent * /*comp*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = objectFunctions.find (nfstr);
    if (funcfind != objectFunctions.end ())
    {
        return funcfind->second;
    }
    std::string fld;
    auto num = stringOps::trailingStringInt (field, fld);
    if ((fld == "output") || (fld == "o") || (fld == "out"))
    {
        return {[num](coreObject *obj) { return static_cast<gridComponent *> (obj)->getOutput (num); }, defUnit};
    }
    if ((fld == "deriv") || (fld == "dodt") || (fld == "doutdt"))
    {
        return {[num](coreObject *obj) {
                    return static_cast<gridComponent *> (obj)->getDoutdt (noInputs, emptyStateData,
                                                                          cLocalbSolverMode, 0);
                },
                defUnit};
    }

    return nullPair;
}

fobjectPair getObjectFunction (const gridBus * /*bus*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);

    auto funcfind = busFunctions.find (nfstr);
    if (funcfind != busFunctions.end ())
    {
        return funcfind->second;
    }
    return getObjectFunction (static_cast<const gridComponent *> (nullptr), field);
}

fobjectPair getObjectFunction (const Load * /*ld*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = loadFunctions.find (nfstr);
    if (funcfind != loadFunctions.end ())
    {
        return funcfind->second;
    }

    return getObjectFunction (static_cast<const gridComponent *> (nullptr), field);
}

fobjectPair getObjectFunction (const Generator * /*gen*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = genFunctions.find (nfstr);
    if (funcfind != genFunctions.end ())
    {
        return funcfind->second;
    }

    return getObjectFunction (static_cast<const gridComponent *> (nullptr), field);
}

fobjectPair getObjectFunction (const Area * /*area*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = areaFunctions.find (nfstr);
    if (funcfind != areaFunctions.end ())
    {
        return funcfind->second;
    }

    return getObjectFunction (static_cast<const gridComponent *> (nullptr), field);
}

fobjectPair getObjectFunction (const Link * /*lnk*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = linkFunctions.find (nfstr);
    if (funcfind != linkFunctions.end ())
    {
        return funcfind->second;
    }

    return getObjectFunction (static_cast<const gridComponent *> (nullptr), field);
}

fobjectPair getObjectFunction (const Relay *rel, const std::string &field)
{
    fobjectPair retPair (nullptr, defUnit);
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = objectFunctions.find (nfstr);
    if (funcfind != objectFunctions.end ())
    {
        return funcfind->second;
    }
    std::string fld;
    int num = stringOps::trailingStringInt (field, fld, 0);
    if ((field == "cv") || (field == "currentvalue") || (field == "value") || (field == "output"))
    {
        retPair.first = [](coreObject *obj) -> double { return static_cast<Relay *> (obj)->getOutput (0); };
    }
    else if (fld == "status")
    {
        retPair.first = [num](coreObject *obj) {
            return static_cast<double> (static_cast<Relay *> (obj)->getConditionStatus (num));
        };
    }
    else if ((fld == "output") || (fld == "o"))
    {  // TODO:: PT figure out why the basic getOutput function doesn't work here
        retPair.first = [=](coreObject *obj) {
            return static_cast<Relay *> (obj)->getOutput (noInputs, emptyStateData, cLocalSolverMode, num);
        };
    }
    else if ((fld == "block") || (fld == "b"))
    {
        if (dynamic_cast<const sensor *> (rel) != nullptr)
        {
            retPair.first = [=](coreObject *obj) {
                return static_cast<sensor *> (obj)->getBlockOutput (emptyStateData, cLocalSolverMode, num);
            };
        }
    }
    else if ((fld == "condition") || (fld == "c"))
    {
        retPair.first = [num](coreObject *obj) { return static_cast<Relay *> (obj)->getConditionValue (num); };
    }
    else if ((fld == "input") || (fld == "i"))
    {
        if (dynamic_cast<const sensor *> (rel) != nullptr)
        {
            retPair.first = [num](coreObject *obj) {
                return static_cast<sensor *> (obj)->getInput (emptyStateData, cLocalSolverMode, num);
            };
        }
    }
    else
    {
        if (dynamic_cast<const sensor *> (rel) != nullptr)
        {
            // try to lookup named output for sensors
            index_t outIndex = static_cast<const sensor *> (rel)->lookupOutput (field);
            if (outIndex != kNullLocation)
            {
                retPair.first = [outIndex](coreObject *obj) {
                    return static_cast<sensor *> (obj)->getOutput (noInputs, emptyStateData, cLocalSolverMode,
                                                                   outIndex);
                };
            }
        }
        else if (dynamic_cast<const relays::controlRelay *> (rel) != nullptr)
        {
            if ((fld == "point") || (fld == "measurement"))
            {
                retPair.first = [num](coreObject *obj) {
                    return static_cast<relays::controlRelay *> (obj)->getMeasurement (num);
                };
            }
            else
            {
                // try to lookup named output for control relays
                index_t outIndex = static_cast<const relays::controlRelay *> (rel)->findMeasurement (field);
                if (outIndex != kNullLocation)
                {
                    retPair.first = [outIndex](coreObject *obj) {
                        return static_cast<relays::controlRelay *> (obj)->getMeasurement (outIndex);
                    };
                }
            }
        }
    }

    return retPair;
}

fobjectPair getObjectFunction (const gridSubModel *sub, const std::string &field)
{
    return getObjectFunction (static_cast<const gridComponent *> (sub), field);
}

fvecPair getObjectVectorFunction (const gridComponent *comp, const std::string & /*field*/)
{
    // TODO:: add a vector function for all the outputs
    static_cast<void> (comp);
    return nullVecPair;
}

fvecPair getObjectVectorFunction (const Area * /*area*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = areaVecFunctions.find (nfstr);
    if (funcfind != areaVecFunctions.end ())
    {
        return funcfind->second;
    }
    return nullVecPair;
}

descVecFunc getObjectVectorDescFunction (const gridComponent *comp, const std::string & /*field*/)
{
    static_cast<void> (comp);
    return nullptr;
}

descVecFunc getObjectVectorDescFunction (const Area * /*area*/, const std::string &field)
{
    std::string nfstr = mapFind (stringTranslate, field, field);
    auto funcfind = areaVecDescFunctions.find (nfstr);
    if (funcfind != areaVecDescFunctions.end ())
    {
        return funcfind->second;
    }
    return nullptr;
}

}  // namespace griddyn