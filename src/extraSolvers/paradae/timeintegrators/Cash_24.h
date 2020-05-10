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
#ifndef Cash_24_h
#define Cash_24_h

#include "RungeKutta_SDIRK.h"

/*!
  Butcher tableau:
  (not accurate enough, reaches only \f$3.10^{-13}\f$ on the Expo1D)
  \f[\begin{array}{c|ccccc}
   0.435866521508  & 0.435866521508&&&&\\
   -0.7            &-1.13586652150 &  0.435866521508&&&\\
   0.8             & 1.08543330679 & -0.721299828287  & 0.435866521508&&\\
   0.924556761814  & 0.416349501547 & 0.190984004184  &-0.118643265417  & 0.435866521508&\\
   1.0             & 0.896869652944 & 0.0182725272734 &-0.0845900310706 &-0.266418670647 & 0.435866521508\\\hline
   (4) & 0.896869652944 & 0.0182725272734 &-0.0845900310706 &-0.266418670647 & 0.435866521508\\
   (2) & 1.05646216107052 & -0.0564621610705236 & 0.0 & 0.0 & 0.0
  \end{array}
  \f]
 */
namespace griddyn {
namespace paradae {
    class Cash_24: public RungeKutta_SDIRK {
      public:
        Cash_24(Equation* eq, bool variable_step = false);
        virtual std::string GetName() { return "RK_ImpCa_24"; };
    };
}  // namespace paradae
}  // namespace griddyn

#endif
