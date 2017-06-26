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

#include "config.h"

#include "helicsLibrary.h"
#include "helicsCollector.h"
#include "helicsCoordinator.h"
#include "helicsLoad.h"
#include "helicsSource.h"
#include "core/factoryTemplates.hpp"
#include "core/objectFactoryTemplates.hpp"

#include "fileInput/readerInfo.h"
//#include "helics.hpp"

namespace griddyn
{
namespace helicsLib
{
static childClassFactory<helicsCollector, collector> helicsFac(std::vector<std::string> {"helics"});

static childTypeFactory<helicsSource, sources::rampSource> fnsrc("source", stringVec{ "helics" });
static childTypeFactory<helicsLoad, loads::rampLoad> fnld("load", "helics");

//the factory for the coordinator
static typeFactory<helicsCoordinator> cbuild("extra", "helics");
} //namespace helicsLib

void loadHELICSLibrary()
{
	static int loaded = 0;

	if (loaded == 0)
	{
		loaded = 1;
	}
}

void loadHelicsReaderInfoDefinitions(readerInfo &ri)
{
	ri.addTranslate("helics", "extra");
}

} // namespace griddyn