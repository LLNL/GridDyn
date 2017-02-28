# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

#project  GriddDyn

set(genmodel_sources
	submodels/genModels/gridDynGenModel.cpp
	submodels/genModels/gridDynGenModelClassical.cpp
	submodels/genModels/gridDynGenModel3.cpp
	submodels/genModels/gridDynGenModel4.cpp
	submodels/genModels/gridDynGenModel5.cpp
	submodels/genModels/gridDynGenModel5type2.cpp
	submodels/genModels/gridDynGenModel5type3.cpp
	submodels/genModels/gridDynGenModel6.cpp	
	submodels/genModels/gridDynGenModel6type2.cpp	
	submodels/genModels/gridDynGenModelGENROU.cpp	
	submodels/genModels/gridDynGenModel8.cpp
	submodels/genModels/gridDynGenModelInverter.cpp
	submodels/gridDynGenModel.h
	submodels/otherGenModels.h
	)

set(generator_headers
	generators/gridDynGenerator.h
	generators/variableGenerator.h
	generators/isocController.h
	)
	
set (generator_sources
	${generator_headers}
	generators/gridDynGenerator.cpp
	generators/variableGenerator.cpp
	generators/isocController.cpp
	)

set(gridSource_sources
	sourceModels/gridSource.h
	sourceModels/otherSources.h
	sourceModels/sourceTypes.h
	sourceModels/gridSource.cpp
	sourceModels/rampSource.cpp
	sourceModels/fileSource.cpp
	sourceModels/randomSource.cpp
	sourceModels/pulseSource.cpp
	sourceModels/sineSource.cpp
	sourceModels/functionSource.cpp
	sourceModels/commSource.cpp
	sourceModels/blockSource.cpp
	sourceModels/grabberSource.cpp
	)

set(exciter_sources
	submodels/exciters/gridDynExciter.cpp
	submodels/exciters/gridDynExciterDC1A.cpp
	submodels/exciters/gridDynExciterDC2A.cpp
	submodels/exciters/gridDynExciterIEEEtype1.cpp
	submodels/exciters/gridDynExciterIEEEtype2.cpp
	submodels/pss/gridDynPSS.cpp
	submodels/gridDynExciter.h
	submodels/gridDynPSS.h
	)
	

	
set(gov_sources
	submodels/governors/gridDynGovernor.cpp
	submodels/governors/gridDynGovernorHydro.cpp
	submodels/governors/gridDynGovernorIeeeSimple.cpp
	submodels/governors/gridDynGovernorReheat.cpp
	submodels/governors/gridDynGovernorSteamNR.cpp
	submodels/governors/gridDynGovernorSteamTCSR.cpp
	submodels/governors/gridDynGovernorTgov1.cpp
	submodels/gridDynGovernor.h
	submodels/otherGovernors.h
	)

set (controlblock_sources
	submodels/controlBlocks/basicBlock.cpp
	submodels/controlBlocks/integralBlock.cpp
	submodels/controlBlocks/deadbandBlock.cpp
	submodels/controlBlocks/delayBlock.cpp
	submodels/controlBlocks/pidBlock.cpp
	submodels/controlBlocks/controlBlock.cpp
	submodels/controlBlocks/derivativeBlock.cpp
	submodels/controlBlocks/functionBlock.cpp
	submodels/controlBlocks/lutBlock.cpp
	submodels/controlBlocks/transferFunctionBlock.cpp
	submodels/controlBlocks/filteredDerivativeBlock.cpp
	submodels/controlBlocks/blockSequence.cpp
	submodels/controlBlocks/blockSequence.h
	submodels/controlBlocks/valueLimiter.cpp
	submodels/controlBlocks/valueLimiter.h
	submodels/controlBlocks/rampLimiter.cpp
	submodels/controlBlocks/rampLimiter.h
	submodels/otherBlocks.h
	submodels/gridControlBlocks.h
	)
	
set(submodel_sources
	${genmodel_sources}
	${exciter_sources}
	${gov_sources}
	${gridSource_sources}
	${controlblock_sources}
	)

set(submodel_headers
	submodels/gridDynExciter.h
	submodels/gridDynPSS.h
	sourceModels/gridSource.h
	submodels/gridDynGenModel.h
	submodels/otherGenModels.h
	submodels/gridDynGovernor.h
	submodels/gridControlBlocks.h
	)
	
set(load_headers
	loadModels/gridLoad.h
	loadModels/otherLoads.h
	loadModels/motorLoad.h
	loadModels/gridLabDLoad.h
	loadModels/svd.h
	loadModels/compositeLoad.h
	loadModels/zipLoad.h
	loadModels/gridLoad3Phase.h
	)
	
