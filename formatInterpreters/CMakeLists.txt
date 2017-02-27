# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
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
)



#SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/config)

include(buildYAML)
include(buildTinyXML2)
include(buildJsonCPP)
include(buildTICPP)

###########################################
# building yaml
#########################################
OPTION(ENABLE_YAML "enable yaml file support in GridDyn" OFF)
if (ENABLE_YAML)
set(YAML_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

find_package(Yaml)

if (NOT YAML_FOUND)
build_yaml()
find_package(Yaml)
SHOW_VARIABLE(YAML_ROOT_DIR PATH "location of the yaml libraries" "${PROJECT_BINARY_DIR}/ThirdParty/yaml-cpp")
endif (NOT YAML_FOUND)
endif(ENABLE_YAML)

if (YAML_FOUND)
list(APPEND elementReader_files 
yamlReaderElement.h
yamlReaderElement.cpp
yamlElement.h
yamlElement.cpp)

list(APPEND optional_library_test_files ${PROJECT_SOURCE_DIR}/formatInterpreters/test/testYamlReader.cpp)
set (optional_library_test_files ${optional_library_test_files} PARENT_SCOPE)

ENDIF(YAML_FOUND)

set(YAML_FOUND ${YAML_FOUND} PARENT_SCOPE)
###########################################
# building jsoncpp
###########################################
set(JSONCPP_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

find_package(JsonCPP)

if (NOT JSONCPP_FOUND)
build_jsoncpp()
find_package(JsonCPP)
endif (NOT JSONCPP_FOUND)

###########################################
# building ticpp
###########################################

set(TICPP_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

find_package(TICPP)

if (NOT TICPP_FOUND)
build_ticpp()
find_package(TICPP)
endif (NOT TICPP_FOUND)

###########################################
# building tinyxml2
###########################################

set(TINYXML2_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

find_package(TinyXML2)

if (NOT TINYXML2_FOUND)
build_tinyxml2()
find_package(TinyXML2)
endif (NOT TINYXML2_FOUND)


add_library(formatInterpreter STATIC ${elementReader_files})

ADD_DEFINITIONS(-DTIXML_USE_TICPP)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)


IF (YAML_FOUND)
list(APPEND external_library_list ${YAML_LIBRARIES})
ENDIF(YAML_FOUND)

list(APPEND external_library_list ${JSONCPP_LIBRARIES} ${TICPP_LIBRARIES} ${TINYXML2_LIBRARIES})

set (external_library_list ${external_library_list} PARENT_SCOPE)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include/)


INSTALL(TARGETS formatInterpreter
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)


set(formatInterpreter_include_directories
  ${PROJECT_SOURCE_DIR}/formatInterpreters
  ${PROJECT_BINARY_DIR}/libs/include/
 PARENT_SCOPE)
 
 
