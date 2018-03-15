# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name

set (elementReader_files 
jsonReaderElement.cpp
jsonReaderElement.h
jsonElement.h
jsonElement.cpp
tinyxml2ReaderElement.cpp
tinyxml2ReaderElement.h
tinyxmlReaderElement.cpp
tinyxmlReaderElement.h
readerElement.h
readerElement.cpp
${PROJECT_SOURCE_DIR}/ThirdParty/json/jsoncpp.cpp
)



#SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config)

include(buildYAML)
include(buildTinyXML2)
include(buildTICPP)

###########################################
# building yaml
#########################################
OPTION(ENABLE_YAML "enable yaml file support in GridDyn" OFF)
if (ENABLE_YAML)
set(YAML_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

if (NOT EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/yaml-cpp-targets.cmake)
build_yaml()
endif()
include(${PROJECT_BINARY_DIR}/libs/cmake/yaml-cpp-targets.cmake)

endif(ENABLE_YAML)

if (YAML_FOUND)
list(APPEND elementReader_files 
yamlReaderElement.h
yamlReaderElement.cpp
yamlElement.h
yamlElement.cpp)

list(APPEND optional_library_test_files ${PROJECT_SOURCE_DIR}/src/formatInterpreters/test/testYamlReader.cpp)
set (optional_library_test_files ${optional_library_test_files} PARENT_SCOPE)

ENDIF(YAML_FOUND)

set(YAML_FOUND ${YAML_FOUND} PARENT_SCOPE)



set(ZLIB_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)



###########################################
# building ticpp
###########################################

set(TICPP_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

if (NOT EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/ticpp-targets.cmake)
build_ticpp()
endif()
include(${PROJECT_BINARY_DIR}/libs/cmake/ticpp-targets.cmake)

###########################################
# building tinyxml2
###########################################

set(TINYXML2_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

if (NOT EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/tinyxml2-targets.cmake)
build_tinyxml2()
endif()
include(${PROJECT_BINARY_DIR}/libs/cmake/tinyxml2-targets.cmake)


add_library(formatInterpreter STATIC ${elementReader_files})
target_link_libraries(formatInterpreter tinyxml2 ticpp)

set_target_properties (formatInterpreter PROPERTIES FOLDER libraries)

ADD_DEFINITIONS(-DTIXML_USE_TICPP)

IF (YAML_FOUND)
target_link_libraries(formatInterpreter yaml-cpp)

ENDIF(YAML_FOUND)

INSTALL(TARGETS formatInterpreter DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  FILES_MATCHING PATTERN "*.h" COMPONENT headers)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  FILES_MATCHING PATTERN "*.hpp" COMPONENT headers)

set(formatInterpreter_include_directories
  ${PROJECT_SOURCE_DIR}/src/formatInterpreters
 PARENT_SCOPE)
 
 
