# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_tinyxml2)

    set(TinyXML2_CMAKE_C_COMPILER "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
    set(TinyXML2_CMAKE_CXX_COMPILER "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
 
    if(ENABLE_AUTOBUILD_COMPILERS)
        unset(TinyXML2_CMAKE_C_COMPILER)
        unset(TinyXML2_CMAKE_CXX_COMPILER)
    endif()
    
    set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_tinyxml2)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(tinyxml2
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ThirdParty/tinyxml2
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
    BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/tinyxml2
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
		-DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
        -DCMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/config/cmake
        ${TinyXML2_CMAKE_CXX_COMPILER}
        ${TinyXML2_CMAKE_C_COMPILER}
        -DCMAKE_LINKER=${CMAKE_LINKER}
        
    INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
    )")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev ${TinyXML2_CMAKE_CXX_COMPILER} ${TinyXML2_CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
        -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/tinyxml2_autobuild_config_release.log
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/tinyxml2_autobuild_build_release.log
        )
		    
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev ${TinyXML2_CMAKE_CXX_COMPILER} ${TinyXML2_CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
        -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/tinyxml2_autobuild_config_debug.log
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/tinyxml2_autobuild_build_debug.log
        )

endfunction()
