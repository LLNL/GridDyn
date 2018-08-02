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
#ifndef READERHELPER_H_
#define READERHELPER_H_

#include "griddyn/gridDynDefinitions.hpp"
#include <iostream>
#include <stdexcept>

#define READER_DEFAULT_PRINT READER_NO_PRINT

#define LEVELPRINT(LEVEL, X)                                                                                      \
    if (LEVEL <= readerConfig::printMode)                                                                         \
    (std::cout << X << '\n')

#define WARNPRINT(LEVEL, X)                                                                                       \
    if (LEVEL <= readerConfig::warnMode)                                                                          \
    (++readerConfig::warnCount, std::cout << "WARNING(" << readerConfig::warnCount << "): " << X << '\n')

// helper function for grabbing parameters and attributes from an xml file

namespace griddyn
{
class gridParameter;
class readerInfo;
class coreObject;
class basicReaderInfo;

void paramStringProcess (gridParameter &param, readerInfo &ri);

double convertBV (std::string &bv);

using mArray = std::vector<std::vector<double>>;

double interpretString (const std::string &command, readerInfo &ri);

// NOTE:PT I am leaving these as size_t since they are part of file reading and text location types and spread
// across multiple files
void readMatlabArray (const std::string &text, size_t start, mArray &matA);
bool readMatlabArray (const std::string &Name, const std::string &text, mArray &matA);
stringVec readMatlabCellArray (const std::string &text, size_t start);
void removeMatlabComments (std::string &text);

void loadPSAT (coreObject *parentObject, const std::string &filetext, const basicReaderInfo &bri);
void loadMatPower (coreObject *parentObject,
                   const std::string &filetext,
                   const std::string &basename,
                   const basicReaderInfo &bri);
void loadMatDyn (coreObject *parentObject, const std::string &filetext, const basicReaderInfo &bri);
void loadMatDynEvent (coreObject *parentObject, const std::string &filetext, const basicReaderInfo &bri);

}  // namespace griddyn
#endif
