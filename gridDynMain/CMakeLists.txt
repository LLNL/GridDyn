# LLNS Copyright Start
# Copyright (c) 2016, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(griddynMain_sources
	gridDynMain.cpp
	)
	
set(griddynMain_headers
	)

link_directories(${SUNDIALS_LIBRARY_DIR})

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

set(external_library_list
	${SUNDIALS_LIBRARIES}
	${Boost_LIBRARIES}
	)
	

if(KLU_ENABLE)
 link_directories(${KLU_LIBRARY_DIR})
 list(APPEND external_library_list ${KLU_LIBRARIES})
endif(KLU_ENABLE)

IF (FMI_ENABLE)
 link_directories(${FMI_LIBRARY_DIRS})
 list(APPEND external_library_list ${FMI_LIBRARIES})

ENDIF(FMI_ENABLE)

IF(FSKIT_ENABLE)
list(APPEND external_library_list ${FSKIT_LIBRARIES})
ENDIF(FSKIT_ENABLE)

add_executable(griddynMain ${griddynMain_sources} ${griddynMain_headers})

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynFileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/coupling)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn/recorder_events)

set(GRIDYNMAIN_BINARY_LOC ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)


IF(OPTIMIZATION_ENABLE)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynOpt)
ENDIF(OPTIMIZATION_ENABLE)

IF (FMI_ENABLE)
 INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/fmi)
 INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
ENDIF(FMI_ENABLE)

IF(LOAD_EXTRA_MODELS)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/extraModels)
ENDIF(LOAD_EXTRA_MODELS)

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

set(gd_library_list
gridDynFileInput
gridDyn
coupling_static_lib
utilities
tinyxml
)
IF(OPTIMIZATION_ENABLE)
list(INSERT gd_library_list 1 gridDynOpt)
ENDIF(OPTIMIZATION_ENABLE)

IF(LOAD_EXTRA_MODELS)
list(INSERT gd_library_list 0 extraModelLibrary)
ENDIF(LOAD_EXTRA_MODELS)

IF(FMI_ENABLE)
list(INSERT gd_library_list 0 fmiGDLibrary)
ENDIF(FMI_ENABLE)



target_link_libraries(griddynMain ${gd_library_list} ${external_library_list} )

INSTALL(TARGETS griddynMain RUNTIME DESTINATION bin)
