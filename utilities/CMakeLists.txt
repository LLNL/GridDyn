# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
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
	units.cpp
	vectorOps.cpp
	functionInterpreter.cpp
	charMapper.cpp
	workQueue.cpp
	zipUtilities.cpp
	base64.cpp
	)
	
set(utilities_headers
	units.h
	timeSeries.h
	charMappers.h
	timeSeriesMulti.h
	vectorOps.hpp
	stringOps.h
	gridRandom.h
	saturation.h
	stackInfo.h
	vectData.h
	matrixData.h
	matrixDataCompact.h
	matrixDataSparse.h
	matrixDataBoost.h
	matrixDataSparseSM.h
	matrixDataTranslate.h
	matrixDataScale.h
	matrixDataOrdered.h
	functionInterpreter.h
	workQueue.h
	zipUtilities.h
	base64.h
	stringConversion.h
	)

add_library(utilities STATIC ${utilities_sources} ${utilities_headers})

target_link_libraries(utilities minizip)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
IF (ENABLE_64_BIT_INDEXING)
add_definitions(-DENABLE_64_BIT_INDEXING)
ENDIF(ENABLE_64_BIT_INDEXING)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

INSTALL(TARGETS utilities
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib)

INSTALL(FILES ${utilities_headers} DESTINATION include/gridDyn)
