# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_minizip)


include(escape_string)
	
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
	escape_string(zlib_includes_string ${ZLIB_INCLUDES})
	
	escape_string(binary_dir_string ${CMAKE_BINARY_DIR})
	
	escape_string(project_src_dir_string ${PROJECT_SOURCE_DIR})
	
    set(trigger_build_dir ${binary_dir_string}/autobuild/force_minizip)

    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(minizip
    SOURCE_DIR ${project_src_dir_string}/ThirdParty/Minizip
    DOWNLOAD_COMMAND " "
    UPDATE_COMMAND " "
	BINARY_DIR ${binary_dir_string}/ThirdParty/Minizip
	 
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${binary_dir_string}/libs
        -DCMAKE_BUILD_TYPE=Release
		-DCMAKE_MODULE_PATH=${project_src_dir_string}/config
		-DZLIB_INCLUDES=${zlib_includes_string}
		-DCMAKE_C_COMPILER=${c_compiler_string}
		-DCMAKE_LINKER=${linker_string}
		
	INSTALL_DIR ${binary_dir_string}/libs
	)")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

message(STATUS ${trigger_build_dir})

#execute_process(COMMAND ${CMAKE_COMMAND} WORKING_DIRECTORY ${trigger_build_dir}/build)

    execute_process(COMMAND ${CMAKE_COMMAND}  -Wno-dev -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER} -D CMAKE_LINKER=${CMAKE_LINKER}
        -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
        )

endfunction()