set(load_sources
	${load_headers}
	loadModels/gridLoad.cpp
	loadModels/rampLoad.cpp
	loadModels/fileLoad.cpp
	loadModels/motorLoad.cpp
	loadModels/motorLoad3.cpp
	loadModels/motorLoad5.cpp
	loadModels/gridLabDLoad.cpp
	loadModels/exponentialLoad.cpp
	loadModels/compositeLoad.cpp
	loadModels/sourceLoad.cpp
	loadModels/zipLoad.cpp
	loadModels/svd.cpp
	loadModels/gridLoad3Phase.cpp
	)

set (link_headers
	linkModels/dcLink.h
	linkModels/acdcConverter.h
	linkModels/gridLink.h
	linkModels/subsystem.h
	linkModels/hvdc.h
	linkModels/zBreaker.h
	linkModels/longLine.h
	linkModels/acLine.h
	)
	
set(link_sources
	${link_headers}
	linkModels/gridLink.cpp
	linkModels/adjustableTransformer.cpp
	linkModels/dcLink.cpp
	linkModels/acdcConverter.cpp
	linkModels/subsystem.cpp
	linkModels/hvdc.cpp
	linkModels/zBreaker.cpp
	linkModels/longLine.cpp
	linkModels/acLine.cpp
	)

set(simulation_headers
	simulation/contingency.h
	simulation/continuation.h
	gridDyn.h
	simulation/gridSimulation.h
	simulation/diagnostics.h
	simulation/powerFlowErrorRecovery.h
	simulation/dynamicInitialConditionRecovery.h
	simulation/faultResetRecovery.h
	simulation/gridDynActions.h
	simulation/gridDynSimulationFileOps.h
	)
	
set(simulation_sources
	${simulation_headers}
    simulation/gridDynContinuation.cpp
	simulation/gridDynSimulation.cpp	
	simulation/contingency.cpp
	simulation/gridDynPowerFlow.cpp
	simulation/gridDynDynamic.cpp
	simulation/gridSimulation.cpp
	simulation/gridDynActions.cpp
	simulation/gridDynSimulationFileOps.cpp
	simulation/diagnostics.cpp
	simulation/powerFlowErrorRecovery.cpp
	simulation/dynamicInitialConditionRecovery.cpp
	simulation/faultResetRecovery.cpp
	simulation/gridDynContingency.cpp
	
	)

set(solver_headers
	solvers/solverInterface.h
	solvers/sundialsInterface.h
	solvers/sundialsMatrixData.h
	solvers/solverMode.h
	)
	
set(solver_sources
	${solver_headers}
	solvers/idaInterface.cpp
	solvers/kinsolInterface.cpp
	solvers/solverInterface.cpp
	solvers/basicSolver.cpp
	solvers/sundialsMatrixDataDense.cpp
	solvers/sundialsMatrixDataSparseRow.cpp
	solvers/sundialsMatrixDataSparseColumn.cpp
	solvers/sundialsInterface.cpp
	solvers/basicOdeSolver.cpp
	)
	
IF (LOAD_CVODE)
	set(solver_sources 
	${solver_sources}
	solvers/cvodeInterface.cpp
	)
	
ENDIF(LOAD_CVODE)

IF (LOAD_ARKODE)
	set(solver_sources 
	${solver_sources}
	solvers/arkodeInterface.cpp
	)
	
ENDIF(LOAD_ARKODE)

set (core_headers
	gridObjects.h
	parameterOperator.hpp
	gridObjectsHelperClasses.h
	numericEstimationFunctions.h
	gridDynDefinitions.h
	)

set (core_sources
	${core_headers}
	gridObjects.cpp
	gridPrimary.cpp
	gridSecondary.cpp
	gridSubModel.cpp
	gridObjectsHelperClasses.cpp
	numericEstimationFunctions.cpp
	)

set(primary_headers
	gridBus.h
	gridArea.h
	primary/dcBus.h
	allGridDynObjects.h
	primary/busControls.h
	primary/listMaintainer.h
	primary/infiniteBus.h
	primary/acBus.h
	)
	
set(primary_sources
	${primary_headers}
	primary/gridBus.cpp
	primary/gridArea.cpp		
	primary/dcBus.cpp
	primary/infiniteBus.cpp
	primary/busControls.cpp
	primary/dcBusControls.cpp
	primary/acBus.cpp
	primary/listMaintainer.cpp
	)

set(event_headers
	events/gridEvent.h
	events/eventAdapters.h
	events/eventQueue.h
	events/eventInterface.h
	)
	
set(event_sources
	${event_headers}
	events/gridEvent.cpp
	events/eventAdapters.cpp
	events/eventQueue.cpp
	events/gridPlayer.cpp
	events/interpolatingPlayer.cpp
	events/compoundEvent.cpp
	events/compoundEventPlayer.cpp
	)	
