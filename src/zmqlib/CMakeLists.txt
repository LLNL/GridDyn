# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
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
set_target_properties (zmqInterfaceLibrary PROPERTIES FOLDER libraries)

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
target_link_libraries(dimeLibrary PUBLIC zmqInterfaceLibrary)
set_target_properties (dimeLibrary PROPERTIES FOLDER libraries)

INSTALL(TARGETS dimeLibrary EXPORT griddyn-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

ENDIF(DIME_ENABLE)

target_include_directories(zmqInterfaceLibrary PRIVATE ${ZeroMQ_INCLUDE_DIR})


if (ZMQ_USE_STATIC_LIBRARY)
	set(ZMQ_DEPENDENCY ${ZeroMQ_STATIC_LIBRARY})
	target_add_definitions(zmqInterfaceLibrary PUBLIC -DZMQ_STATIC)
else()
	set(ZMQ_DEPENDENCY ${ZeroMQ_LIBRARY})
endif()
	
	message(status "zmq lib ${ZMQ_DEPENDENCY}")
target_link_libraries(zmqInterfaceLibrary PUBLIC ${ZMQ_DEPENDENCY} )
target_link_libraries(zmqInterfaceLibrary PUBLIC griddyn)

source_group("ZMQLibrary" FILES ${zmqLibrary_sources} ${zmqLibrary_headers})

INSTALL(TARGETS zmqInterfaceLibrary EXPORT griddyn-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${zmqLibrary_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/zmqlib
   COMPONENT headers)

 IF(DIME_ENABLE)
  list(APPEND griddyn_optional_libraries dimeLibrary)
  INSTALL(FILES ${dimeInterface_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/zmqlib
   COMPONENT headers)
 ELSE(DIME_ENABLE)
  list(APPEND griddyn_optional_libraries zmqInterfaceLibrary)
 ENDIF(DIME_ENABLE)


list (APPEND optional_library_key_headers zmqlib/zmqInterface.h)
list (APPEND optional_library_functions loadZMQLibrary)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)

list(APPEND optional_component_test_files ${PROJECT_SOURCE_DIR}/src/zmqlib/test/zmqTest.cpp)
set (optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)

