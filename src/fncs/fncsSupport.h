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

#ifndef FNCS_SUPPORT_HEADER_
#define FNCS_SUPPORT_HEADER_

#include "config.h"
#include "fncs.hpp"
#include "gridDynDefinitions.h"
#include <memory>
#include <vector>

#ifdef HAVE_VARIABLE_TEMPLATES
template<typename T>
constexpr T fncsTickPerSecond = T(1'000'000.00);
#else
const double fncsTickPerSecond_f(1'000'000.00);
const unsigned long long fncsTickPerSecond_i(1'000'000);
#endif

fncs::time gd2fncsTime(coreTime evntTime);

coreTime fncs2gdTime(fncs::time ftime);

class zplInfo {
  public:
    std::string name;
    std::string brokerAddress;
    int minTimeStep;
    std::string minTimeStepUnits;
};

class fncsRegister {
  public:
    enum class dataType {
        fncsInteger,
        fncsDouble,
        fncsComplex,
        fncsString,
        fncsArray,
        fncsJSON,
    };

  private:
    class cInfo {
      public:
        std::string topic;
        std::string defValue;
        dataType type;
        bool list;
        cInfo(const std::string& top,
              dataType fncstype,
              const std::string& def = "",
              bool lst = false):
            topic(top),
            defValue(def), type(fncstype), list(lst){};
    };
    static std::shared_ptr<fncsRegister> p_instance;

    std::vector<cInfo> subscriptions;
    std::vector<cInfo> publications;

    fncsRegister(){};

  public:
    void registerSubscription(const std::string& sub,
                              dataType dtype = dataType::fncsDouble,
                              const std::string& defval = "",
                              bool requestList = false);
    void registerPublication(const std::string& pub, dataType dtype = dataType::fncsDouble);
    std::string makeZPLConfig(const zplInfo& info);

    static std::shared_ptr<fncsRegister> instance();

  private:
    std::string type2string(dataType dtype);
};
#endif
