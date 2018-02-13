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
	coreDefinitions.hpp
	coreProperties.hpp
	objectFactory.hpp
	objectFactoryTemplates.hpp
	factoryTemplates.hpp
	coreExceptions.h
	coreOwningPtr.hpp 
	helperObject.h
	coreObject.h
	nullObject.h
	coreObjectList.h
	coreObjectTemplates.hpp
	objectOperatorInterface.hpp
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
	
	set_target_properties (coreObjects PROPERTIES FOLDER libraries)

	INSTALL(TARGETS coreObjects
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")

