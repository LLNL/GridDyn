
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
      #  file(RENAME ${sundials_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
      #       ${sundials_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old
      #  )
      #  file(
      #      DOWNLOAD
      #      https://raw.githubusercontent.com/zeromq/libzmq/master/builds/cmake/ZeroMQConfig.cmake.in
      #      ${sundials_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in
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

endif()

if(NOT EXISTS ${sundials_SOURCE_DIR}/config/SundialsKLU.old)
     file(RENAME ${sundials_SOURCE_DIR}/config/SundialsKLU.cmake
            ${sundials_SOURCE_DIR}/config/SundialsKLU.old
       )
file(COPY ${PROJECT_SOURCE_DIR}/config/cmake/SundialsKLU.cmake
              DESTINATION
             ${sundials_SOURCE_DIR}/config
             )
 endif()

option(${PROJECT_NAME}_ENABLE_IDA ON "Enable IDA for use in the computation")
option(${PROJECT_NAME}_ENABLE_CVODE ON "Enable Cvode for use in the computation")
option(${PROJECT_NAME}_ENABLE_ARKODE ON "Enable arkode for use in the computation")
option(${PROJECT_NAME}_ENABLE_KINSOL ON "Enable kinsol for use in the computation")

set(BUILD_CVODES OFF CACHE INTERNAL "")
set(BUILD_IDAS OFF CACHE INTERNAL "")

if (${PROJECT_NAME}_ENABLE_IDA)
set(BUILD_IDA ON CACHE INTERNAL "")
else ()
set(BUILD_IDA OFF CACHE INTERNAL "")
endif()

if (${PROJECT_NAME}_ENABLE_KINSOL)
set(BUILD_KINSOL ON CACHE INTERNAL "")
else ()
set(BUILD_KINSOL OFF CACHE INTERNAL "")
endif()

if (${PROJECT_NAME}_ENABLE_CVODE)
set(BUILD_CVODE ON CACHE INTERNAL "")
else ()
set(BUILD_CVODE OFF CACHE INTERNAL "")
endif()

if (${PROJECT_NAME}_ENABLE_ARKODE)
set(BUILD_ARKODE ON CACHE INTERNAL "")
else ()
set(BUILD_ARKODE OFF CACHE INTERNAL "")
endif()

set(EXAMPLES_ENABLE_C OFF CACHE INTERNAL "")
set(EXAMPLES_ENABLE_CXX OFF CACHE INTERNAL "")
set(EXAMPLES_INSTALL OFF CACHE INTERNAL "")
set(SUNDIALS_INDEX_SIZE 32 CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")

if (${PROJECT_NAME}_ENABLE_OPENMP_SUNDIALS)
   set(OPENMP_ENABLE ON CACHE INTERNAL "")
endif(${PROJECT_NAME}_ENABLE_OPENMP_SUNDIALS)

if (${PROJECT_NAME}_ENABLE_KLU)
        set(KLU_ENABLE ON CACHE INTERNAL "")
    else()
        set(KLU_ENABLE OFF CACHE INTERNAL "")
    endif()

add_subdirectory(${sundials_SOURCE_DIR} ${sundials_BINARY_DIR})

add_library(sundials_all INTERFACE)
target_include_directories(sundials_all INTERFACE $<BUILD_INTERFACE:${sundials_SOURCE_DIR}/include>)
target_include_directories(sundials_all INTERFACE $<BUILD_INTERFACE:${sundials_BINARY_DIR}/include>)
add_library(SUNDIALS::SUNDIALS ALIAS sundials_all) 

set(SUNDIALS_LIBRARIES
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

if (${PROJECT_NAME}_ENABLE_CVODE)
    list(APPEND SUNDIALS_LIBRARIES sundials_cvode_static)
endif()

if (${PROJECT_NAME}_ENABLE_ARKODE)
    list(APPEND SUNDIALS_LIBRARIES sundials_arkode_static)
endif()

target_link_libraries(sundials_all INTERFACE ${SUNDIALS_LIBRARIES})

if (TARGET sundials_nvecopenmp_static )
   set_target_properties ( sundials_nvecopenmp_static PROPERTIES FOLDER sundials)

   target_link_libraries(sundials_all INTERFACE sundials_nvecopenmp_static)
endif()

if (TARGET sundials_sunlinsolklu_static)
    set_target_properties (sundials_sunlinsolklu_static PROPERTIES FOLDER sundials)
    target_link_libraries(sundials_all INTERFACE sundials_sunlinsolklu_static)
endif()

if (MSVC)
target_compile_options(sundials_cvode_static PRIVATE "/sdl-")
target_compile_options(sundials_cvode_static PRIVATE "/W3")
endif()
