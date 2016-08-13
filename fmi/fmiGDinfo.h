/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  c-set-offset 'innamespace 0; -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2014, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/


#ifndef FMI_IMPORT_GD_H_
#define FMI_IMPORT_GD_H_

#include <fmilib.h>

int fmi1_test(fmi_import_context_t* context, const char* dirPath);

int fmi2_test(fmi_import_context_t* context, const char* dirPath);
/** function to create the linkages from the FMI library into the object factory system
*/
void loadFmiLibrary();

#define CTEST_RETURN_FAIL (-1)
#define CTEST_RETURN_SUCCESS (0)

#endif