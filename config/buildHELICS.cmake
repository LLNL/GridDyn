# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_helics)

	include(escape_string)
	
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
	#message(STATUS "${CMAKE_CXX_COMPILER} to ${compiler_string}")
	
	escape_string(binary_dir_string ${CMAKE_BINARY_DIR})
    set(trigger_build_dir ${binary_dir_string}/autobuild/force_helics)

    get_filename_component(ZMQ_TARGET ${ZMQ_INCLUDE_DIR} DIRECTORY)
	escape_string(zmq_target "${ZMQ_TARGET}")
	message(STATUS "BUILDING HELICS WITH ZMQ target=${ZMQ_TARGET}")
    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(helics
    SOURCE_DIR ${binary_dir_string}/Download/helics
    GIT_REPOSITORY  https://github.com/GMLC-TDC/HELICS-src.git
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
    BINARY_DIR ${binary_dir_string}/ThirdParty/helics
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${binary_dir_string}/libs
        -DCMAKE_BUILD_TYPE=Release
		-DBOOST_ROOT=${BOOST_ROOT}
		-DBUILD_HELICS_TESTS=OFF
		-DBUILD_SHARED_LIBS=OFF
		-DBUILD_PYTHON=OFF
        -DCMAKE_CXX_COMPILER=${cxx_compiler_string}
        -DCMAKE_C_COMPILER=${c_compiler_string}
		-DZMQ_ENABLE=ON
		-DZMQ_INSTALL_PATH:PATH=${zmq_target}/
        -DCMAKE_LINKER=${linker_string}
        
    INSTALL_DIR ${binary_dir_string}/libs
    )")



    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compilier_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
        -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )
        
    
#now build the debug version	
        set(trigger_build_debug_dir ${binary_dir_string}/autobuild/force_helics_debug)
     file(MAKE_DIRECTORY ${trigger_build_debug_dir} ${trigger_build_debug_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT_DEBUG "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(helics
    SOURCE_DIR ${binary_dir_string}/Download/helics
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
    BINARY_DIR ${binary_dir_string}/ThirdParty/helics-debug
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${binary_dir_string}/libs/debug
        -DCMAKE_BUILD_TYPE=Debug
		-DBOOST_ROOT=${BOOST_ROOT}
		-DBUILD_HELICS_TESTS=OFF
		-DBUILD_SHARED_LIBS=OFF
		-DBUILD_PLAYER=OFF
		-DBUILD_RECORDER=OFF
		-DBUILD_BROKER=OFF
		-DBUILD_PYTHON=OFF
        -DCMAKE_CXX_COMPILER=${cxx_compiler_string}
        -DCMAKE_C_COMPILER=${c_compiler_string}
		-DZMQ_ENABLE=ON
		-DZMQ_INSTALL_PATH=${zmq_target}/
        -DCMAKE_LINKER=${linker_string}
        
    INSTALL_DIR ${binary_dir_string}/libs/debug
    )")


    file(WRITE ${trigger_build_debug_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT_DEBUG}")

    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
        -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )

endfunction()