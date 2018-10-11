add_subdirectory(core)
add_subdirectory(coupling)
add_subdirectory(fileInput)
add_subdirectory(formatInterpreters)
add_subdirectory(griddyn)
add_subdirectory(gridDynMain)
add_subdirectory(runner)
add_subdirectory(utilities)

# ------------------------------------------------------------
# Enable support for Plugins
# ------------------------------------------------------------
if(ENABLE_PLUGINS)
    if(BOOST_VERSION_LEVEL==0)
        set(ENABLE_PLUGINS OFF)
        message(WARNING "Plugin support requires BOOST 1.61 or higher")
    else()
        add_subdirectory(plugins)
        set(DL_REQUIRED ON)
    endif()
endif(ENABLE_PLUGINS)

# ------------------------------------------------------------
# Enable compilation of extraModels?
# ------------------------------------------------------------
# If extra models are used enabled try to locate the libraries
# link against them.
if(ENABLE_EXTRA_MODELS)
    add_subdirectory(extraModels)
    target_link_libraries(griddyn_optional INTERFACE extraModelLibrary)
endif(ENABLE_EXTRA_MODELS)

# ------------------------------------------------------------
# Enable compilation of extraSolvers?
# ------------------------------------------------------------
# If extra solvers are used enabled try to locate the libraries
# link against them.
if(ENABLE_EXTRA_SOLVERS)
    add_subdirectory(extraSolvers)
    target_link_libraries(griddyn_optional INTERFACE extraSolverLibrary)
endif(ENABLE_EXTRA_SOLVERS)

# ------------------------------------------------------------
# Enable compilation of FMI?
# ------------------------------------------------------------
if(ENABLE_FMI)
    if(BOOST_VERSION_LEVEL==0)
        set(ENABLE_FMI OFF CACHE BOOL "Enable FMI Support" FORCE)
        message(WARNING "FMI support requires BOOST 1.61 or higher, disabling it")
    else()
        add_subdirectory(fmi)
        target_link_libraries(griddyn_optional INTERFACE fmiGDLibrary)
        set(DL_REQUIRED ON)
    endif()
endif(ENABLE_FMI)

# ------------------------------------------------------------
# Enable compilation of FMI_EXPORT?
# ------------------------------------------------------------
if(ENABLE_FMI_EXPORT)
    add_subdirectory(fmi_export)
    target_link_libraries(griddyn_optional INTERFACE fmiBuilder)
endif(ENABLE_FMI_EXPORT)

# ------------------------------------------------------------
# Enable compilation of FSKIT?
# ------------------------------------------------------------
if(ENABLE_FSKIT)
    # TODO this needs to do a find_package(fskit) of some sort
    add_subdirectory(fskit)
    target_link_libraries(griddyn_optional INTERFACE fskitLibrary)
endif(ENABLE_FSKIT)

# ------------------------------------------------------------
# Enable compilation of networking?
# ------------------------------------------------------------
if(ENABLE_NETWORKING_LIBRARY)
    add_subdirectory(networking)
    target_link_libraries(griddyn_optional INTERFACE networking)
endif()

# ------------------------------------------------------------
# Enable compilation of HELICS?
# ------------------------------------------------------------
if(ENABLE_HELICS_EXECUTABLE)
    include(addHELICS)
    add_subdirectory(helics)
    target_link_libraries(griddyn_optional INTERFACE helicsLib)
endif(ENABLE_HELICS_EXECUTABLE)

# ------------------------------------------------------------
# Enable compilation of optimization library?
# ------------------------------------------------------------
if(ENABLE_OPTIMIZATION_LIBRARY)
    add_subdirectory(optimization)
    target_link_libraries(griddyn_optional INTERFACE optimization)
endif(ENABLE_OPTIMIZATION_LIBRARY)

# ------------------------------------------------------------
# Enable compilation of the griddyn shared library?
# ------------------------------------------------------------
if(BUILD_SHARED_LIBS)
    add_subdirectory(griddyn_shared)
endif(BUILD_SHARED_LIBS)

# ------------------------------------------------------------
# Generate libraryLoader.cpp
#
# Do this last, it depends on parameters set in other folder CMakeLists
# ------------------------------------------------------------
add_subdirectory(gridDynLoader)
