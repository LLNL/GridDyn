# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
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
	gridParameter.cpp
	readerInfo.cpp
	)

set(elementReader_sources
	readElementHelperFunctions.cpp
	readBusElement.cpp
	readEventElement.cpp
	readLinkElement.cpp
	readCollectorElement.cpp
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

set(formattedfileInput_sources
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
	loadGDZ.cpp
	readXMLfile.cpp
	)
	
set(fileInput_headers
	readElement.h
	readerHelper.h
	gridDynFileInput.h
	gridDynRunner.h
	elementReaderTemplates.hpp
	readerInfo.h
	gridParameter.h
	readElementFile.h
	)


set (fileInput_sources
	${xml_sources}
	${reader_sources}
	${elementReader_sources}
	${formattedfileInput_sources}
	${fileInput_headers}
	)
add_library(gridDynFileInput STATIC ${fileInput_sources})

source_group("Elements"  FILES  ${elementReader_sources})
source_group("formatSpecific"  FILES  ${formattedfileInput_sources})
source_group("general" FILES ${reader_sources})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynOpt)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynCombined)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${griddyn_optional_include_dirs})

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

INSTALL(TARGETS gridDynFileInput
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${fileInput_headers} DESTINATION include/gridDyn)
