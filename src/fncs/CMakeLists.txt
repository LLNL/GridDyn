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

IF(USE_DUMMY_FNCS)
	list(APPEND griddynFNCS_headers stub/fncs.hpp)
	list (APPEND griddynFNCS_sources stub/fncs.cpp)
ELSE(USE_DUMMY_FNCS)
	list(APPEND griddynFNCS_headers fncslib/fncs.hpp)
	list (APPEND griddynFNCS_sources fncslib/fncs.cpp)
	list(APPEND griddynFNCS_headers fncslib/log.hpp)
	list(APPEND griddynFNCS_headers fncslib/fncs_internal.hpp)
	list (APPEND griddynFNCS_headers fncslib/fncs.h)
ENDIF(USE_DUMMY_FNCS)

link_directories(${external_link_directories})

add_executable(griddynFNCS ${griddynFNCS_sources} ${griddynFNCS_headers})

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynFileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)

IF(USE_DUMMY_FNCS)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fncs/stub)
ADD_DEFINITIONS(-DUSE_DUMMY_FNCS )
ELSE (USE_DUMMY_FNCS)

IF (USE_FNCS_SHARED_LIBRARY)
INCLUDE_DIRECTORIES(${FNCS_INCLUDE_DIR})
ELSE (USE_FNCS_SHARED_LIBRARY)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fncs/fncslib)
INCLUDE_DIRECTORIES(${CZMQ_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZMQ_INCLUDE_DIR})
ENDIF(USE_FNCS_SHARED_LIBRARY)

ENDIF(USE_DUMMY_FNCS)

FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test test_dir)

IF(WIN32)
#there are 4 slashes because cmake escapes "\" and regex escapes it so I need 4
# to get the regex to work and I need two for visual studio to actually print it.
# thus to it looks like this to replace '\' with '\\' 
#this will probably fail on macs yet

STRING(REGEX REPLACE "\\\\" "\\\\\\\\" test_dir ${test_dir})
ENDIF(WIN32)
#message(${test_dir})

ADD_DEFINITIONS(-DFNCS_TEST_DIRECTORY="${test_dir}" )

target_link_libraries(griddynFNCS gridDynCombined)
IF(USE_DUMMY_FNCS)


ELSE(USE_DUMMY_FNCS)
IF (USE_FNCS_SHARED_LIBRARY)
target_link_libraries(griddynFNCS ${FNCS_LIBRARIES})
ELSE (USE_FNCS_SHARED_LIBRARY)
ADD_DEFINITIONS(-DLIBFNCS_STATIC)
ENDIF(USE_FNCS_SHARED_LIBRARY)

ENDIF(USE_DUMMY_FNCS)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET griddynFNCS POST_BUILD        # Adds a post-build event to griddynFNCS
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:griddynFNCS>)                 # <--this is out-file path
endforeach(keyfile)

INSTALL(TARGETS griddynFNCS RUNTIME DESTINATION bin)
