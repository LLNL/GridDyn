# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_yaml)

    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_yaml)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(yaml-cpp
	SOURCE_DIR ${PROJECT_BINARY_DIR}/Download/yaml-cpp
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp.git
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/yaml-cpp
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=Release
		-DYAML_CPP_BUILD_TOOLS=OFF
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
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
		
	
#now build the debug version	
		set(trigger_build_debug_dir ${CMAKE_BINARY_DIR}/autobuild/force_yaml_debug)
	 file(MAKE_DIRECTORY ${trigger_build_debug_dir} ${trigger_build_debug_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT_DEBUG "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(yaml-cpp
    SOURCE_DIR ${PROJECT_BINARY_DIR}/Download/yaml-cpp
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/yaml-cpp-debug
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs/debug
        -DCMAKE_BUILD_TYPE=Debug
		-DBUILD_SHARED_LIBS=OFF
		-DYAML_CPP_BUILD_TOOLS=OFF
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
		-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
		
	INSTALL_DIR ${PROJECT_BINARY_DIR}/libs/debug
	)")


    file(WRITE ${trigger_build_debug_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT_DEBUG}")

    execute_process(COMMAND ${CMAKE_COMMAND}  -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	    -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_debug_dir}/build
        )

endfunction()