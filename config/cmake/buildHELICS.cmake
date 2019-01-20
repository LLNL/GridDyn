# This function is used to force a build on a dependant project at cmake configuration phase.
# 

function (build_helics)

	include(escape_string)
	
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	
	#message(STATUS "${CMAKE_CXX_COMPILER} to ${compiler_string}")
    escape_string(binary_dir_string ${PROJECT_BINARY_DIR})

	escape_string(install_location_string ${HELICS_INSTALL_PATH})
    set(trigger_build_dir ${binary_dir_string}/autobuild/force_helics)

    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.5)
    include(ExternalProject)
ExternalProject_Add(helics
    SOURCE_DIR ${binary_dir_string}/Download/helics
    GIT_REPOSITORY  https://github.com/GMLC-TDC/HELICS-src.git
	GIT_TAG v2.0.0-beta.3
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
    BINARY_DIR ${binary_dir_string}/ThirdParty/helics
     
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${install_location_string}
		-DBOOST_INSTALL_PATH=${BOOST_ROOT}
		-DUSE_BOOST_STATIC_LIBS=${USE_BOOST_STATIC_LIBS}
        -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
		-DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
		-DBUILD_HELICS_TESTS=OFF
		-DBUILD_HELICS_EXAMPLES=OFF
		-DBUILD_C_SHARED_LIB=OFF
        -DCMAKE_CXX_COMPILER=${cxx_compiler_string}
        -DCMAKE_C_COMPILER=${c_compiler_string}
		-DCMAKE_LINKER=${linker_string}
		-DENABLE_CXX_17=${ENABLE_CXX_17}
		-DZeroMQ_ENABLE=ON
		-DAUTOBUILD_INSTALL_PATH=${AUTOBUILD_INSTALL_PATH}
        
        
    INSTALL_DIR ${install_location_string}

    )")

	file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

if (MSVC)

if (NOT BUILD_RELEASE_ONLY)
	
	message(STATUS "Configuring HELICS Autobuild for debug logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_debug.log")
	execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
         -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_debug.log
        )
		
	message(STATUS "Building HELICS debug build logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_debug.log")
	 execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_debug.log
        )

  endif()
  
  if (NOT BUILD_DEBUG_ONLY)
  if (NOT MSVC_RELEASE_BUILD_TYPE)
		set(MSVC_RELEASE_BUILD_TYPE "Release")
	endif()
  message(STATUS "Configuring HELICS Autobuild for ${MSVC_RELEASE_BUILD_TYPE} logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
         -D CMAKE_BUILD_TYPE=${MSVC_RELEASE_BUILD_TYPE} -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config_release.log
        )
	
	message(STATUS "Building HELICS ${MSVC_RELEASE_BUILD_TYPE} build logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${MSVC_RELEASE_BUILD_TYPE}
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build_release.log
        )
	endif()
else(MSVC)

	if (CMAKE_BUILD_TYPE)
	list(APPEND valid_btypes "Release" "Debug" "RelWithDebInfo" "MinSizeRel")
	if (${CMAKE_BUILD_TYPE} IN_LIST valid_btypes)
		set(LOCAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
	else()
		set(LOCAL_BUILD_TYPE "RelWithDebInfo")
	endif()
else ()
	set(LOCAL_BUILD_TYPE "Release")
endif()
	
	message(STATUS "Configuring HELICS Autobuild for ${LOCAL_BUILD_TYPE} logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config.log")
    execute_process(COMMAND ${CMAKE_COMMAND} -Wno-dev -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string} -D CMAKE_LINKER=${linker_string}
         -D CMAKE_BUILD_TYPE=${LOCAL_BUILD_TYPE} -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_config.log
        )
	
	message(STATUS "Building HELICS ${LOCAL_BUILD_TYPE} build logging to ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config ${LOCAL_BUILD_TYPE}
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/helics_autobuild_build.log
        )
	endif(MSVC)
	endfunction()
        
