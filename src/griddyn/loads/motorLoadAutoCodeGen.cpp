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

 /*  This code has been automatically generated. */

 #include "motorLoad.h"
 #include "core/coreObjectTemplates.hpp"
 #include <map>

 using namespace std;
 using namespace gridUnits;
 using namespace griddyn::loads;

 enum class motorLoadParams {  PMOT, R, X, R1, X1, XM, H, ALPHA, BETA, GAMMA, A, B, C, MBASE, VCONTROL, INIT_SLIP, SCALE, };


 static const map<string, motorLoadParams> ParamsMapmotorLoad{
 {"pmot", motorLoadParams::PMOT},
 {"r", motorLoadParams::R},
 {"x", motorLoadParams::X},
 {"r1", motorLoadParams::R1},
 {"x1", motorLoadParams::X1},
 {"xm", motorLoadParams::XM},
 {"h", motorLoadParams::H},
 {"alpha", motorLoadParams::ALPHA},
 {"beta", motorLoadParams::BETA},
 {"gamma", motorLoadParams::GAMMA},
 {"a", motorLoadParams::A},
 {"b", motorLoadParams::B},
 {"c", motorLoadParams::C},
 {"mbase", motorLoadParams::MBASE},
 {"vcontrol", motorLoadParams::VCONTROL},
 {"init_slip", motorLoadParams::INIT_SLIP},
 {"scale", motorLoadParams::SCALE}
 };

 namespace griddyn  { 
     namespace loads  { 
         double motorLoad::get(const string &param, units_t unitType) const
         {
             double val = kNullVal;
             auto it = ParamsMapmotorLoad.find(param);

             if(it == ParamsMapmotorLoad.end())
             {
                 return  Load::get(param, unitType);
             }
             switch (it->second)
             {
                 case motorLoadParams::PMOT:
                     val = Pmot;
                     break;
                 case motorLoadParams::R:
                     val = r;
                     break;
                 case motorLoadParams::X:
                     val = x;
                     break;
                 case motorLoadParams::R1:
                     val = r1;
                     break;
                 case motorLoadParams::X1:
                     val = x1;
                     break;
                 case motorLoadParams::XM:
                     val = xm;
                     break;
                 case motorLoadParams::H:
                     val = H;
                     break;
                 case motorLoadParams::ALPHA:
                     val = alpha;
                     break;
                 case motorLoadParams::BETA:
                     val = beta;
                     break;
                 case motorLoadParams::GAMMA:
                     val = gamma;
                     break;
                 case motorLoadParams::A:
                     val = a;
                     break;
                 case motorLoadParams::B:
                     val = b;
                     break;
                 case motorLoadParams::C:
                     val = c;
                     break;
                 case motorLoadParams::MBASE:
                     val = mBase;
                     break;
                 case motorLoadParams::VCONTROL:
                     val = Vcontrol;
                     break;
                 case motorLoadParams::INIT_SLIP:
                     val = init_slip;
                     break;
                 case motorLoadParams::SCALE:
                     val = scale;
                     break;
                 default:
                     val = Load::get(param, unitType);
                     break;
             }
             return val;
         }


     }
 }
