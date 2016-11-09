# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project name

set(ticppfiles ticpp/ticpp.cpp ticpp/tinystr.cpp ticpp/tinyxml.cpp ticpp/tinyxmlerror.cpp ticpp/tinyxmlparser.cpp)
set(tinyxml2files tinyxml2/tinyxml2.cpp)
set(jsonfiles json/jsoncpp.cpp json/json.h json/json-forwards.h)

set(ticpp_headers ticpp/ticppapi.h ticpp/ticpp.h ticpp/ticpprc.h ticpp/tinystr.h ticpp/tinyxml.h)
set (tinyxml2headers tinyxml2/tinyxml2.h)

set (elementReader_files 
jsonReaderElement.cpp
jsonReaderElement.h
tinyxml2ReaderElement.cpp
tinyxml2ReaderElement.h
tinyxmlReaderElement.cpp
tinyxmlReaderElement.h
readerElement.h
readerElement.cpp
)

add_library(formatInterpreter STATIC ${ticppfiles} ${tinyxml2files} ${ticpp_headers} ${tinyxml2headers} ${jsonfiles} ${elementReader_files})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-Wno-suggest-override)
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

INSTALL(TARGETS formatInterpreter
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
