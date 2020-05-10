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
#ifndef Equation_DAE_h
#define Equation_DAE_h

#include "Equation.h"

namespace griddyn {
namespace paradae {
    class Equation_DAE: public Equation {
      public:
        // Redefinition of inherited virtual methods
        virtual ~Equation_DAE(){};
        virtual type_Equation GetTypeEq() { return DAE; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
