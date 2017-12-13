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

#ifndef HELICS_SUPPORT_HEADER_
#define HELICS_SUPPORT_HEADER_


#include "helics/helics.hpp"
#include "gridDynDefinitions.hpp"
#include <complex>
#include <future>

namespace griddyn
{
namespace helicsLib
{

helics::Time gd2helicsTime(coreTime evntTime);

coreTime helics2gdTime(helics::Time ftime);

std::future<int> runBroker(const std::string &cmd_args);
std::future<int> runPlayer(const std::string &cmd_args);
std::future<int> runRecorder(const std::string &cmd_args);

}// namespace helicsLib
} // namespace griddyn
#endif