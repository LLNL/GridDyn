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

#include "formatInterpreters/readerElement.h"
#include "gridDynDefinitions.h"
#include "utilities/stringOps.h"
#include "core/coreObject.h"
#include "readerHelper.h"
#include "readElement.h"
#include "gridDynFileInput.h"
#include "readElement.h"
#include "core/coreExceptions.h"

static const stringVec indexAndNumber = { "index","number" };
static const std::string nameString = "name";

std::string getObjectName(std::shared_ptr<readerElement> &element, readerInfo &ri)
{
	std::string newName = getElementField(element, nameString, readerConfig::defMatchType);
	if (!newName.empty())
	{
		newName = ri.checkDefines(newName);
		if (!ri.prefix.empty())
		{
	
			newName=ri.prefix + '_' + newName;
		}
	}
	return newName;
}

void setIndex(std::shared_ptr<readerElement> &element, coreObject *mobj, readerInfo &ri)
{
	
	std::string Index = getElementFieldOptions(element, indexAndNumber, readerConfig::defMatchType);
	if (!Index.empty())
	{
		Index = ri.checkDefines(Index);
		double val = interpretString(Index, ri);
		mobj->locIndex = static_cast<int> (val);
	}
	// check if there is a purpose string which is used in some models
	std::string purp = getElementField(element, "purpose", readerConfig::defMatchType);
	if (!purp.empty())
	{
		purp = ri.checkDefines(purp);
		try
		{
			mobj->set("purpose", purp);
		}
		catch (unrecognizedParameter &)
		{
			mobj->set("description", purp);
		}
	}
}