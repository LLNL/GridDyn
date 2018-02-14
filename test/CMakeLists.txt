# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(testComponent_sources
    componentTests/ConditionTests.cpp
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
	componentTests/testEvents.cpp
	componentTests/testExtraModels.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
	)


	list(APPEND testComponent_sources ${optional_component_test_files})

set (testLibrary_sources
	libraryTests/testCore.cpp
	libraryTests/testXML.cpp
	libraryTests/testStringOps.cpp
	libraryTests/testString_viewOps.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
	libraryTests/libraryTests.cpp
	libraryTests/testGridDynRunner.cpp
	libraryTests/testElementReaders.cpp
	libraryTests/testGridParameter.cpp
	libraryTests/testJsonReader.cpp
	libraryTests/testReaderInfo.cpp
	libraryTests/testMatrixData.cpp
	libraryTests/testWorkQueue.cpp
	libraryTests/testZipUtilities.cpp
	libraryTests/testStringConversion.cpp
	libraryTests/testString_viewConversion.cpp
	libraryTests/simpleQueueTests.cpp
	libraryTests/testOperatingBounds.cpp
	libraryTests/testValuePredictors.cpp
	)

	list(APPEND testLibrary_sources ${optional_library_test_files})

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
	systemTests/testCloning.cpp
	systemTests/faultTests.cpp
	systemTests/testContingency.cpp
	systemTests/testGridDynRunner.cpp
	)

	list(APPEND testSystem_sources ${optional_system_test_files})

set(testExtra_sources
	extraTests/testExtra.cpp
	extraTests/performanceTests.cpp
	extraTests/largeContingencyTests.cpp
	extraTests/largeInputTests.cpp
	extraTests/objectSizeReport.cpp
	extraTests/extraFMUtests.cpp
	testHelperFunctions.cpp
	exeTestHelper.cpp
)	
	
set(testMain_headers
	testHelper.h
	exeTestHelper.h
	
	)

link_directories(${external_link_directories})

add_executable(testLibrary ${testLibrary_sources} ${testMain_headers})
add_executable(testComponents ${testComponent_sources} ${testMain_headers})
add_executable(testSystem ${testSystem_sources} ${testMain_headers})
add_executable(testExtra ${testExtra_sources} ${testMain_headers})

set_target_properties (testLibrary PROPERTIES FOLDER tests)
set_target_properties (testComponents PROPERTIES FOLDER tests)
set_target_properties (testSystem PROPERTIES FOLDER tests)
set_target_properties (testExtra PROPERTIES FOLDER tests)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/griddyn)
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/fileInput)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/gridDynCombined)

INCLUDE_DIRECTORIES(${griddyn_optional_include_dirs})

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})

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

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET testComponents POST_BUILD        # Adds a post-build event to testComponents
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        $<TARGET_FILE_DIR:testComponents>)                 # <--this is out-file path
endforeach(keyfile)

target_link_libraries(testLibrary gridDynCombined)
target_link_libraries(testComponents ${executable_only_components} gridDynCombined)
target_link_libraries(testSystem gridDynCombined)
target_link_libraries(testExtra gridDynCombined)

#message(${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
install(DIRECTORY DESTINATION bin/test/)
INSTALL(TARGETS testLibrary RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testComponents RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testSystem RUNTIME DESTINATION bin/test/)
INSTALL(TARGETS testExtra RUNTIME DESTINATION bin/test/)
INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin/test/)
