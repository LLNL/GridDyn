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

#ifndef ZMQ_HELPER_HEADER_
#define ZMQ_HELPER_HEADER_

#include <string>

namespace zmq
{
	enum class socket_type:int;
}
namespace zmqlib
{
zmq::socket_type socketTypeFromString(const std::string &socketType);
}// namespace zmqlib
#endif
