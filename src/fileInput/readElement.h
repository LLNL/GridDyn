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

#ifndef GRIDDYNREADELEMENT_H_
#define GRIDDYNREADELEMENT_H_

// headers

#include "fileInput.h"
#include "readerHelper.h"
#include <memory>
#include <unordered_set>

// forward declarations
class readerElement;

namespace griddyn
{
class gridParameter;
class helperObject;

// struct for holding and passing the information in Element reader files
class readerInfo;

class coreObject;

class zipLoad;
class Generator;
class Area;
class Link;
class EventInfo;
class gridSimulation;
class gridDynSimulation;
class Relay;
class gridBus;
class gridPrimary;
class gridSecondary;
class gridSubModel;

gridBus *
readBusElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject = nullptr);
Relay *
readRelayElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject = nullptr);

// zipLoad * readLoadElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject =
// nullptr); Generator * readGeneratorElement (std::shared_ptr<readerElement> &element, readerInfo *ri, coreObject
// *searchObject = nullptr);
Link *readLinkElement (std::shared_ptr<readerElement> &element,
                       readerInfo &ri,
                       coreObject *searchObject = nullptr,
                       bool warnlink = true);
Area *
readAreaElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject = nullptr);
gridSimulation *readSimulationElement (std::shared_ptr<readerElement> &element,
                                       readerInfo &ri,
                                       coreObject *searchObject = nullptr,
                                       gridSimulation *gs = nullptr);

coreObject *
readEconElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *searchObject = nullptr);
void readArrayElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject);
void loadConditionElement (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject);
void loadSubObjects (std::shared_ptr<readerElement> &element, readerInfo &ri, coreObject *parentObject);

void readImports (std::shared_ptr<readerElement> &element,
                  readerInfo &ri,
                  coreObject *parentObject,
                  bool finalFlag);

void loadDefines (std::shared_ptr<readerElement> &element,
                  readerInfo &ri);  // NOTE: defined in readLibraryElement.cpp
void loadDirectories (std::shared_ptr<readerElement> &element,
                      readerInfo &ri);  // NOTE: defined in readLibraryElement.cpp
void loadTranslations (std::shared_ptr<readerElement> &element,
                       readerInfo &ri);  // NOTE: defined in readLibraryElement.cpp
void loadCustomSections (std::shared_ptr<readerElement> &element,
                         readerInfo &ri);  // NOTE: defined in readLibraryElement.cpp

void loadSolverElement (std::shared_ptr<readerElement> &element, readerInfo &ri, gridDynSimulation *parentObject);
void readLibraryElement (std::shared_ptr<readerElement> &element, readerInfo &ri);

using IgnoreListType = std::unordered_set<std::string>;
// using IgnoreListType = boost::container::flat_set<std::string>;

void loadElementInformation (coreObject *obj,
                             std::shared_ptr<readerElement> &element,
                             const std::string &objectName,
                             readerInfo &ri,
                             const IgnoreListType &ignoreList);

void objSetAttributes (coreObject *obj,
                       std::shared_ptr<readerElement> &element,
                       const std::string &component,
                       readerInfo &ri,
                       const IgnoreListType &ignoreList);

void paramLoopElement (coreObject *obj,
                       std::shared_ptr<readerElement> &element,
                       const std::string &component,
                       readerInfo &ri,
                       const IgnoreListType &ignoreList);

void setParams (helperObject *obj,
                std::shared_ptr<readerElement> &element,
                const std::string &component,
                readerInfo &ri,
                const IgnoreListType &ignoreList);
void setAttributes (helperObject *obj,
                    std::shared_ptr<readerElement> &element,
                    const std::string &component,
                    readerInfo &ri,
                    const IgnoreListType &ignoreList);

int loadEventElement (std::shared_ptr<readerElement> &element, coreObject *obj, readerInfo &ri);
int loadCollectorElement (std::shared_ptr<readerElement> &element, coreObject *obj, readerInfo &ri);

gridParameter getElementParam (const std::shared_ptr<readerElement> &element);
void getElementParam (const std::shared_ptr<readerElement> &element, gridParameter &param);

std::string findElementName (std::shared_ptr<readerElement> &element,
                             const std::string &ename,
                             readerConfig::match_type matching = readerConfig::match_type::strict_case_match);

std::string getElementField (std::shared_ptr<readerElement> &element,
                             const std::string &ename,
                             readerConfig::match_type matching = readerConfig::match_type::strict_case_match);
std::string getElementAttribute (std::shared_ptr<readerElement> &element,
                                 const std::string &ename,
                                 readerConfig::match_type matching = readerConfig::match_type::strict_case_match);
std::string
getElementFieldOptions (std::shared_ptr<readerElement> &element,
                        const stringVec &names,
                        readerConfig::match_type matching = readerConfig::match_type::strict_case_match);
stringVec
getElementFieldMultiple (std::shared_ptr<readerElement> &element,
                         const std::string &ename,
                         readerConfig::match_type matching = readerConfig::match_type::strict_case_match);

void setIndex (std::shared_ptr<readerElement> &element, coreObject *mobj, readerInfo &ri);
std::string getObjectName (std::shared_ptr<readerElement> &element, readerInfo &ri);

coreObject *getParent (std::shared_ptr<readerElement> &element,
                       readerInfo &ri,
                       coreObject *parentObject,
                       const std::string &alternateName = "");

// This set of constants and functions is to allow templating of the object type but getting an alternative string
// for the parent type
const std::string emptyString = "";
const std::string areaTypeString = "area";
const std::string busTypeString = "bus";

inline const std::string &parentSearchComponent (coreObject *) { return emptyString; }

inline const std::string &parentSearchComponent (gridPrimary *) { return areaTypeString; }

inline const std::string &parentSearchComponent (gridSecondary *) { return busTypeString; }
}  // namespace griddyn
#endif
