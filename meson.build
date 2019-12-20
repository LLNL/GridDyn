project('griddyn', ['c', 'cpp'], version : '0.9.x', default_options: ['cpp_std=c++17'])

subdir('ThirdParty')

griddyn_conf = configuration_data({
        'GRIDDYN_DATE' : 'December 2019',
        'GRIDDYN_VERSION_BUILD' : 'meson',
        'GRIDDYN_VERSION_MAJOR' : '0',
        'GRIDDYN_VERSION_MINOR' : '9',
        'GRIDDYN_VERSION_PATCH' : '2',
        'GRIDDYN_VERSION_STRING' : '0.9.2-helics_updates_meson (dirty)',
        'GRIDDYN_VERSION_STRING_SHORT' : '0.9.2',

        'GRIDDYN_ENABLE_KLU' : get_option('GRIDDYN_ENABLE_KLU'),
        'GRIDDYN_ENABLE_LOGGING' : get_option('GRIDDYN_ENABLE_LOGGING'),
        'GRIDDYN_ENABLE_TRACE_LOGGING' : get_option('GRIDDYN_ENABLE_TRACE_LOGGING'),
        'GRIDDYN_ENABLE_DEBUG_LOGGING' : get_option('GRIDDYN_ENABLE_DEBUG_LOGGING'),
        'GRIDDYN_ENABLE_EXTRA_MODELS' : get_option('GRIDDYN_ENABLE_EXTRA_MODELS'),
        'GRIDDYN_ENABLE_FMI' : get_option('GRIDDYN_ENABLE_FMI'),
        'GRIDDYN_ENABLE_OPENMP' : get_option('GRIDDYN_ENABLE_OPENMP'),
        'HAVE_SHARED_TIMED_MUTEX' : get_option('HAVE_SHARED_TIMED_MUTEX'),
    })

subdir('griddyn')
griddyn_config_inc = include_directories('.')

boost_dep = dependency('boost', modules : [
        'system',
        'unit_test_framework',
        'filesystem',
    ])
cc = meson.get_compiler('c')
dl_lib = cc.find_library('dl')
fs_lib = cc.find_library('stdc++fs')
threads_dep = dependency('threads')

griddyn_core_inc = include_directories('src')
griddyn_utility_inc = include_directories('src')
griddyn_inc = include_directories('src/griddyn', 'src')

griddyn_utility_lib = library('griddyn_utility', [
        'src/utilities/gridRandom.cpp',
        'src/utilities/saturation.cpp',
        'src/utilities/ThreePhaseFunctions.cpp',
        'src/utilities/functionInterpreter.cpp',
        'src/utilities/zipUtilities.cpp',
        'src/utilities/matrixCreation.cpp',
        'src/utilities/matrixDataSparse.cpp',
        'src/utilities/matrixOps.cpp',
        'src/utilities/OperatingBoundary.cpp',
        'src/utilities/GlobalWorkQueue.cpp',
        'src/utilities/logger.cpp',
        'src/utilities/loggerCore.cpp',
    ],
    dependencies : [
        minizip_dep,
        boost_dep,
        fs_lib,
        gmlc_dep,
        threads_dep,
    ],
    include_directories : [ griddyn_config_inc, griddyn_utility_inc ],
    install : true)
griddyn_utility_dep = declare_dependency(link_with : griddyn_utility_lib, include_directories : griddyn_utility_inc)

griddyn_format_interpreter_inc = include_directories('src/formatInterpreters')
griddyn_format_interpreter_lib = library('griddyn_format_interpreter', [
        'src/formatInterpreters/jsonReaderElement.cpp',
        'src/formatInterpreters/jsonElement.cpp',
        'src/formatInterpreters/tinyxml2ReaderElement.cpp',
        'src/formatInterpreters/tinyxmlReaderElement.cpp',
        'src/formatInterpreters/readerElement.cpp',
        'src/formatInterpreters/tomlElement.cpp',
        'src/formatInterpreters/tomlReaderElement.cpp',
        'src/formatInterpreters/iniReaderElement.cpp',
        'src/formatInterpreters/yamlElement.cpp',
        'src/formatInterpreters/yamlReaderElement.cpp',
    ],
    dependencies : [
        griddyn_utility_dep,
        jsoncpp_dep,
        toml_dep,
        ticpp_dep,
        inih_dep,
        tinyxml2_dep,
        yaml_cpp_dep,
        gmlc_dep,
    ],
    include_directories : griddyn_format_interpreter_inc,
    install : true)
