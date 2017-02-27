# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_sundials)

    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_sundials)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)
#find the directory where KLU is located
	get_filename_component(SuiteSparse_LIBRARY_DIR ${SuiteSparse_KLU_LIBRARY_RELEASE} DIRECTORY)
	message(STATUS "KLU_DIR=${SuiteSparse_LIBRARY_DIR}")
    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(sundials
    URL http://computation.llnl.gov/projects/sundials/download/sundials-2.7.0.tar.gz
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/sundials
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=Release
		-DBUILD_CVODES=OFF
		-DBUILD_IDAS=OFF
		-DBUILD_SHARED_LIBS=OFF
		-DEXAMPLES_ENABLE=OFF
		-DEXAMPLES_INSTALL=OFF
		-DKLU_ENABLE=ON
		-DOPENMP_ENABLE=${OPENMP_FOUND}
		-DKLU_INCLUDE_DIR=${SuiteSparse_INCLUDE_DIRS}
		-DKLU_LIBRARY_DIR=${SuiteSparse_LIBRARY_DIR}
		-DSUITESPARSECONFIG_LIBRARY=${SuiteSparse_SUITESPARSECONFIG_LIBRARY_RELEASE}
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		
		
	INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
	)")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

    execute_process(COMMAND ${CMAKE_COMMAND}  -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	    -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )

endfunction()