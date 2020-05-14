/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#pragma once

#include "griddyn/griddyn-config.h"

/** @file
file linking with version info and containing some convenience functions
*/
namespace griddyn {
/** a string representation of the HELICS version*/
constexpr auto versionString = GRIDDYN_VERSION_STRING;

/** get the Major version number*/
constexpr int versionMajor = GRIDDYN_VERSION_MAJOR;
/** get the Minor version number*/
constexpr int versionMinor = GRIDDYN_VERSION_MINOR;
/** get the patch number*/
constexpr int versionPatch = GRIDDYN_VERSION_PATCH;
/** the build string if any*/
constexpr auto versionBuild = GRIDDYN_VERSION_BUILD;
/** build flags used to compile helics*/
constexpr auto buildFlags = GRIDDYN_BUILD_FLAGS;
/** compiler used to build helics*/
constexpr auto compiler = GRIDDYN_COMPILER_VERSION;
}  // namespace helics
