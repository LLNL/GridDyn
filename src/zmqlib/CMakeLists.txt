# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End



set(zmqLibrary_sources
	zmqLibrary/zmqContextManager.cpp
	zmqLibrary/zmqHelper.cpp
	zmqLibrary/zmqReactor.cpp
	zmqLibrary/zmqProxyHub.cpp
	zmqLibrary/zmqSocketDescriptor.cpp
	)

set(zmqLibrary_headers
	zmqLibrary/zmqContextManager.h
	zmqLibrary/zmqHelper.h
	zmqLibrary/zmqReactor.h
	zmqLibrary/zmqProxyHub.h
	zmqLibrary/zmqSocketDescriptor.h
	)
	
set(zmqInterface_sources
	zmqCommunicator.cpp
	zmqInterface.cpp
	)


	
set(zmqInterface_headers
	zmqCommunicator.h
	zmqInterface.h
	)

add_library(zmqInterfaceLibrary ${zmqLibrary_sources} ${zmqLibrary_headers} ${zmqInterface_sources} ${zmqInterface_headers})
 

IF(DIME_ENABLE)
set(dimeInterface_sources
	dimeCollector.cpp
	dimeCommunicator.cpp
	dimeClientInterface.cpp
	dimeRunner.cpp
	dimeInterface.cpp
	)

set(dimeInterface_headers
	dimeCollector.h
	dimeCommunicator.h
	dimeClientInterface.h
	dimeRunner.h
	dimeInterface.h
	)

add_library(dimeLibrary ${dimeInterface_sources} ${dimeInterface_headers})
ENDIF(DIME_ENABLE)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${ZMQ_INCLUDE_DIR})


if (ZMQ_USE_STATIC_LIBRARY)
	set(ZMQ_DEPENDENCY ${ZMQ_STATIC_LIBRARY})
	ADD_DEFINITIONS(-DZMQ_STATIC)
else()
	set(ZMQ_DEPENDENCY ${ZMQ_LIBRARY})
endif()
	
target_link_libraries( ${ZMQ_LIBRARY} )

source_group("ZMQLibrary" FILES ${zmqLibrary_sources} ${zmqLibrary_headers})

INSTALL(TARGETS zmqInterfaceLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/griddyn
  FILES_MATCHING PATTERN "*.hpp")

 IF(DIME_ENABLE)
  list(APPEND griddyn_optional_libraries dimeLibrary zmqInterfaceLibrary)
 ELSE(DIME_ENABLE)
  list(APPEND griddyn_optional_libraries zmqInterfaceLibrary)
 ENDIF(DIME_ENABLE)

list(APPEND griddyn_optional_include_dirs ${ZMQ_INCLUDE_DIR})
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/src/zmqlib)


set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)

list (APPEND optional_library_key_headers zmqInterface.h)
list (APPEND optional_library_functions loadZMQLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)

list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/src/zmqlib/test/zmqTest.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

