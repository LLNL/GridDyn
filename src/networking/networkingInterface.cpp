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

#include "griddyn/griddyn-config.h"

#ifdef ENABLE_ZMQ
#include "zmqInterface.h"
#endif
#ifdef ENABLE_TCP
#include "tcpInterface.h"
#endif


namespace griddyn
{
void loadNetworkingLibrary ()
{
#ifdef ENABLE_ZMQ
    loadZMQLibrary ();
#endif

#ifdef ENABLE_TCP
    loadTcpLibrary ();
#endif

}

}  // namespace griddyn
