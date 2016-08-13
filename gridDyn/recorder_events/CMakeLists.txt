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


set(re_sources
	gridRecorder.cpp
	gridEvent.cpp
	gridGrabbers.cpp
	grabberInterpreter.cpp
	stateGrabbers.cpp
	gridRelay.cpp
	gridCondition.cpp
	)
	
set(re_headers
	gridCondition.h
	gridGrabbers.h
	gridRecorder.h
	gridEvent.h
	gridCommunicator.h
	stateGrabber.h
	gridRelay.h
	zonalRelay.h
	)	



add_library(recorder_lib STATIC ${re_sources} ${re_headers})
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynIO)

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})



