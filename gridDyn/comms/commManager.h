
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

#ifndef _COMM_MANAGER_H_
#define _COMM_MANAGER_H_
#include <string>
#include <cstdint>
#include <memory>

class gridCommunicator;
class propertyBuffer;
class commMessage;

/** class for build and maintaining communicators*/
class commManager
{
private:
	std::string commName;        //!< The name to use on the commlink
	std::uint64_t commId = 0;        //!< the id to use on the commlink
	std::string commType;       //!< the type of comms to construct
	std::string commDestName;       //!< the default communication destination as a string
	std::uint64_t commDestId = 0;       //!< the default communication destination id

	std::shared_ptr<gridCommunicator> commLink;             //!<communicator link
	propertyBuffer *commPropBuffer = nullptr;

public:
	commManager();
	commManager(const commManager &cm);
	commManager(commManager &&cm);
	~commManager();

	commManager &operator=(const commManager &cm);
	commManager &operator=(commManager &&cm);

	bool set(const std::string &param, const std::string &val);
	bool set(const std::string &param, double val);
	bool setFlag(const std::string &param, bool val);
	std::shared_ptr<gridCommunicator> build();
	std::shared_ptr<gridCommunicator> getCommLink() const
	{
		return commLink;
	}

	void send(std::shared_ptr<commMessage> m);

	const std::string &destName() const
	{
		return commDestName;
	}
	const std::string &name() const
	{
		return commName;
	}

	std::uint64_t id() const
	{
		return commId;
	}
	std::uint64_t destId() const
	{
		return commDestId;
	}
};
#endif