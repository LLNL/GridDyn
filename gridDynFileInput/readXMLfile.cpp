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

#include "readElement.h"
#include "readElementFile.h"
#include "gridDyn.h"
#include "readerHelper.h"
#include "tinyxmlReaderElement.h"
#include "tinyxml2ReaderElement.h"
#include "jsonReaderElement.h"
#include "elementReaderTemplates.hpp"

#include <boost/filesystem.hpp>
#include <sstream>
#include <cstdio>
#include <utility>

using namespace readerConfig;

std::unique_ptr<gridDynSimulation> readSimXMLFile (const std::string &filename, readerInfo *ri, xmlreader rtype)
{
	if (rtype == xmlreader::default_reader)
	{
		rtype = readerConfig::default_xml_reader;
	}
	switch (rtype)
	{
	case xmlreader::tinyxml:
	default:
		return std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(loadElementFile<tinyxmlReaderElement>(nullptr, filename, ri)));
	case xmlreader::tinyxml2:
		return std::unique_ptr<gridDynSimulation>(static_cast<gridDynSimulation *>(loadElementFile<tinyxml2ReaderElement>(nullptr, filename, ri)));
	}

  
}

