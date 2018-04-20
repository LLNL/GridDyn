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

set (elementReader_headers
jsonReaderElement.h
jsonElement.h
tinyxml2ReaderElement.h
tinyxmlReaderElement.h
readerElement.h
tomlElement.h
tomlReaderElement.h
)

set (elementReader_files
jsonReaderElement.cpp
jsonElement.cpp
tinyxml2ReaderElement.cpp
tinyxmlReaderElement.cpp
readerElement.cpp
tomlElement.cpp
tomlReaderElement.cpp
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
yamlReaderElement.cpp
yamlElement.cpp)

list(APPEND elementReader_headers
yamlReaderElement.h
yamlElement.h)

list(APPEND optional_library_test_files ${PROJECT_SOURCE_DIR}/src/formatInterpreters/test/testYamlReader.cpp)
set (optional_library_test_files ${optional_library_test_files} PARENT_SCOPE)

ENDIF(YAML_FOUND)

set(YAML_FOUND ${YAML_FOUND} PARENT_SCOPE)

set(ZLIB_ROOT_DIR ${PROJECT_BINARY_DIR}/libs)

###########################################
# building ticpp and tinyxml2
###########################################

file(GLOB ticppfiles ${PROJECT_SOURCE_DIR}/ThirdParty/ticpp/*.cpp)
file(GLOB ticpp_headers ${PROJECT_SOURCE_DIR}/ThirdParty/ticpp/*.h)

add_library(tinyxml STATIC ${ticppfiles} ${ticpp_headers}
	${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/tinyxml2.cpp
	${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2/tinyxml2.h)

target_link_libraries(tinyxml griddyn_base)
target_compile_definitions(tinyxml PUBLIC -DTIXML_USE_TICPP)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_compile_options(tinyxml PRIVATE -Wno-suggest-override)
ENDIF ()

set_target_properties (tinyxml PROPERTIES FOLDER libraries)

add_library(formatInterpreter STATIC ${elementReader_files} ${elementReader_headers} )
target_link_libraries(formatInterpreter PUBLIC tinyxml utilities)

set_target_properties (formatInterpreter PROPERTIES FOLDER libraries)



IF (YAML_FOUND)
target_link_libraries(formatInterpreter PUBLIC yaml-cpp)

ENDIF(YAML_FOUND)

INSTALL(TARGETS formatInterpreter tinyxml EXPORT griddyn-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${elementReader_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn
  COMPONENT headers)



