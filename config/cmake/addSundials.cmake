
set(sundials_version v5.0.0)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
    include(FetchContent)

    fetchcontent_declare(
        sundials
        GIT_REPOSITORY https://github.com/LLNL/sundials.git
        GIT_TAG ${sundials_version}
    )

    fetchcontent_getproperties(sundials)

    if(NOT ${gbName}_POPULATED)
        # Fetch the content using previously declared details
        fetchcontent_populate(sundials)
        
        # this section to be removed at the next release of ZMQ for now we need to
        # download the file in master as the one in the release doesn't work
      #  file(RENAME ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
      #       ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old
      #  )
      #  file(
      #      DOWNLOAD
      #      https://raw.githubusercontent.com/zeromq/libzmq/master/builds/cmake/ZeroMQConfig.cmake.in
      #      ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
      #  )

    endif()

    hide_variable(FETCHCONTENT_SOURCE_DIR_SUNDIALS)
    hide_variable(FETCHCONTENT_UPDATES_DISCONNECTED_SUNDIALS)

else() # cmake <3.11

    # create the directory first
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

    include(GitUtils)
    git_clone(
        PROJECT_NAME
        sundials
        GIT_URL
        https://github.com/LLNL/sundials.git
        GIT_TAG
        ${sundials_version}
        DIRECTORY
        ${PROJECT_BINARY_DIR}/_deps
    )

    set(${gbName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${gbName}-build)
    
  #   if(NOT EXISTS ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old)
  #      file(RENAME ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
  #           ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old
  #      )
  #      file(
  #          DOWNLOAD
  #          https://raw.githubusercontent.com/zeromq/libzmq/master/builds/cmake/ZeroMQConfig.cmake.in
  #          ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
  #      )
    endif()

endif()

set(BUILD_CVODES OFF CACHE INTERNAL "")
set(BUILD_IDAS OFF CACHE INTERNAL "")
set(BUILD_IDA ON CACHE INTERNAL "")
set(BUILD_KINSOL ON CACHE INTERNAL "")

set(EXAMPLES_ENABLE_C OFF CACHE INTERNAL "")
set(EXAMPLES_ENABLE_CXX OFF CACHE INTERNAL "")
set(EXAMPLES_INSTALL OFF CACHE INTERNAL "")
set(SUNDIALS_INDEX_SIZE 32 CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")

if (GRIDDYN_ENABLE_OPENMP_SUNDIALS)
set(OPENMP_ENABLE ON CACHE INTERNAL "")
endif(GRIDDYN_ENABLE_OPENMP_SUNDIALS)

if (NOT GRIDDYN_ENABLE_KLU)
        set(KLU_ENABLE ON CACHE INTERNAL "")
        get_target_property(SuiteSparse_DIRECT_INCLUDE_DIR SuiteSparse::klu INTERFACE_INCLUDE_DIRECTORIES)
        find_path (SUNDIALS_KLU_INCLUDE_PATH klu.h
            HINTS ${SuiteSparse_DIRECT_INCLUDE_DIR}
            PATH_SUFFIXES suitesparse)
        set(KLU_INCLUDE_DIR "${SUNDIALS_KLU_INCLUDE_PATH}" CACHE INTERNAL "")

        get_target_property(_klu_configs SuiteSparse::klu IMPORTED_CONFIGURATIONS)
        if(_klu_configs)

            set(klu_config "RELEASE")
            if(NOT "RELEASE" IN_LIST _klu_configs)
                    list(GET _klu_configs 0 klu_config)
            endif()

            get_target_property(KLU_LIBRARY_FILE SuiteSparse::klu IMPORTED_LOCATION_${klu_config})
        else()
            get_target_property(KLU_LIBRARY_FILE SuiteSparse::klu IMPORTED_LOCATION)
        endif()
        get_filename_component(SUNDIALS_KLU_LIBRARY_DIR_NAME ${KLU_LIBRARY_FILE} DIRECTORY)
        set(KLU_LIBRARY_DIR ${SUNDIALS_KLU_LIBRARY_DIR_NAME} CACHE INTERNAL "")
    else()
        set(KLU_ENABLE OFF CACHE INTERNAL "")
    endif()

add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})

add_library(sundials_all INTERFACE)
target_include_directories(sundials_all INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/extern/sundials/include>)
target_include_directories(sundials_all INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/extern/sundials/include>)
add_library(SUNDIALS::SUNDIALS ALIAS sundials_all) 

set(SUNDIALS_LIBRARIES
	sundials_arkode_static
	sundials_cvode_static
	sundials_ida_static
	sundials_kinsol_static
	sundials_nvecserial_static
	sundials_sunlinsolband_static
	sundials_sunlinsoldense_static
	sundials_sunlinsolpcg_static
	sundials_sunlinsolspbcgs_static
	sundials_sunlinsolspfgmr_static
	sundials_sunlinsolspgmr_static
	sundials_sunlinsolsptfqmr_static
	sundials_sunmatrixband_static
	sundials_sunmatrixdense_static
	sundials_sunmatrixsparse_static
	sundials_sunnonlinsolfixedpoint_static
	sundials_sunnonlinsolnewton_static
	sundials_nvecmanyvector_static
)
set_target_properties ( ${SUNDIALS_LIBRARIES} sundials_generic_static_obj PROPERTIES FOLDER sundials)

target_link_libraries(sundials_all INTERFACE ${SUNDIALS_LIBRARIES})

if (TARGET sundials_nvecopenmp_static )
   set_target_properties ( sundials_nvecopenmp_static PROPERTIES FOLDER sundials)

   target_link_libraries(sundials_all INTERFACE sundials_nvecopenmp_static)
endif()

if (MSVC)
target_compile_options(sundials_cvode_static PRIVATE "/sdl-")
target_compile_options(sundials_cvode_static PRIVATE "/W3")
endif()