griddyn_format_interpreter_dep = declare_dependency(link_with : griddyn_format_interpreter_lib, include_directories : griddyn_format_interpreter_inc)

griddyn_core_lib = library('griddyn_core', [
        'src/core/coreExceptions.cpp',
        'src/core/coreObject.cpp',
        'src/core/coreObjectList.cpp',
        'src/core/helperObject.cpp',
        'src/core/nullObject.cpp',
        'src/core/objectFactory.cpp',
        'src/core/objectInterpreter.cpp',
        'src/core/propertyBuffer.cpp',
    ],
    include_directories : griddyn_core_inc,
    dependencies : [
        griddyn_utility_dep,
        gmlc_dep,
        units_dep,
    ],
    install : true)
griddyn_core_dep = declare_dependency(
    link_with : griddyn_core_lib,
    include_directories : griddyn_core_inc,
    dependencies : [
        griddyn_utility_dep,
        gmlc_dep,
        units_dep,
    ])

griddyn_coupling_lib = library('griddyn_coupling', [
        'src/coupling/GhostSwingBus.cpp',
        'src/coupling/GhostSwingBusManager.cpp',
    ],
    install : true)
griddyn_coupling_dep = declare_dependency(
    link_with : griddyn_coupling_lib)

griddyn_lib = library('griddyn', [
        'src/griddyn/loads/fileLoad.cpp',
        'src/griddyn/loads/motorLoad5.cpp',
        'src/griddyn/loads/approximatingLoad.cpp',
        'src/griddyn/loads/svd.cpp',
        'src/griddyn/loads/exponentialLoad.cpp',
        'src/griddyn/loads/motorLoad3.cpp',
        'src/griddyn/loads/fDepLoad.cpp',
        'src/griddyn/loads/frequencySensitiveLoad.cpp',
        'src/griddyn/loads/sourceLoad.cpp',
        'src/griddyn/loads/aggregateLoad.cpp',
        'src/griddyn/loads/Load.cpp',
        'src/griddyn/loads/rampLoad.cpp',
        'src/griddyn/loads/ThreePhaseLoad.cpp',
        'src/griddyn/loads/motorLoad.cpp',
        'src/griddyn/loads/zipLoad.cpp',
        'src/griddyn/loads/gridLabDLoad.cpp',
        'src/griddyn/blocks/functionBlock.cpp',
        'src/griddyn/blocks/rampLimiter.cpp',
        'src/griddyn/blocks/blockSequence.cpp',
        'src/griddyn/blocks/integralBlock.cpp',
        'src/griddyn/blocks/lutBlock.cpp',
        'src/griddyn/blocks/delayBlock.cpp',
        'src/griddyn/blocks/Block.cpp',
        'src/griddyn/blocks/derivativeBlock.cpp',
        'src/griddyn/blocks/pidBlock.cpp',
        'src/griddyn/blocks/transferFunctionBlock.cpp',
        'src/griddyn/blocks/valueLimiter.cpp',
        'src/griddyn/blocks/deadbandBlock.cpp',
        'src/griddyn/blocks/nullBlock.cpp',
        'src/griddyn/blocks/controlBlock.cpp',
        'src/griddyn/blocks/filteredDerivativeBlock.cpp',
        'src/griddyn/numericEstimationFunctions.cpp',
        'src/griddyn/stabilizers/Stabilizer.cpp',
        'src/griddyn/gridPrimary.cpp',
        'src/griddyn/measurement/collector.cpp',
        'src/griddyn/measurement/grabberInterpreter.cpp',
        'src/griddyn/measurement/Recorder.cpp',
        'src/griddyn/measurement/objectGrabbers.cpp',
        'src/griddyn/measurement/grabberSet.cpp',
        'src/griddyn/measurement/compoundCondition.cpp',
        'src/griddyn/measurement/Condition.cpp',
        'src/griddyn/measurement/stateGrabbers.cpp',
        'src/griddyn/measurement/gridGrabbers.cpp',
        'src/griddyn/genmodels/GenModel.cpp',
        'src/griddyn/genmodels/GenModelClassical.cpp',
        'src/griddyn/genmodels/GenModel5type2.cpp',
        'src/griddyn/genmodels/GenModel6type2.cpp',
        'src/griddyn/genmodels/GenModel5type3.cpp',
        'src/griddyn/genmodels/GenModel5.cpp',
        'src/griddyn/genmodels/GenModel3.cpp',
        'src/griddyn/genmodels/GenModelInverter.cpp',
        'src/griddyn/genmodels/GenModelGENROU.cpp',
        'src/griddyn/genmodels/GenModel6.cpp',
        'src/griddyn/genmodels/GenModel8.cpp',
        'src/griddyn/genmodels/GenModel4.cpp',
        'src/griddyn/gridComponent.cpp',
        'src/griddyn/simulation/gridSimulation.cpp',
        'src/griddyn/simulation/dynamicInitialConditionRecovery.cpp',
        'src/griddyn/simulation/gridDynContinuation.cpp',
        'src/griddyn/simulation/gridDynSimulationFileOps.cpp',
        'src/griddyn/simulation/powerFlowErrorRecovery.cpp',
        'src/griddyn/simulation/contingency.cpp',
        'src/griddyn/simulation/diagnostics.cpp',
        'src/griddyn/simulation/gridDynActions.cpp',
        'src/griddyn/simulation/gridDynPowerFlow.cpp',
        'src/griddyn/simulation/faultResetRecovery.cpp',
        'src/griddyn/simulation/gridDynContingency.cpp',
        'src/griddyn/simulation/gridDynSimulation.cpp',
        'src/griddyn/simulation/gridDynDynamic.cpp',
        'src/griddyn/gridSubModel.cpp',
        'src/griddyn/offsetTable.cpp',
        'src/griddyn/controllers/AGControl.cpp',
        'src/griddyn/controllers/dispatcher.cpp',
        'src/griddyn/controllers/schedulerReg.cpp',
        'src/griddyn/controllers/scheduler.cpp',
        'src/griddyn/controllers/schedulerRamp.cpp',
        'src/griddyn/controllers/reserveDispatcher.cpp',
        'src/griddyn/controllers/controlSystem.cpp',
        'src/griddyn/relays/differentialRelay.cpp',
        'src/griddyn/relays/breaker.cpp',
        'src/griddyn/relays/Relay.cpp',
        'src/griddyn/relays/fuse.cpp',
        'src/griddyn/relays/busRelay.cpp',
        'src/griddyn/relays/controlRelay.cpp',
        'src/griddyn/relays/sensor.cpp',
        'src/griddyn/relays/loadRelay.cpp',
        'src/griddyn/relays/zonalRelay.cpp',
        'src/griddyn/relays/pmu.cpp',
        'src/griddyn/links/dcLink.cpp',
        'src/griddyn/links/zBreaker.cpp',
        'src/griddyn/links/longLine.cpp',
        'src/griddyn/links/adjustableTransformer.cpp',
        'src/griddyn/links/subsystem.cpp',
        'src/griddyn/links/hvdc.cpp',
        'src/griddyn/links/Link.cpp',
        'src/griddyn/links/acLine.cpp',
        'src/griddyn/links/acdcConverter.cpp',
        'src/griddyn/exciters/ExciterDC1A.cpp',
        'src/griddyn/exciters/ExciterDC2A.cpp',
        'src/griddyn/exciters/ExciterIEEEtype2.cpp',
        'src/griddyn/exciters/Exciter.cpp',
        'src/griddyn/exciters/ExciterIEEEtype1.cpp',
        'src/griddyn/gridSecondary.cpp',
        'src/griddyn/primary/gridBus.cpp',
        'src/griddyn/primary/acBus.cpp',
        'src/griddyn/primary/Area.cpp',
        'src/griddyn/primary/infiniteBus.cpp',
        'src/griddyn/primary/dcBus.cpp',
        'src/griddyn/primary/DcBusControls.cpp',
        'src/griddyn/primary/listMaintainer.cpp',
        'src/griddyn/primary/BusControls.cpp',
        'src/griddyn/sources/functionSource.cpp',
        'src/griddyn/sources/Source.cpp',
        'src/griddyn/sources/pulseSource.cpp',
        'src/griddyn/sources/blockSource.cpp',
        'src/griddyn/sources/sineSource.cpp',
        'src/griddyn/sources/grabberSource.cpp',
        'src/griddyn/sources/rampSource.cpp',
        'src/griddyn/sources/randomSource.cpp',
        'src/griddyn/sources/fileSource.cpp',
        'src/griddyn/sources/commSource.cpp',
        'src/griddyn/generators/variableGenerator.cpp',
        'src/griddyn/generators/Generator.cpp',
        'src/griddyn/generators/DynamicGenerator.cpp',
        'src/griddyn/generators/isocController.cpp',
        'src/griddyn/comms/controlMessage.cpp',
        'src/griddyn/comms/commMessage.cpp',
        'src/griddyn/comms/communicationsCore.cpp',
        'src/griddyn/comms/commManager.cpp',
        'src/griddyn/comms/Communicator.cpp',
        'src/griddyn/comms/schedulerMessage.cpp',
        'src/griddyn/gridComponentHelperClasses.cpp',
        'src/griddyn/events/eventAdapters.cpp',
        'src/griddyn/events/interpolatingPlayer.cpp',
        'src/griddyn/events/Event.cpp',
        'src/griddyn/events/Player.cpp',
        'src/griddyn/events/reversibleEvent.cpp',
        'src/griddyn/events/eventQueue.cpp',
        'src/griddyn/events/compoundEvent.cpp',
        'src/griddyn/events/compoundEventPlayer.cpp',
        'src/griddyn/events/parameterOperator.cpp',
        'src/griddyn/solvers/sundialsMatrixData.cpp',
        'src/griddyn/solvers/basicSolver.cpp',
        'src/griddyn/solvers/cvodeInterface.cpp',
        'src/griddyn/solvers/solverInterface.cpp',
        'src/griddyn/solvers/sundialsMatrixDataSparseColumn.cpp',
        'src/griddyn/solvers/idaInterface.cpp',
        'src/griddyn/solvers/sundialsInterface.cpp',
        'src/griddyn/solvers/arkodeInterface.cpp',
        'src/griddyn/solvers/basicOdeSolver.cpp',
        'src/griddyn/solvers/sundialsMatrixDataDense.cpp',
        'src/griddyn/solvers/sundialsMatrixDataSparseRow.cpp',
        'src/griddyn/solvers/kinsolInterface.cpp',
        'src/griddyn/governors/Governor.cpp',
        'src/griddyn/governors/GovernorTgov1.cpp',
        'src/griddyn/governors/GovernorReheat.cpp',
        'src/griddyn/governors/GovernorSteamTCSR.cpp',
        'src/griddyn/governors/GovernorSteamNR.cpp',
        'src/griddyn/governors/GovernorIeeeSimple.cpp',
        'src/griddyn/governors/GovernorHydro.cpp',
    ],
    include_directories : griddyn_inc,
    dependencies : [
        griddyn_core_dep,
        griddyn_utility_dep,
        griddyn_coupling_dep,
        cereal_dep,
        ticpp_dep,
        sundials_dep,
        boost_dep,
        dl_lib,
        gmlc_dep,
        units_dep,
    ],
    install : true)
