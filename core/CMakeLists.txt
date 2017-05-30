# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

cmake_minimum_required (VERSION 3.4)

set(coreObject_headers
    propertyBuffer.h
	coreDefinitions.h
	objectFactory.h
	objectFactoryTemplates.h
	factoryTemplates.h
	coreExceptions.h
	helperObject.h
	coreObject.h
	nullObject.h
	coreObjectList.h
	coreObjectTemplates.h
	objectOperatorInterface.h
	helperTemplates.h
	objectInterpreter.h
    )
    
set(coreObject_sources
	objectFactory.cpp
	coreObject.cpp
	nullObject.cpp
	coreObjectList.cpp
	coreExceptions.cpp
	helperObject.cpp
	propertyBuffer.cpp
	objectInterpreter.cpp
	)
	
	add_library(coreObjects STATIC ${coreObject_headers} ${coreObject_sources})
	
	INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
	INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
	INSTALL(TARGETS coreObjects
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/core
  FILES_MATCHING PATTERN "*.h")




