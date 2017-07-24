# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(plugin_sources
	gridDynPlugins.cpp
	)
	
set(plugin_headers
	gridDynPluginApi.h
	gridDynPlugins.h
	)

add_library(pluginLibrary ${plugin_sources} ${plugin_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)

INSTALL(TARGETS pluginLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${plugin_headers} DESTINATION include/plugins)

