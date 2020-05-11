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

set(griddynFNCS_sources
    gridDynFNCS.cpp
    fncsSource.cpp
    fncsCollector.cpp
    fncsGhostBus.cpp
    fncsLoad.cpp
    fncsLibrary.cpp
    fncsSupport.cpp
    test/fncsTest.cpp
)

set(griddynFNCS_headers
    fncsLibrary.h
    fncsSource.h
    fncsCollector.h
    fncsGhostBus.h
    fncsLoad.h
    fncsSupport.h
    test/fncsTest.h
)

if(USE_DUMMY_FNCS)
    list(APPEND griddynFNCS_headers stub/fncs.hpp)
    list(APPEND griddynFNCS_sources stub/fncs.cpp)
else(USE_DUMMY_FNCS)
    list(APPEND griddynFNCS_headers fncslib/fncs.hpp)
    list(APPEND griddynFNCS_sources fncslib/fncs.cpp)
    list(APPEND griddynFNCS_headers fncslib/log.hpp)
    list(APPEND griddynFNCS_headers fncslib/fncs_internal.hpp)
    list(APPEND griddynFNCS_headers fncslib/fncs.h)
endif(USE_DUMMY_FNCS)

link_directories(${external_link_directories})

add_executable(griddynFNCS ${griddynFNCS_sources} ${griddynFNCS_headers})
target_include_directories(griddynFNCS SYSTEM PRIVATE ${Boost_INCLUDE_DIR})

include_directories(.)

include_directories(${formatInterpreter_include_directories})
include_directories(${PROJECT_SOURCE_DIR}/src/griddyn)
include_directories(${PROJECT_SOURCE_DIR}/src/gridDynFileInput)
include_directories(${PROJECT_SOURCE_DIR}/src/utilities)
include_directories(${PROJECT_SOURCE_DIR}/src/gridDynCombined)

if(USE_DUMMY_FNCS)
    include_directories(${PROJECT_SOURCE_DIR}/src/fncs/stub)
    add_definitions(-DUSE_DUMMY_FNCS)
else(USE_DUMMY_FNCS)

    if(USE_FNCS_SHARED_LIBRARY)
        include_directories(${FNCS_INCLUDE_DIR})
    else(USE_FNCS_SHARED_LIBRARY)
        include_directories(${PROJECT_SOURCE_DIR}/src/fncs/fncslib)
        include_directories(${CZMQ_INCLUDE_DIR})
        include_directories(${ZMQ_INCLUDE_DIR})
    endif(USE_FNCS_SHARED_LIBRARY)

endif(USE_DUMMY_FNCS)

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test test_dir)

if(WIN32)
    # there are 4 slashes because cmake escapes "\" and regex escapes it so I need 4 to get the
    # regex to work and I need two for visual studio to actually print it. thus to it looks like
    # this to replace '\' with '\\' this will probably fail on macs yet

    string(REGEX REPLACE "\\\\" "\\\\\\\\" test_dir ${test_dir})
endif(WIN32)
# message(${test_dir})

add_definitions(-DFNCS_TEST_DIRECTORY="${test_dir}")

target_link_libraries(griddynFNCS gridDynCombined)
if(USE_DUMMY_FNCS)

else(USE_DUMMY_FNCS)
    if(USE_FNCS_SHARED_LIBRARY)
        target_link_libraries(griddynFNCS ${FNCS_LIBRARIES})
    else(USE_FNCS_SHARED_LIBRARY)
        add_definitions(-DLIBFNCS_STATIC)
    endif(USE_FNCS_SHARED_LIBRARY)

endif(USE_DUMMY_FNCS)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(
        TARGET griddynFNCS
        POST_BUILD # Adds a post-build event to griddynFNCS
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
            "${keyfile}" # <--this is in-file
            $<TARGET_FILE_DIR:griddynFNCS>
    ) # <--this is out-file path
endforeach(keyfile)

install(TARGETS griddynFNCS RUNTIME DESTINATION bin)
