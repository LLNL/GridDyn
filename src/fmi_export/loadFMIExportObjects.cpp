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
#include "core/factoryTemplates.hpp"
#include "fmiCoordinator.h"
#include "core/objectFactoryTemplates.hpp"
#include "fmiCollector.h"
#include "fmiEvent.h"
#include "fileInput/readerInfo.h"

namespace griddyn
{
static childClassFactory<fmi::fmiCollector, collector>
  fmiColFac (std::vector<std::string>{"fmioutput", "fmicollector"});

static childClassFactoryArg<fmi::fmiEvent, Event, fmi::fmiEvent::fmiEventType>
  fmiIn (std::vector<std::string>{"fmiinput", "fmievent"}, fmi::fmiEvent::fmiEventType::input);

static childClassFactoryArg<fmi::fmiEvent, Event, fmi::fmiEvent::fmiEventType>
  fmiParam (std::vector<std::string>{"fmiparam", "fmiparameter"}, fmi::fmiEvent::fmiEventType::parameter);
// static childClassFactory<dimeCommunicator, Communicator> dimeComm(std::vector<std::string>{"dime"});

static typeFactory<fmi::fmiCoordinator> coord("extra", std::vector<std::string>{"fmi", "fmicoord"});

void loadFMIExportObjects () {}

void loadFmiExportReaderInfoDefinitions (readerInfo &ri)
{
	ri.addTranslate("fmi", "extra");
	ri.addTranslate("fmicoord", "extra");
    ri.addTranslate ("fmiparam", "event");
    ri.addTranslate ("fmiinput", "event");
    ri.addTranslate ("fmioutput", "collector");
}

}//namespace griddyn