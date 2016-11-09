/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
   * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "vectData.h"



static const index_t kNullLocation((index_t)(-1));

bool compareLocVectData (vLoc A, vLoc B)
{
  return (A.first < B.first);
}


void vectData::compact ()


void vectData::assign(index_t X, double num)


void vectData::assignCheck(index_t X, double num)


void vectData::sortIndex()


double vectData::find (index_t rowN)


void vectData::scale (double factor, index_t start, count_t count)

