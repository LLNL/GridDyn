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
set(testComponent_sources
	componentTests/testComponents.cpp
	componentTests/testLoads.cpp
	componentTests/testRecorders.cpp
	componentTests/testExciters.cpp
	componentTests/testGovernors.cpp
	componentTests/testAdjustableTX.cpp
	componentTests/testGenModels.cpp
	componentTests/testdcLinks.cpp
	componentTests/testGridLab.cpp
	componentTests/testLinks.cpp
	componentTests/testRelays.cpp
	componentTests/testBlocks.cpp
	componentTests/testGenerators.cpp
	componentTests/testArea.cpp
	componentTests/testSource.cpp
	componentTests/simulationTests.cpp
	componentTests/faultTests.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
	)

set (testLibrary_sources
	libraryTests/testCore.cpp
	libraryTests/testXML.cpp
	libraryTests/testStringOps.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
	libraryTests/libraryTests.cpp
	libraryTests/testGridDynRunner.cpp
	libraryTests/testElementReaders.cpp
	libraryTests/testGridParameter.cpp
	libraryTests/testJsonReader.cpp
	libraryTests/testReaderInfo.cpp
	libraryTests/testArrayData.cpp
	)

set(testSystem_sources
	systemTests/testSystem.cpp
	systemTests/testpFlow.cpp
	systemTests/testDyn1.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
	systemTests/testDyn2.cpp
	systemTests/testInputs.cpp
	systemTests/testRoots.cpp
	systemTests/testConstraints.cpp
	systemTests/validationTests.cpp
	systemTests/testSolverModes.cpp
	systemTests/testMainExe.cpp
	systemTests/testOutputs.cpp
	)

set(testExtra_sources
extraTests/testExtra.cpp
extraTests/performanceTests.cpp
testHelperFunctions.cpp
exeTestHelper.cpp
)	
	
set(testMain_headers
	testHelper.h
	exeTestHelper.h
	
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
 list(APPEND testSystem systemTests/testFMI.cpp)
ENDIF(FMI_ENABLE)

IF(FSKIT_ENABLE)
list(APPEND external_library_list ${FSKIT_LIBRARIES})
ENDIF(FSKIT_ENABLE)

add_executable(testLibrary ${testLibrary_sources} ${testMain_headers})
add_executable(testComponents ${testComponent_sources} ${testMain_headers})
add_executable(testSystem ${testSystem_sources} ${testMain_headers})
add_executable(testExtra ${testExtra_sources} ${testMain_headers})
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynFileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn/recorder_events)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/coupling)

IF(OPTIMIZATION_ENABLE)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDynOpt)
ENDIF(OPTIMIZATION_ENABLE)

IF (FMI_ENABLE)
 INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/fmi)
 INCLUDE_DIRECTORIES(${FMI_INCLUDE_DIR})
ENDIF(FMI_ENABLE)

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test_files test_dir)

IF(WIN32)
#there are 4 slashes because cmake escapes "\" and regex escapes it so I need 4
#this will probably fail on macs yet

STRING(REGEX REPLACE "\\\\" "\\\\\\\\" test_dir ${test_dir})
ENDIF(WIN32)
#message(${test_dir})

ADD_DEFINITIONS(-DGRIDDYN_TEST_DIRECTORY="${test_dir}" )

ADD_DEFINITIONS(-DGRIDDYNMAIN_LOCATION="${GRIDYNMAIN_BINARY_LOC}")
ADD_DEFINITIONS(-DGRIDDYNSERVER_LOCATION="${GRIDYNSERVER_BINARY_LOC}")
ADD_DEFINITIONS(-DGRIDDYNINSTALL_LOCATION="${CMAKE_INSTALL_PREFIX}")

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

target_link_libraries(testLibrary ${gd_library_list} ${external_library_list})
target_link_libraries(testComponents ${gd_library_list} ${external_library_list})
target_link_libraries(testSystem ${gd_library_list} ${external_library_list})
target_link_libraries(testExtra ${gd_library_list} ${external_library_list})

#message(${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
install(DIRECTORY DESTINATION bin/test/)
INSTALL(TARGETS testLibrary RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testComponents RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testSystem RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testExtra RUNTIME DESTINATION bin/test/)