griddyn_dep = declare_dependency(
    link_with : griddyn_lib,
    include_directories : griddyn_inc,
    dependencies : [
        griddyn_core_dep,
        units_dep,
    ])

griddyn_file_input_inc = include_directories('src/fileInput')
griddyn_file_input_lib = library('griddyn_file_input',
    [
        'src/fileInput/fileInput.cpp',
        'src/fileInput/gridDynReadCDF.cpp',
        'src/fileInput/gridDynReadCSV.cpp',
        'src/fileInput/gridDynReadDYR.cpp',
        'src/fileInput/gridDynReadPSP.cpp',
        'src/fileInput/gridDynReadPTI.cpp',
        'src/fileInput/gridDynReadRAW.cpp',
        'src/fileInput/gridParameter.cpp',
        'src/fileInput/gridReadEPC.cpp',
        'src/fileInput/gridReadMatPower.cpp',
        'src/fileInput/gridReadPSAT.cpp',
        'src/fileInput/loadGDZ.cpp',
        'src/fileInput/loadSubObjectsElement.cpp',
        'src/fileInput/objectLoadFromElementHelperFunctions.cpp',
        'src/fileInput/objectLoadHelperFunctions.cpp',
        'src/fileInput/readAreaElement.cpp',
        'src/fileInput/readArrayElement.cpp',
        'src/fileInput/readBusElement.cpp',
        'src/fileInput/readCollectorElement.cpp',
        'src/fileInput/readConditionElement.cpp',
        'src/fileInput/readEconElement.cpp',
        'src/fileInput/readElementFile.cpp',
        'src/fileInput/readElementHelperFunctions.cpp',
        'src/fileInput/readerHelper.cpp',
        'src/fileInput/readerInfo.cpp',
        'src/fileInput/readEventElement.cpp',
        'src/fileInput/readLibraryElement.cpp',
        'src/fileInput/readLinkElement.cpp',
        'src/fileInput/readMatDyn.cpp',
        'src/fileInput/readMatlabData.cpp',
        'src/fileInput/readRelayElement.cpp',
        'src/fileInput/readSimulationElement.cpp',
        'src/fileInput/readSolverElement.cpp',
        'src/fileInput/readXMLfile.cpp',
        'src/fileInput/stringInterpret.cpp',
    ],
    include_directories : griddyn_file_input_inc,
    dependencies : [
        griddyn_utility_dep,
        griddyn_format_interpreter_dep,
        griddyn_core_dep,
        griddyn_dep,
        boost_dep,
        fs_lib,
        gmlc_dep,
        units_dep,
    ],
    install : true)
