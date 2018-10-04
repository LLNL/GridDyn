# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(extra_sources extraModels.cpp txThermalModel.cpp txLifeSpan.cpp)

set(extra_headers extraModels.h txThermalModel.h txLifeSpan.h)

add_library(extraModelLibrary STATIC ${extra_sources} ${extra_headers})
set_target_properties(extraModelLibrary PROPERTIES FOLDER libraries)

target_link_libraries(extraModelLibrary PUBLIC griddyn)
install(
    TARGETS extraModelLibrary
    EXPORT griddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES ${extra_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/extraModels
    COMPONENT headers
)

list(APPEND optional_library_key_headers extraModels/extraModels.h)
list(APPEND optional_library_functions loadExtraModels)

set(optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set(optional_library_functions ${optional_library_functions} PARENT_SCOPE)
