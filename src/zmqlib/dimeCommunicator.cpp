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

#include "dimeCommunicator.h"
#include "zmqLibrary/zmqHelper.h"
#include "core/helperTemplates.hpp"
#include "cppzmq/zmq_addon.hpp"

namespace griddyn
{
namespace dimeLib
{
dimeCommunicator::dimeCommunicator() :zmqCommunicator()
{

}

dimeCommunicator::dimeCommunicator(std::string name) : zmqCommunicator(name)
{

}

dimeCommunicator::dimeCommunicator(std::string name, std::uint64_t id) : zmqCommunicator(name, id)
{

}

dimeCommunicator::dimeCommunicator(std::uint64_t id) : zmqCommunicator(id)
{

}

std::shared_ptr<Communicator> dimeCommunicator::clone(std::shared_ptr<Communicator> comm) const
{
	auto dimeComm = cloneBaseStack<dimeCommunicator, zmqCommunicator,Communicator>(this, comm);
	if (!dimeComm)
	{
		return comm;
	}
	return dimeComm;
}

void dimeCommunicator::messageHandler(const zmq::multipart_t &msg)
{

}

void dimeCommunicator::addHeader(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message)
{

}

void dimeCommunicator::addMessageBody(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message)
{

}

void dimeCommunicator::set(const std::string &param, const std::string &val)
{
	if (param[0] == '#')
	{
		
	}
	
	else
	{
		zmqCommunicator::set(param, val);
	}

}

void dimeCommunicator::set(const std::string &param, double val)
{
	zmqCommunicator::set(param, val);
}

void dimeCommunicator::setFlag(const std::string &flag, bool val)
{
	if (flag[0] == '#') 
	{
		
	}
	else
	{
		zmqCommunicator::setFlag(flag, val);
	}

}

}//namespace dimeLib
}//namespace griddyn