griddyn_file_input_dep = declare_dependency(
    link_with : griddyn_file_input_lib,
    include_directories : griddyn_file_input_inc,
    dependencies : units_dep)

griddyn_runner_lib = library('griddyn_runner', [
        'src/runner/gridDynRunner.cpp'
    ],
    dependencies : [
        griddyn_dep,
        cli11_dep,
        boost_dep,
        griddyn_file_input_dep,
        griddyn_coupling_dep,
        threads_dep,
    ],
    install : true)
griddyn_runner_dep = declare_dependency(link_with : griddyn_runner_lib)

griddyn_shared_inc = include_directories('src/griddyn_shared')
griddyn_shared_lib = library('griddyn_shared', [
        'src/griddyn_shared/griddyn_exportSimulation.cpp',
        'src/griddyn_shared/griddyn_exportObjects.cpp',
        'src/griddyn_shared/griddyn_exportMathFunctions.cpp',
    ],
    dependencies : [
        griddyn_core_dep,
        griddyn_dep,
        griddyn_runner_dep,
        griddyn_file_input_dep,
    ],
    install : true)
griddyn_shared_dep = declare_dependency(link_with : griddyn_shared_lib, include_directories : griddyn_shared_inc)

griddyn_loader_lib = library('griddyn_loader', [
        'src/gridDynLoader/libraryLoader.cpp'
    ],
    install : true)
griddyn_loader_dep = declare_dependency(link_with : griddyn_loader_lib)

griddyn_main = executable('griddyn.exe',
    'src/gridDynMain/gridDynMain.cpp',
    dependencies : [
        griddyn_dep,
        griddyn_runner_dep,
        griddyn_loader_dep,
    ],
    install : true)

subdir('headers')

pkg_config = import('pkgconfig')
pkg_config.generate(
    libraries : griddyn_lib,
    name : 'libgriddyn',
    filebase : 'griddyn',
    description : 'griddyn',
    subdirs : [
        '.',
        'griddyn'
    ])

subdir('test')