set(measurement_headers
	measurement/gridGrabbers.h
	measurement/objectGrabbers.h
	measurement/collector.h
	measurement/stateGrabber.h
	measurement/gridCondition.h
	measurement/grabberSet.h
	)
	
set(measurement_sources
	${measurement_headers}
	measurement/gridRecorder.cpp
	measurement/gridGrabbers.cpp
	measurement/grabberInterpreter.cpp
	measurement/grabberInterpreter.hpp
	measurement/stateGrabbers.cpp
	measurement/gridCondition.cpp
	measurement/objectGrabbers.cpp
	measurement/compoundCondition.cpp
	measurement/collector.cpp
	measurement/grabberSet.cpp
	)	

set (comm_headers
	comms/commMessage.h
	comms/gridCommunicator.h
	comms/communicationsCore.h
	comms/relayMessage.h
	comms/schedulerMessage.h
	comms/controlMessage.h
	comms/commManager.h
	)
	
set (comm_sources
	${comm_headers}
	comms/commMessage.cpp
	comms/gridCommunicator.cpp
	comms/communicationsCore.cpp
	comms/relayMessage.cpp
	comms/schedulerMessage.cpp
	comms/controlMessage.cpp
	comms/commManager.cpp
	)
	
set(relay_headers
	relays/gridRelay.h
	relays/zonalRelay.h
	relays/loadRelay.h
	relays/busRelay.h
	relays/fuse.h
	relays/breaker.h
	relays/sensor.h
	relays/differentialRelay.h
	relays/controlRelay.h
	)
	
set (relay_sources
	${relay_headers}
	relays/gridRelay.cpp
	relays/zonalRelay.cpp
	relays/loadRelay.cpp
	relays/busRelay.cpp
	relays/fuse.cpp
	relays/breaker.cpp
	relays/sensor.cpp
	relays/differentialRelay.cpp
	relays/controlRelay.cpp
)


set (controller_headers
	controllers/schedulerInfo.h
	controllers/scheduler.h
	controllers/reserveDispatcher.h
	controllers/AGControl.h
	controllers/controlSystem.h
	controllers/dispatcher.h
	)
	
set (controller_sources
	${controller_headers}
	controllers/scheduler.cpp
	controllers/schedulerRamp.cpp
	controllers/schedulerReg.cpp
	controllers/reserveDispatcher.cpp
	controllers/controlSystem.cpp
	controllers/AGControl.cpp
	controllers/dispatcher.cpp
	)

set(sublibrary_files
	${event_sources}
	${measurement_sources}
	${griddynIO_sources}
	${gridSource_sources}
	${controller_sources}
	${comm_sources}
	${solver_sources}
	)

set(model_files
	${relay_sources}
	${submodel_sources}
	${load_sources}
	${link_sources}
	${generator_sources}
	)


set(header_files
	${core_headers}
	${primary_headers}
	${link_headers}
	${solver_headers}
	${controller_headers}
	${comm_headers}
	${event_headers}
	${measurement_headers}
	${gridDynIO_headers}
	${simulation_headers}
	${submodel_headers}
	${load_headers}
	${relay_headers}
	${generator_headers}
	)


add_library(gridDyn STATIC ${simulation_sources} ${model_files} ${core_sources} ${primary_sources} ${sublibrary_files})

source_group("loads" FILES ${load_sources})
source_group("exciters" FILES ${exciter_sources})
source_group("governors" FILES ${gov_sources})
source_group("genmodels" FILES ${genmodel_sources})
source_group("links" FILES ${link_sources})
source_group("events" FILES ${event_sources})
source_group("measurement" FILES ${measurement_sources})
source_group("generators" FILES ${generator_sources})
source_group("gridSources" FILES ${gridSource_sources})
source_group("simulation" FILES ${simulation_sources})
source_group("core" FILES ${core_sources})
source_group("primary" FILES ${primary_sources})
source_group("relays" FILES ${relay_sources})
source_group("comms" FILES ${comm_sources})
source_group("controllers" FILES ${controller_sources})
source_group("controlblocks" FILES ${controlblock_sources})
source_group("solvers" FILES ${solver_sources})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/gridDyn)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utilities)
INCLUDE_DIRECTORIES(${formatInterpreter_include_directories})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

INCLUDE_DIRECTORIES(SYSTEM ${SUNDIALS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})

#create a griddyn_config.h file for program options

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libs/include)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/griddyn-config.h.in ${PROJECT_BINARY_DIR}/libs/include/griddyn-config.h)

INSTALL(TARGETS gridDyn
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)

INSTALL(FILES ${PROJECT_BINARY_DIR}/griddyn-config.h DESTINATION include/gridDyn)
#INSTALL(FILES ${header_files} DESTINATION include/gridDyn)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include/gridDyn
  FILES_MATCHING PATTERN "*.h")

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION include
  FILES_MATCHING PATTERN "*.h")
