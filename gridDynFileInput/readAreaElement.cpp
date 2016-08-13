/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#include "readElement.h"
#include "readerHelper.h"
#include "elementReaderTemplates.hpp"
#include "readerElement.h"
#include "gridArea.h"
#include <cstdio>

using namespace readerConfig;

static const IgnoreListType areaIgnoreElements{ "agc", "reserve", "reservedispatch", "dispatch" };
static const std::string areaComponentName("area");
// "aP" is the XML element passed from the reader
gridArea *readAreaElement(std::shared_ptr<readerElement> &element, readerInfo *ri, gridCoreObject *searchObject)
{

	auto riScope = ri->newScope();

	//boiler plate code to setup the object from references or new object
	gridArea *area = ElementReaderSetup(element, (gridArea *)nullptr, areaComponentName, ri, searchObject);

	loadElementInformation(area, element, areaComponentName, ri, areaIgnoreElements);
	
	LEVELPRINT(READER_NORMAL_PRINT, "loaded Area " << area->getName());

	ri->closeScope(riScope);
	return area;
}