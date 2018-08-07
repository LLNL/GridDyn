# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(networking_sources networkingInterface.cpp)
set(networking_headers networkingInterface.h)

if((ZMQ_ENABLE) OR (ZMQ_NEEDED))
    set(
        zmqLibrary_sources
        zmqLibrary/zmqContextManager.cpp
        zmqLibrary/zmqHelper.cpp
        zmqLibrary/zmqReactor.cpp
        zmqLibrary/zmqProxyHub.cpp
        zmqLibrary/zmqSocketDescriptor.cpp
    )

    set(
        zmqLibrary_headers
        zmqLibrary/zmqContextManager.h
        zmqLibrary/zmqHelper.h
        zmqLibrary/zmqReactor.h
        zmqLibrary/zmqProxyHub.h
        zmqLibrary/zmqSocketDescriptor.h
    )

    set(zmqInterface_sources zmqCommunicator.cpp zmqInterface.cpp)

    set(zmqInterface_headers zmqCommunicator.h zmqInterface.h)

    list(APPEND networking_sources ${zmqLibrary_sources})
    list(APPEND networking_headers ${zmqLibrary_headers})
    list(APPEND networking_sources ${zmqInterface_sources})
    list(APPEND networking_headers ${zmqInterface_headers})

endif()

if(DIME_ENABLE)
    set(
        dimeInterface_sources
        dimeCollector.cpp
        dimeCommunicator.cpp
        dimeClientInterface.cpp
        dimeRunner.cpp
        dimeInterface.cpp
    )

    set(
        dimeInterface_headers
        dimeCollector.h
        dimeCommunicator.h
        dimeClientInterface.h
        dimeRunner.h
        dimeInterface.h
    )
    list(APPEND networking_sources ${dimeInterface_sources})
    list(APPEND networking_headers ${dimeInterface_headers})
endif()

if(TCP_ENABLE)
    set(
        tcpInterface_sources
        AsioServiceManager.cpp
        tcpCommunicator.cpp
        tcpInterface.cpp
        TcpHelperClasses.cpp
        tcpCollector.cpp
    )

    set(
        tcpInterface_headers
        AsioServiceManager.h
        tcpCommunicator.h
        tcpInterface.h
        TcpHelperClasses.h
        tcpCollector.h
    )
    list(APPEND networking_sources ${tcpInterface_sources})
    list(APPEND networking_headers ${tcpInterface_headers})
endif()

add_library(networking STATIC ${networking_sources} ${networking_headers})
set_target_properties(networking PROPERTIES FOLDER libraries)

install(
    TARGETS networking
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

if((ZMQ_ENABLE) OR (ZMQ_NEEDED))
    target_compile_definitions(networking PRIVATE -DZMQ_ENABLE)
    if(ZMQ_USE_STATIC_LIBRARY)
        set(ZMQ_DEPENDENCY libzmq-static)
        target_compile_definitions(networking PUBLIC -DZMQ_STATIC)
    else()
        set(ZMQ_DEPENDENCY libzmq)
    endif()

    message(STATUS "zmq lib ${ZMQ_DEPENDENCY}")
    target_link_libraries(networking PUBLIC griddyn ${ZMQ_DEPENDENCY})

    source_group("ZMQLibrary" FILES ${zmqLibrary_sources} ${zmqLibrary_headers})
    source_group(
        "zmqinterface"
        FILES
        ${zmqInterface_sources}
        ${zmqInterface_headers}
    )
endif()

if(DIME_ENABLE)
    source_group("dime" FILES ${dimeInterface_sources} ${dimeInterface_headers})
    target_compile_definitions(networking PRIVATE -DDIME_ENABLE)
endif()

if(TCP_ENABLE)
    source_group("tcp" FILES ${tcpInterface_sources} ${tcpInterface_headers})
    target_compile_definitions(networking PRIVATE -DTCP_ENABLE)
endif()

install(
    FILES ${networking_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/networking
    COMPONENT headers
)

list(APPEND optional_library_key_headers networking/networkingInterface.h)
list(APPEND optional_library_functions loadNetworkingLibrary)

set(optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set(optional_library_functions ${optional_library_functions} PARENT_SCOPE)

list(
    APPEND
        optional_component_test_files
        ${CMAKE_CURRENT_SOURCE_DIR}/test/zmqTest.cpp
)
set(optional_component_test_files ${optional_component_test_files} PARENT_SCOPE)
