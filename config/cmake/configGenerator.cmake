# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

if ( MSVC )
    set(WERROR_FLAG "/W4 /WX")
else( MSVC )
    set(WERROR_FLAG "-Werror")
endif ( MSVC )

set(TEST_CXX_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test_compiler_cxx)

if (ENABLE_CXX_17)
    try_compile(HAVE_OPTIONAL ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_optional.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
    try_compile(HAVE_VARIANT ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_variant.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})
    try_compile(HAVE_STRING_VIEW ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_string_view.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})
    try_compile(HAVE_CLAMP ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_clamp.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
    try_compile(HAVE_HYPOT3 ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_hypot3.cpp  COMPILE_DEFINITIONS ${VERSION_OPTION})
    try_compile(HAVE_IF_CONSTEXPR ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_constexpr_if.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} )
    try_compile(HAVE_FALLTHROUGH ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_fallthrough.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} ${VERSION_OPTION})
    try_compile(HAVE_UNUSED ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_unused.cpp  COMPILE_DEFINITIONS ${WERROR_FLAG} ${VERSION_OPTION})
endif(ENABLE_CXX_17)

try_compile(HAVE_VARIABLE_TEMPLATES ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_variable_template.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

# this is normally a C++17 thing but clang <3.5 had it available before the standard switched to shared_timed_mutex
try_compile(HAVE_SHARED_MUTEX ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_shared_mutex.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

try_compile(HAVE_SHARED_TIMED_MUTEX ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_shared_timed_mutex.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})
try_compile(HAVE_EXPERIMENTAL_STRING_VIEW ${CMAKE_BINARY_DIR} ${TEST_CXX_DIRECTORY}/check_experimental_string_view.cpp COMPILE_DEFINITIONS ${VERSION_OPTION})

if (NOT NO_CONFIG_GENERATION)
    if (CONFIGURE_TARGET_LOCATION)
        CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/compiler-config.h.in ${CONFIGURE_TARGET_LOCATION}/compiler-config.h)
    else()
        CONFIGURE_FILE(${CMAKE_CURRENT_LIST_DIR}/compiler-config.h.in ${PROJECT_BINARY_DIR}/compiler-config.h)
    endif()
endif(NOT NO_CONFIG_GENERATION)
