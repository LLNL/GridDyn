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

project(ticpp)
set(ticppfiles ticpp.cpp tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp)
set(ticpp_headers ticppapi.h ticpp.h ticpprc.h tinystr.h tinyxml.h)

include(CheckLatestCXXStandardOption)
IF (VERSION_OPTION)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
ELSE ()
set(CMAKE_CXX_STANDARD 14)
ENDIF ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_DEBUG_POSTFIX)
set(CMAKE_DEBUG_POSTFIX d)
endif()

ADD_DEFINITIONS(-DTIXML_USE_TICPP)
add_library(ticpp STATIC ${ticppfiles} ${ticpp_headers})

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-Wno-suggest-override)
ENDIF ()


INSTALL(TARGETS ticpp
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/ EXPORT ticpp-targets)

install (EXPORT ticpp-targets DESTINATION cmake/)

INSTALL(FILES ${ticpp_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ticpp)
 
 
