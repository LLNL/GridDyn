# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(griddynHELICS_sources
	helicsSource.cpp
	helicsCollector.cpp
	helicsGhostBus.cpp
	helicsLoad.cpp
	helicsLibrary.cpp
	helicsSupport.cpp
	helicsRunner.cpp
	helicsCoordinator.cpp
	)
	
set(griddynHELICS_headers
	helicsLibrary.h
	helicsSource.h
	helicsCollector.h
	helicsGhostBus.h
	helicsLoad.h
	helicsRunner.h
	helicsSupport.h
	helicsCoordinator.h
	)

link_directories(${external_link_directories})

add_library(helicsLib STATIC ${griddynHELICS_sources} ${griddynHELICS_headers})

set_target_properties (helicsLib PROPERTIES FOLDER libraries)

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)
INCLUDE_DIRECTORIES(${HELICS_INCLUDE_DIR})
message(status "helics include ${HELICS_INCLUDE_DIR}")
message(status "helics libraries ${HELICS_STATIC_LIBRARY}")

INCLUDE_DIRECTORIES(${ZeroMQ_INCLUDE_DIR})


FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test test_dir)

IF(WIN32)
#there are 4 slashes because cmake escapes "\" and regex escapes it so I need 4
# to get the regex to work and I need two for visual studio to actually print it.
# thus to it looks like this to replace '\' with '\\' 
#this will probably fail on macs yet

STRING(REGEX REPLACE "\\\\" "\\\\\\\\" test_dir ${test_dir})
ENDIF(WIN32)
#message(${test_dir})

ADD_DEFINITIONS(-DHELICS_TEST_DIRECTORY="${test_dir}" )
ADD_DEFINITIONS(-DHELICS_BROKER_EXECUTABLE="${HELICS_BROKER}")
ADD_DEFINITIONS(-DHELICS_PLAYER_EXECUTABLE="${HELICS_PLAYER}")
ADD_DEFINITIONS(-DHELICS_RECORDER_EXECUTABLE="${HELICS_RECORDER}")

target_link_libraries(helicsLib helics-static)

# this has to come before the zmq library in order
list(FIND griddyn_optional_libraries ${ZeroMQ_LIBRARY} ZMQ_INDEX)

list(INSERT griddyn_optional_libraries ${ZMQ_INDEX} helicsLib)
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/src/helics)
list(APPEND griddyn_optional_include_dirs ${HELICS_INCLUDE_DIR})

set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)


list (APPEND optional_library_key_headers helicsLibrary.h)
list (APPEND optional_library_functions loadHELICSLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)


list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/src/helics/test/helicsTest.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

INSTALL(TARGETS helicsLib DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  FILES_MATCHING PATTERN "*.h" COMPONENT headers)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  FILES_MATCHING PATTERN "*.hpp" COMPONENT headers)
