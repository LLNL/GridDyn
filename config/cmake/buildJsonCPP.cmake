# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_jsoncpp)

    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_jsoncpp)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(jsoncpp
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/jsoncpp
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=Release
		-DJSONCPP_WITH_TESTS=OFF
		-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
		-DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF
		-DCMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/config
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_LINKER=${CMAKE_LINKER}
		-DCMAKE_CXX_STANDARD=14
		
	INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
	)")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
	    -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )

#now build the debug version
set(trigger_build_debug_dir ${CMAKE_BINARY_DIR}/autobuild/force_jsoncpp_debug)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_debug_dir} ${trigger_build_debug_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(jsoncpp
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/jsoncpp_debug
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs/debug
        -DCMAKE_BUILD_TYPE=Debug
		-DJSONCPP_WITH_TESTS=OFF
		-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
		-DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		-DCMAKE_LINKER=${CMAKE_LINKER}
		-DCMAKE_CXX_STANDARD=14
		
	INSTALL_DIR ${PROJECT_BINARY_DIR}/libs/debug
	)")


    file(WRITE ${trigger_build_debug_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

    execute_process(COMMAND ${CMAKE_COMMAND}  -Wno-dev -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
	    -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )

endfunction()