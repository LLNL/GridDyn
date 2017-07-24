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
	logger.cpp
	matrixDataSparse.cpp
	OperatingBoundary.cpp
	stringToCmdLine.cpp
	)

set(utilities_headers
	units.h
	dataDictionary.hpp
	timeSeries.hpp
	charMapper.h
	timeSeriesMulti.hpp
	vectorOps.hpp
	generic_string_ops.hpp
	stringOps.h
	string_viewOps.h
	string_viewDef.h
	optionalDef.hpp
	gridRandom.h
	saturation.h
	stackInfo.h
	vectData.hpp
	matrixData.hpp
	matrixOps.h
	matrixDataCompact.hpp
	matrixDataSparse.hpp
	matrixDataSparse_impl.hpp
	matrixDataBoost.hpp
	matrixDataSparseSM.hpp
	matrixDataTranslate.hpp
	matrixDataContainer.hpp
	matrixDataScale.hpp
	matrixDataOrdered.hpp
	matrixDataOrdering.hpp
	matrixDataFilter.hpp
	matrixDataCustomWriteOnly.hpp
	functionInterpreter.h
	workQueue.h
	simpleQueue.hpp
	zipUtilities.h
	base64.h
	stringConversion.h
	string_viewConversion.h
	mapOps.hpp
	valuePredictor.hpp
	matrixCreation.h
	timeRepresentation.hpp
	indexTypes.hpp
	logger.h
	OperatingBoundary.h
	stringToCmdLine.h
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
set(NO_SYSTEM_MINIZIP 1)
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

INSTALL(TARGETS utilities
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")
