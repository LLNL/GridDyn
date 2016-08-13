# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name gridDynInout

set (reader_sources
	stringInterpret.cpp
	readerHelper.cpp
	gridDynFileInput.cpp
	gridDynRunner.cpp
	readerElement.cpp
	gridParameter.cpp
	readerInfo.cpp
	)

	set (xml_sources
	tinyxmlReaderElement.cpp
	tinyxmlReaderElement.h
	jsonReaderElement.cpp
	jsonReaderElement.h
	tinyxml2ReaderElement.cpp
	tinyxml2ReaderElement.h
	readXMLfile.cpp
	)

set(elementReader_sources
	readElementHelperFunctions.cpp
	readBusElement.cpp
	readEventElement.cpp
	readLinkElement.cpp
	readRecorderElement.cpp
	readEconElement.cpp
	readLibraryElement.cpp
	readAreaElement.cpp
	readArrayElement.cpp
	readConditionElement.cpp
	readRelayElement.cpp
	readSolverElement.cpp
	readSimulationElement.cpp
	loadSubObjectsElement.cpp
	readElementFile.cpp
	objectLoadFromElementHelperFunctions.cpp
	objectLoadHelperFunctions.cpp
	)

set(otherfileInput_sources
	gridDynReadCDF.cpp
	gridDynReadRAW.cpp
	gridDynReadDYR.cpp
	gridDynReadPSP.cpp
	gridDynReadPTI.cpp
	gridReadEPC.cpp
	gridReadMatPower.cpp
	gridReadPSAT.cpp
	readMatDyn.cpp
	gridDynReadCSV.cpp
	readMatlabData.cpp
	)
	
set(fileInput_headers
	readElement.h
	readerHelper.h
	gridDynFileInput.h
	gridDynRunner.h
	elementReaderTemplates.hpp
	readerElement.h
	readerInfo.h
	gridParameter.h
	readElementFile.h
	)


set (fileInput_sources
	${xml_sources}
	${reader_sources}
	${elementReader_sources}
	${otherfileInput_sources}
	${fileInput_headers}
	)
add_library(gridDynFileInput STATIC ${fileInput_sources})

source_group("XML"  FILES ${xml_sources})
source_group("Elements"  FILES  ${elementReader_sources})
source_group("OthersFormats"  FILES  ${otherfileInput_sources})
source_group("general" FILES ${reader_sources})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/coupling)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn/recorder_events)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynOpt)

IF(LOAD_EXTRA_MODELS)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/extraModels)
ENDIF(LOAD_EXTRA_MODELS)

IF (FMI_ENABLE)
 INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/fmi)
 INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
ENDIF(FMI_ENABLE)

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

INSTALL(TARGETS gridDynFileInput
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${fileInput_headers} DESTINATION include/griddyn)
