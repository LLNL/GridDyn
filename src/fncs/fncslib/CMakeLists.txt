# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
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
    fncsLoad.cpp
    fncsLibrary.cpp
    fncsSupport.cpp
    test/fncsTest.cpp
)

set(griddynFNCS_headers
    fncsLibrary.h
    fncsSource.h
    fncsCollector.h
    fncsLoad.h
    fncsSupport.h
    test/fncsTest.h
)

if(USE_DUMMY_FNCS)
    list(APPEND griddynFNCS_headers stub/fncs.hpp)
    list(APPEND griddynFNCS_sources stub/fncs.cpp)
endif(USE_DUMMY_FNCS)

link_directories(${external_link_directories})

add_executable(griddynFNCS ${griddynFNCS_sources} ${griddynFNCS_headers})

include_directories(.)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/formatInterpreters)
include_directories(${PROJECT_SOURCE_DIR}/gridDyn)
include_directories(${PROJECT_SOURCE_DIR}/gridDynFileInput)
include_directories(${PROJECT_SOURCE_DIR}/utilities)
include_directories(${PROJECT_SOURCE_DIR}/coupling)
include_directories(${PROJECT_SOURCE_DIR}/gridDyn/recorder_events)
include_directories(${PROJECT_SOURCE_DIR}/gridDynCombined)

if(USE_DUMMY_FNCS)
    include_directories(${PROJECT_SOURCE_DIR}/fncs/stub)
    add_definitions(-DUSE_DUMMY_FNCS)
else(USE_DUMMY_FNCS)
    include_directories(${FNCS_INCLUDE_DIR})

endif(USE_DUMMY_FNCS)

include_directories(SYSTEM ${SUNDIALS_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test test_dir)

if(WIN32)
    # there are 4 slashes because cmake escapes "\" and regex escapes it so I need 4 to get the
    # regex to work and I need two for visual studio to actually print it. thus to it looks like
    # this to replace '\' with '\\' this will probably fail on macs yet

    string(REGEX REPLACE "\\\\" "\\\\\\\\" test_dir ${test_dir})
endif(WIN32)
# message(${test_dir})

add_definitions(-DFNCS_TEST_DIRECTORY="${test_dir}")

if(USE_DUMMY_FNCS)
    target_link_libraries(griddynFNCS gridDynCombined)
else(USE_DUMMY_FNCS)
    target_link_libraries(griddynFNCS gridDynCombined ${FNCS_LIBRARIES})
endif(USE_DUMMY_FNCS)

install(TARGETS griddynFNCS RUNTIME DESTINATION bin)
