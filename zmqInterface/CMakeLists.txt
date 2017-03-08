# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End





set(zmqInterface_sources
	dimeCollector.cpp
	zmqCommunicator.cpp
	dimeCommunicator.cpp
	zmqInterface.cpp
	dimeClientInterface.cpp
	zmqContextManager.cpp
	zmqHelper.cpp
	zmqReactor.cpp
	zmqProxyHub.cpp
	zmqSocketDescriptor.cpp
	)
	
set(zmqInterface_headers
	dimeCollector.h
	zmqCommunicator.h
	dimeCommunicator.h
	zmqInterface.h
	dimeClientInterface.h
	zmqContextManager.h
	zmqHelper.h
	zmqReactor.h
	zmqProxyHub.h
	zmqSocketDescriptor.h
	)

add_library(zmqInterfaceLibrary ${zmqInterface_sources} ${zmqInterface_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${ZMQ_INCLUDE_DIR})
if(ZMQ_STATIC_LIB)
ADD_DEFINITIONS(-DZMQ_STATIC)
endif(ZMQ_STATIC_LIB)

target_link_libraries(zmqInterfaceLibrary ${ZMQ_LIBRARY} )

INSTALL(TARGETS zmqInterfaceLibrary
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${zmqInterface_headers} DESTINATION include/zmqInteface)

list(APPEND griddyn_optional_libraries zmqInterfaceLibrary)
list(APPEND griddyn_optional_include_dirs ${ZMQ_INCLUDE_DIR})
list(APPEND griddyn_optional_include_dirs ${PROJECT_SOURCE_DIR}/zmqInterface)


set(griddyn_optional_libraries ${griddyn_optional_libraries} PARENT_SCOPE)
set(griddyn_optional_include_dirs ${griddyn_optional_include_dirs} PARENT_SCOPE)

list (APPEND optional_library_key_headers zmqInterface.h)
list (APPEND optional_library_functions loadZMQLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)

list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/zmqInterface/test/zmqTest.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)
