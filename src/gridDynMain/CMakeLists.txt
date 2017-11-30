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
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)

set(GRIDYNMAIN_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

message(STATUS "execOnly:${executable_only_components}")
target_link_libraries(griddynMain ${executable_only_components} gridDynCombined)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET griddynMain POST_BUILD        # Adds a post-build event to griddynMain
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:griddynMain>)                 # <--this is out-file path
endforeach(keyfile)

INSTALL(TARGETS griddynMain RUNTIME DESTINATION bin)
