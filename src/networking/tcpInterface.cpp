/*
* LLNS Copyright Start
* Copyright (c) 2014-2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "tcpInterface.h"

#include "core/factoryTemplates.hpp"
#include "core/objectFactory.hpp"
#include "tcpCollector.h"
#include "tcpCommunicator.h"

namespace griddyn {

static childClassFactory<tcpLib::tcpCollector, collector> tcpcol(std::vector<std::string>{"tcp"});

static childClassFactory<tcpLib::tcpCommunicator, Communicator>
    tcpComm(std::vector<std::string>{"tcp"});

void loadTcpLibrary()
{
    static int loaded = 0;

    if (loaded == 0) {
        loaded = 1;
    }
}

}  //namespace griddyn
