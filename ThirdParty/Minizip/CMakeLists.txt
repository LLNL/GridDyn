cmake_minimum_required(VERSION 2.8)

project(minizip C)

set(BUILD_SHARED_LIBS OFF)

cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)

if (EXISTS ${ZLIB_LOCATION}/cmake/zlib-targets.cmake)
include (${ZLIB_LOCATION}/cmake/zlib-targets.cmake)

else()
find_package(ZLIB)
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
  add_definitions(/wd4131 /wd4013 /wd4127 /wd4100 /wd4189 /wd4456 -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

include_directories(${ZLIB_LOCATION}/include)
set(SOURCES
  ioapi.c
  miniunz.c
  minizip.c
  #mztools.c
  unzip.c
  zip.c
)

if(WIN32)
    set(SOURCES ${SOURCES} iowin32.c)
endif(WIN32)

set(HEADERS
  crypt.h
  ioapi.h  
  miniunz.h
  minizip.h
  #mztools.h
  unzip.h
)

if(WIN32)
    set(HEADERS ${HEADERS} iowin32.h)
endif(WIN32)

if (NOT CMAKE_DEBUG_POSTFIX)
set(CMAKE_DEBUG_POSTFIX d)
endif()

add_library(minizip ${SOURCES} ${HEADERS})


	install(TARGETS minizip EXPORT minizip-targets
       LIBRARY DESTINATION lib
       ARCHIVE DESTINATION lib
	)

install (EXPORT minizip-targets NAMESPACE minizip:: DESTINATION cmake/)
INSTALL(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/minizip)
