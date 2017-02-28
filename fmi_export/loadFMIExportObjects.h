/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#pragma once
#ifndef _FMI_LOADEXPORTOBJECTS_H_
#define _FMI_LOADEXPORTOBJECTS_H_

class readerInfo;

void loadFMIExportObjects();

void loadFmiExportReaderInfoDefinitions(readerInfo &ri);

#endif
