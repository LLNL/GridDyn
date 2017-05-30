/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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

#include "loadFMIExportObjects.h"
#include "core/factoryTemplates.h"
#include "fmiCollector.h"
#include "fmiEvent.h"
#include "gridDynFileInput/readerInfo.h"

static childClassFactory<fmiCollector, collector>
  fmiColFac (std::vector<std::string>{"fmioutput", "fmicollector"});

static childClassFactoryArg<fmiEvent, gridEvent, fmiEvent::fmiEventType>
  fmiIn (std::vector<std::string>{"fmiinput", "fmievent"}, fmiEvent::fmiEventType::input);

static childClassFactoryArg<fmiEvent, gridEvent, fmiEvent::fmiEventType>
  fmiParam (std::vector<std::string>{"fmiparam", "fmiparameter"}, fmiEvent::fmiEventType::parameter);
// static childClassFactory<dimeCommunicator, gridCommunicator> dimeComm(std::vector<std::string>{"dime"});

void loadFMIExportObjects () {}

void loadFmiExportReaderInfoDefinitions (readerInfo &ri)
{
    ri.addTranslate ("fmiparam", "event");
    ri.addTranslate ("fmiinput", "event");
    ri.addTranslate ("fmioutput", "collector");
}