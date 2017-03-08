# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(griddynMain_sources
	gridDynMain.cpp
	)
	
set(griddynMain_headers
	)

link_directories(${external_link_directories})

add_executable(griddynMain ${griddynMain_sources} ${griddynMain_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynFileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynCombined)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

set(GRIDYNMAIN_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)


INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})



target_link_libraries(griddynMain ${executable_only_components} gridDynCombined)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET griddynMain POST_BUILD        # Adds a post-build event to griddynMain
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:griddynMain>)                 # <--this is out-file path
endforeach(keyfile)

INSTALL(TARGETS griddynMain RUNTIME DESTINATION bin)
