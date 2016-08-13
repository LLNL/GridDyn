# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(extra_sources
	extraModels.cpp
	txThermalModel.cpp
	txLifeSpan.cpp
	)
	
set(extra_headers
	extraModels.h
	txThermalModel.h
	txLifeSpan.h
	)

add_library(extraModelLibrary ${extra_sources} ${extra_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)

INSTALL(TARGETS extraModelLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${extra_headers} DESTINATION include/griddyn)

