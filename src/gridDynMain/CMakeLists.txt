# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(gridDynMain_sources gridDynMain.cpp)

set(gridDynMain_headers)

add_executable(gridDynMain ${gridDynMain_sources} ${gridDynMain_headers})

set(GRIDYNMAIN_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

# TODO runner and fmiBuilder should be in a griddyn_exe library of some sort
target_link_libraries(gridDynMain runner griddyn_base griddyn_optional)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(
        TARGET gridDynMain
        POST_BUILD # Adds a post-build event to gridDynMain
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
            "${keyfile}" # <--this is in-file
            $<TARGET_FILE_DIR:gridDynMain>
    ) # <--this is out-file path
endforeach(keyfile)

install(TARGETS gridDynMain RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT RUNTIME)
