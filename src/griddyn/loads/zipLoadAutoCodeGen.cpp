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

 #include "zipLoad.h"
 #include "core/coreObjectTemplates.hpp"
 #include <map>

 using namespace std;
 using namespace gridUnits;
 using namespace griddyn;

 enum class zipLoadParams {  IP, IQ, YP, YQ, };


 static const map<string, zipLoadParams> ParamsMapzipLoad{
 {"ip", zipLoadParams::IP},
 {"iq", zipLoadParams::IQ},
 {"yp", zipLoadParams::YP},
 {"yq", zipLoadParams::YQ}
 };

 namespace griddyn  { 
     double zipLoad::get(const string &param, units_t unitType) const
     {
         double val = kNullVal;
         auto it = ParamsMapzipLoad.find(param);

         if(it == ParamsMapzipLoad.end())
         {
             return  Load::get(param, unitType);
         }
         switch (it->second)
         {
             case zipLoadParams::IP:
                 val = unitConversion (Ip, pu, unitType, systemBasePower);
                 break;
             case zipLoadParams::IQ:
                 val = unitConversion (Iq, pu, unitType, systemBasePower);
                 break;
             case zipLoadParams::YP:
                 val = unitConversion (Yp, pu, unitType, systemBasePower);
                 break;
             case zipLoadParams::YQ:
                 val = unitConversion (Yq, pu, unitType, systemBasePower);
                 break;
             default:
                 val = Load::get(param, unitType);
                 break;
         }
         return val;
     }


 }
