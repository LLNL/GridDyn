cmake_minimum_required(VERSION 2.8)

project(minizip C)

set(BUILD_SHARED_LIBS OFF)
set(SKIP_INSTALL_HEADERS ON)
set(SKIP_INSTALL_FILES ON)
if(NOT FMILIB_INSTALL_SUBLIBS)
	set(SKIP_INSTALL_LIBRARIES ON)
endif()
add_subdirectory("../Zlib/zlib-1.2.6" "${CMAKE_BINARY_DIR}/zlib")

if(CMAKE_CL_64)
	set_target_properties(zlib PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
endif()

if(CMAKE_HOST_APPLE)
set(PLATFORM __APPLE__)
elseif(CMAKE_HOST_UNIX)
set(PLATFORM unix)
elseif(CMAKE_HOST_WIN32)
set(PLATFORM _WIN32)
else(CMAKE_HOST_APPLE)
message(FATAL_ERROR "Not supported Platform")
endif(CMAKE_HOST_APPLE)

add_definitions(-D${PLATFORM})

if(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

if(MSVC)
  add_definitions(/wd4131 /wd4013 /wd4127 /wd4100 /wd4189 /wd4456)
endif(MSVC)

include_directories("../Zlib/zlib-1.2.6" "${CMAKE_BINARY_DIR}/zlib")
set(SOURCE
  ioapi.c
  miniunz.c
  minizip.c
  #mztools.c
  unzip.c
  zip.c
)

if(WIN32)
    set(SOURCE ${SOURCE} iowin32.c)
endif(WIN32)

set(HEADERS
  crypt.h
  ioapi.h  
  miniunz.h
  #mztools.h
  unzip.h
)

if(WIN32)
    set(HEADERS ${HEADERS} iowin32.h)
endif(WIN32)

add_library(minizip ${SOURCE} ${HEADERS})

target_link_libraries(minizip zlib)


	install(TARGETS minizip
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib 
	)

