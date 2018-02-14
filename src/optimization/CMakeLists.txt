# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project  name


set(opt_sim_headers
optHelperClasses.h
	gridDynOpt.h
	gridOptObjects.h
	optObjectFactory.h
	optimizerInterface.h
	)
	
set(opt_sim_sources
   ${opt_sim_headers}
   optHelperClasses.cpp
	gridDynOpt.cpp	
	gridOptObjects.cpp
	optimizerInterface.cpp
	optObjectFactory.cpp
	)
	
set(opt_model_headers
	models/gridBusOpt.h
	models/gridLinkOpt.h
	models/gridGenOpt.h
	models/gridLoadOpt.h
	models/gridAreaOpt.h
	models/gridRelayOpt.h
	)

set(opt_model_sources
	${opt_model_headers}
	models/gridBusOpt.cpp
	models/gridLinkOpt.cpp
	models/gridGenOpt.cpp
	models/gridRelayOpt.cpp
	models/gridLoadOpt.cpp
	models/gridAreaOpt.cpp)

set(opt_files
	${opt_sim_sources}
	${opt_model_sources}
	)

set(header_files
	${opt_sim_headers}
	${opt_models_headers}
	)
	
add_library(optimization STATIC ${opt_files})

source_group("optimization" FILES ${opt_sim_sources})
source_group("models" FILES ${opt_model_sources})

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)

INSTALL(TARGETS optimization
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")

list(APPEND griddyn_optional_libraries optimization)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/src/optimization)

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)
