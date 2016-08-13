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

set(XMLFILES ticpp.cpp tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp)
set(tinyxml2files tinyxml2/tinyxml2.cpp)
set(jsonfiles jsoncpp.cpp json/json.h json/json-forwards.h)

set(XML_HEADERS ticppapi.h ticpp.h ticpprc.h tinystr.h tinyxml.h)
set (tinyxml2headers tinyxml2/tinyxml2.h)

add_library(tinyxml STATIC ${XMLFILES} ${tinyxml2files} ${XML_HEADERS} ${tinyxml2headers} ${jsonfiles})

INSTALL(TARGETS tinyxml 
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
