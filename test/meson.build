component_tests = [
    'ConditionTests',
    # simulationTests is empty
    'testAdjustableTX',
    'testArea',
    'testBlocks',
    # testComponents is empty
    'testdcLinks',
    'testEvents',
    'testExciters',
    'testExtraModels',
    # don't care about testFMI
    'testGenerators',
    'testGenModels',
    'testGovernors',
    'testGridLab',
    'testLinks',
    'testLoads',
    'testRecorders',
    'testRelays',
    'testSource',
]

extra_tests = [
    # extraFMUtests contains paths only on Philip's computer
    'largeContingencyTests',
    'largeInputTests',
    'objectSizeReport',
    'performanceTests',
    # testExtra is empty
]

library_tests = [
    # libraryTests is empty
    'testCore',
    'testElementReaders',
    'testGridDynRunner',
    'testGridParameter',
    'testJsonReader',
    'testMatrixData',
    'testOperatingBounds',
    'testReaderInfo',
    'testValuePredictors',
    # testWorkQueue requires utilities/workQueue.h, which no longer exists
    'testXML',
    'testZipUtilities',
]

system_tests = [
    'faultTests',
    'testCloning',
    'testConstraints',
    'testContingency',
    'testDyn1',
    'testDyn2',
    'testGridDynRunner',
    'testInputs',
    'testMainExe',
    'testOutputs',
    'testpFlow',
    'testRoots',
    'testSolverModes',
    # testSystem is empty
    'validationTests',
]

shared_library_tests = [
    'basic_tests',
    # shared_library_tests is empty
]

folder_map = [
    [library_tests, 'libraryTests'],
    [system_tests, 'systemTests'],
    [component_tests, 'componentTests'],
    [extra_tests, 'extraTests'],
    [shared_library_tests, 'testSharedLibrary'],
]

foreach test_name_folder_name : folder_map
    test_names = test_name_folder_name[0]
    folder_name = test_name_folder_name[1]

    foreach test_name : test_names
        test_exe = executable(folder_name + '_' + test_name, [
                folder_name + '/' + test_name + '.cpp',
                'testHelperFunctions.cpp',
                'exeTestHelper.cpp',
            ],
            cpp_args : [
                '-DBOOST_TEST_DYN_LINK',
                '-DBOOST_TEST_MODULE=griddyn_tests',
                '-DGRIDDYN_TEST_DIRECTORY="' + join_paths(meson.current_source_dir(), 'test_files') + '"',
                '-DGRIDDYNMAIN_LOCATION="' + meson.current_build_dir() + '"',
                '-DGRIDDYNINSTALL_LOCATION="' + 'griddyn.exe' + '"',
            ],
            include_directories : griddyn_config_inc,
            dependencies : [
                griddyn_utility_dep,
                griddyn_core_dep,
                griddyn_dep,
                griddyn_file_input_dep,
                griddyn_format_interpreter_dep,
                boost_dep,
                cereal_dep,
                minizip_dep,
                griddyn_shared_dep,
                griddyn_coupling_dep,
                griddyn_loader_dep,
            ])
        test('test_' + folder_name + '_' + test_name, test_exe)
    endforeach
endforeach
