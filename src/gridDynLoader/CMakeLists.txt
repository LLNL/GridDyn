# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

# build the file that loads the optional libraries
file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/libraryLoader.cpp)

file(
    WRITE
        ${PROJECT_BINARY_DIR}/libraryLoader.cpp
        "/*LLNS Copyright Start\n"
        " *Copyright (c) 2014-2018, Lawrence Livermore National Security\n"
        " *This work was performed under the auspices of the U.S. Department\n"
        " *of Energy by Lawrence Livermore National Laboratory in part under\n"
        " *Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.\n"
        " *Produced at the Lawrence Livermore National Laboratory.\n"
        " *All rights reserved.\n"
        " *For details, see the LICENSE file.\n"
        " *LLNS Copyright End\n"
        " */\n"
        "\n"
        "//automatically generated file\n"
        "\#include \"gridDynLoader/libraryLoader.h\"\n"
        "\#include \"griddyn/griddyn-config.h\"\n"
)

foreach(header ${optional_library_key_headers})
    file(
        APPEND
            ${PROJECT_BINARY_DIR}/libraryLoader.cpp "\n"
            "\#include \"${header}\"\n"
    )
endforeach(header)

file(
    APPEND
        ${PROJECT_BINARY_DIR}/libraryLoader.cpp
        "namespace griddyn {\n"
        "void loadLibraries()\n"
        "{\n"
)

foreach(ns ${optional_library_functions})
    file(
        APPEND
            ${PROJECT_BINARY_DIR}/libraryLoader.cpp
            "\n"
            "${ns}();\n"
            "\n"
    )
endforeach(ns)

file(
    APPEND
        ${PROJECT_BINARY_DIR}/libraryLoader.cpp
        "\n"
        "}\n"
        "}//namespace griddyn"
)

add_library(
    gridDynLoader
    STATIC
    libraryLoader.h
    ${PROJECT_BINARY_DIR}/libraryLoader.cpp
)

set_target_properties(gridDynLoader PROPERTIES FOLDER libraries)

# message("optlibs: ${griddyn_optional_libraries}")
# message("optheaders: ${griddyn_optional_include_dirs}")
# message("external_libraries: ${external_library_list}")
# message("external_lib_dirs: ${external_link_directories}")

# fileInput needs to come first since it can depend on some of the optional libraries which in turn depend on the gridDyn Libraries and the external libraries
target_link_libraries(gridDynLoader griddyn_optional)

if(DL_REQUIRED OR FMI_ENABLE)
    target_link_libraries(gridDynLoader ${CMAKE_DL_LIBS})
endif(DL_REQUIRED OR FMI_ENABLE)

install(
    TARGETS gridDynLoader
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES libraryLoader.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
    COMPONENT headers
)
