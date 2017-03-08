# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name

set(utilities_sources
	gridRandom.cpp
	saturation.cpp
	stackInfo.cpp
	stringOps.cpp
	string_viewOps.cpp
	matrixOps.cpp
	stringConversion.cpp
	units.cpp
	vectorOps.cpp
	functionInterpreter.cpp
	charMapper.cpp
	workQueue.cpp
	zipUtilities.cpp
	base64.cpp
	matrixCreation.cpp
	)
	
set(utilities_headers
	units.h
	timeSeries.h
	charMapper.h
	timeSeriesMulti.h
	vectorOps.hpp
	generic_string_ops.hpp
	stringOps.h
	string_viewOps.h
	string_viewDef.h
	optionalDef.h
	gridRandom.h
	saturation.h
	stackInfo.h
	vectData.h
	matrixData.h
	matrixOps.h
	matrixDataCompact.h
	matrixDataSparse.h
	matrixDataBoost.h
	matrixDataSparseSM.h
	matrixDataTranslate.h
	matrixDataContainer.h
	matrixDataScale.h
	matrixDataOrdered.h
	matrixDataOrdering.h
	matrixDataFilter.h
	functionInterpreter.h
	workQueue.h
	simpleQueue.h
	zipUtilities.h
	base64.h
	stringConversion.h
	string_viewConversion.h
	mapOps.h
	valuePredictor.h
	matrixCreation.h
	timeRepresentation.h
	)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})

include(buildZlib)

include(buildMiniZip)
###########################################
# building zlib
###########################################
set(ZLIB_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

unset(ZLIB_FOUND)
find_package(ZLIB)

if (NOT ZLIB_FOUND)
build_zlib()
find_package(ZLIB)
endif (NOT ZLIB_FOUND)




###########################################
# building minizip
###########################################
set(MINIZIP_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

find_package(MiniZip)

if (NOT MINIZIP_FOUND)
build_minizip()
find_package(MiniZip)
endif (NOT MINIZIP_FOUND)

IF (ENABLE_64_BIT_INDEXING)
add_definitions(-DENABLE_64_BIT_INDEXING)
ENDIF(ENABLE_64_BIT_INDEXING)


list(APPEND external_library_list ${MINIZIP_LIBRARIES} ${ZLIB_LIBRARIES})

set (external_library_list ${external_library_list} PARENT_SCOPE)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include/)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

INSTALL(TARGETS utilities
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib)

INSTALL(FILES ${utilities_headers} DESTINATION include/utilities)
