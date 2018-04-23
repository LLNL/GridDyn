/*
* LLNS Copyright Start
* Copyright (c) 2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef LineSearch_h
#define LineSearch_h

#include "../common/def.h"
#include "../math/Vector.h"
#include "Solver.h"

enum LS_error {LS_NOT_CONVERGED, LS_INF_NAN};

class LinearSearch : Solver{
  static const int max_iter_int=100;
public:
  LinearSearch(){};
  LinearSearch(int max_iter_);
  int Solve(Solver_App* app, Vector& x);
};

#endif
