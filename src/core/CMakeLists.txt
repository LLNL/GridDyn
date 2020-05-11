# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(coreObject_headers
    coreDefinitions.hpp
    coreExceptions.h
    coreObject.h
    coreObjectList.h
    coreObjectTemplates.hpp
    coreOwningPtr.hpp
    coreProperties.hpp
    factoryTemplates.hpp
    helperObject.h
    nullObject.h
    objectFactory.hpp
    objectFactoryTemplates.hpp
    objectInterpreter.h
    objectOperatorInterface.hpp
    propertyBuffer.h
)

set(coreObject_sources
    coreExceptions.cpp
    coreObject.cpp
    coreObjectList.cpp
    helperObject.cpp
    nullObject.cpp
    objectFactory.cpp
    objectInterpreter.cpp
    propertyBuffer.cpp
)

add_library(coreObjects STATIC ${coreObject_headers} ${coreObject_sources})
set_target_properties(coreObjects PROPERTIES FOLDER libraries)
target_link_libraries(coreObjects PUBLIC utilities)
target_link_libraries(coreObjects PUBLIC units-static)

target_include_directories(
    coreObjects
    # target_link_libraries(this, extra_includes::variant) when available
    PUBLIC # TODO stop leaking the variant include, make this private
           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
)
