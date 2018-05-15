# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_sundials)

	include(escape_string)
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_sundials)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)
#find the directory where KLU is located
    
    #message(STATUS "KLU_DIR=${SuiteSparse_LIBRARY_DIR}")
	
	message(STATUS "extra c flags ${EXTRA_C_FLAGS}")

    set(SUNDIALS_KLU_ENABLE "-DKLU_ENABLE=ON")
    set(SUNDIALS_KLU_INCLUDE_DIR "-DKLU_INCLUDE_DIR=${SuiteSparse_DIRECT_INCLUDE_DIR}")
    set(SUNDIALS_KLU_LIBRARY_DIR "-DKLU_LIBRARY_DIR=${SuiteSparse_DIRECT_LIBRARY_DIR}")
    if (DISABLE_KLU)
        set(SUNDIALS_KLU_ENABLE "-DKLU_ENABLE=OFF")
        unset(SUNDIALS_KLU_INCLUDE_DIR)
        unset(SUNDIALS_KLU_LIBRARY_DIR)
    endif()
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.5)
    include(ExternalProject)
ExternalProject_Add(sundials
    SOURCE_DIR ${PROJECT_BINARY_DIR}/Download/sundials
    GIT_REPOSITORY  https://github.com/llnl/sundials.git
    GIT_TAG v3.1.1
    UPDATE_COMMAND " " 
    BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/sundials
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
        -DBUILD_CVODES=OFF
        -DBUILD_IDAS=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DEXAMPLES_ENABLE_C=OFF
		-DEXAMPLES_ENABLE_CXX=OFF
        -DEXAMPLES_INSTALL=OFF
		-DSUNDIALS_INDEX_TYPE=int32_t
        -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
        \"-DCMAKE_C_FLAGS=${EXTRA_C_FLAGS}\"
        -DOPENMP_ENABLE=${OPENMP_FOUND}
	${SUNDIALS_KLU_ENABLE}
	${SUNDIALS_KLU_INCLUDE_DIR}
	${SUNDIALS_KLU_LIBRARY_DIR}
        -DCMAKE_C_COMPILER=${c_compiler_string}
		-DCMAKE_LINKER=${linker_string}
		-DCMAKE_DEBUG_POSTFIX=d
        
        
    INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
    )")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")
if (NOT BUILD_RELEASE_ONLY)
	message(STATUS "Configuring Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string}
	    -D CMAKE_LINKER=${linker_string}
        -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log
        )
	
	message(STATUS "Building Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log")	
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log
        )
	
	endif()
	
	message(STATUS "Configuring Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log")	
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string}
	    -D CMAKE_LINKER=${linker_string}
        -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log
        )
		
	message(STATUS "Building Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log
        )

endfunction()

function (build_sundialsMINGW)

message(STATUS "BUILD SUNDIALS with MINGW")
	include(escape_string)
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_sundials)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)
#find the directory where KLU is located
    
    #message(STATUS "KLU_DIR=${SuiteSparse_LIBRARY_DIR}")
	
    set(SUNDIALS_KLU_ENABLE "-DKLU_ENABLE=ON")
    set(SUNDIALS_KLU_INCLUDE_DIR "-DKLU_INCLUDE_DIR=${SuiteSparse_DIRECT_INCLUDE_DIR}")
    set(SUNDIALS_KLU_LIBRARY_DIR "-DKLU_LIBRARY_DIR=${SuiteSparse_DIRECT_LIBRARY_DIR}")
    set(SUNDIALS_SUITESPARSECONFIG "-DSUITESPARSECONFIG_LIBRARY=${SuiteSparse_SUITESPARSECONFIG_LIBRARY_RELEASE}")
    if (DISABLE_KLU)
        set(SUNDIALS_KLU_ENABLE "-DKLU_ENABLE=OFF")
        unset(SUNDIALS_KLU_INCLUDE_DIR)
        unset(SUNDIALS_KLU_LIBRARY_DIR)
        unset(SUNDIALS_SUITESPARSECONFIG)
    endif()
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.5)
    include(ExternalProject)
ExternalProject_Add(sundials
    SOURCE_DIR ${PROJECT_BINARY_DIR}/Download/sundials
    GIT_REPOSITORY  https://github.com/llnl/sundials.git
    GIT_TAG v3.1.0
    UPDATE_COMMAND " " 
    BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/sundials
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
        -DBUILD_CVODES=OFF
        -DBUILD_IDAS=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DEXAMPLES_ENABLE_C=OFF
		-DEXAMPLES_ENABLE_CXX=OFF
        -DEXAMPLES_INSTALL=OFF
		-DSUNDIALS_INDEX_TYPE=int32_t
        -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
        \"-DCMAKE_C_FLAGS=${EXTRA_C_FLAGS}\"
        -DOPENMP_ENABLE=${OPENMP_FOUND}
	${SUNDIALS_KLU_ENABLE}
	${SUNDIALS_KLU_INCLUDE_DIR}
	${SUNDIALS_KLU_LIBRARY_DIR}
	${SUNDIALS_SUITESPARSECONFIG}
        -DCMAKE_C_COMPILER=${c_compiler_string}
        -DCMAKE_LINKER=${linker_string}
		-DCMAKE_DEBUG_POSTFIX=d
        
        
    INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
    )")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")
if (NOT BUILD_RELEASE_ONLY)
	message(STATUS "Configuring Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string}
	    -D CMAKE_LINKER=${linker_string}
        -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_debug.log
        )
	
	message(STATUS "Building Sundials Autobuild for debug: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log")	
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_debug.log
        )
	
	endif()
	
	message(STATUS "Configuring Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log")	
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string}
	    -D CMAKE_LINKER=${linker_string}
        -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_config_release.log
        )
		
	message(STATUS "Building Sundials Autobuild for release: logging to ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/sundials_autobuild_build_release.log
        )

endfunction()
