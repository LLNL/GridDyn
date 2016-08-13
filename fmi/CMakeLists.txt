# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(fmiProcess_sources
	fmiProcess.cpp
	fmi_import1.cpp
	fmi_import2.cpp
	)

set(fmiGD_sources
	fmiGDinfo.cpp
	fmiSubModel.cpp
	fmiSubModel2.cpp
	fmiLoad.cpp
	fmiExciter.cpp
	)
	
set(fmiGD_headers
	fmiGDinfo.h
	fmiSubModel.h
	fmiLoad.h
	fmiExciter.h
	)	
set(fmiProcess_headers
	fmi_importGD.h
	)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

link_directories(${FMI_LIBRARY_DIRS})


add_executable(fmiProcess ${fmiProcess_sources} ${fmiProcess_headers})

add_library(fmiGDLibrary ${fmiGD_sources} ${fmiGD_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/core)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)

set(external_library_list
	${FMI_LIBRARIES}
	${Boost_LIBRARIES}
	)

target_link_libraries(fmiProcess fmiGDLibrary ${external_library_list} )

INSTALL(TARGETS fmiProcess RUNTIME DESTINATION bin)
