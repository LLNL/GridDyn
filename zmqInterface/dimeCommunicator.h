/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef DIME_COMMUNICATOR_HEADER_
#define DIME_COMMUNICATOR_HEADER_

#include "zmqCommunicator.h"

/** class implementing a communicator to interact with the DIME Communication methods*/
class dimeCommunicator :public zmqCommunicator
{

public:

	dimeCommunicator();
	explicit dimeCommunicator(std::string name);
	dimeCommunicator(std::string name, std::uint64_t id);
	explicit dimeCommunicator(std::uint64_t id);

	virtual std::shared_ptr<gridCommunicator> clone(std::shared_ptr<gridCommunicator> comm) const override;

	virtual void set(const std::string &param, const std::string &val) override;
	virtual void set(const std::string &param, double val) override;
	virtual void setFlag(const std::string &flag, bool val) override;

protected:
	virtual void messageHandler(const zmq::multipart_t &msg);
	virtual void addHeader(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message);
	virtual void addMessageBody(zmq::multipart_t &msg, std::shared_ptr<commMessage> &message);

private:

};
#endif