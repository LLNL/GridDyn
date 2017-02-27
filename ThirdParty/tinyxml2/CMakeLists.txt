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

project(tinyxml2)
set(tinyxml2files tinyxml2.cpp)
set(tinyxml2_headers tinyxml2.h)


SET(NO_CONFIG_GENERATION ON)
include(configGenerator)

IF (VERSION_OPTION)
	add_compile_options(${VERSION_OPTION})
ELSE ()
set(CMAKE_CXX_STANDARD 14)
ENDIF ()

add_library(tinyxml2 STATIC ${tinyxml2files} ${tinyxml2_headers})

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-Wno-suggest-override)
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")



INSTALL(TARGETS tinyxml2
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

INSTALL(FILES ${tinyxml2_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/tinyxml2)