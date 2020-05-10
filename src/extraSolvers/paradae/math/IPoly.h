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
#ifndef IPoly_h
#define IPoly_h

#include "../common/def.h"
#include "DenseMatrix.h"
#include "PMultiVector.h"
#include "Vector.h"

namespace griddyn {
namespace paradae {
    class IPoly {
        // y values and associated times
        PMultiVector tyy;
        PMultiVector yy;
        // yp values and associated times
        PMultiVector typ;
        PMultiVector yp;
        // Vandermonde Matrix
        DenseMatrix M;
        bool is_built;
        int nyy, nyp;
        int nx;
        Real min_time, max_time;

      public:
        IPoly(const PMultiVector& tyy_, const PMultiVector& yy_);
        IPoly(const PMultiVector& tyy_,
              const PMultiVector& yy_,
              const PMultiVector& typ_,
              const PMultiVector yp_);
        void Build();
        void GetValueY(Real t, Vector& y);
        void GetValueDY(Real t, Vector& yp);
        inline int GetXSize() { return nx; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
