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

#ifndef ZMQ_CONTEXT_MANAGER_HEADER_
#define ZMQ_CONTEXT_MANAGER_HEADER_

#include <vector>
#include <memory>
#include <string>

namespace zmq
{
	class context_t;
}

/** class defining a singleton context manager for all zmq usage in gridDyn*/
class zmqContextManager
{
private:
	static std::vector<std::shared_ptr<zmqContextManager>> contexts; //!< container for pointers to all the available contexts
	static std::shared_ptr<zmqContextManager> defContext;  //!< pointer to the default context
	std::string name;  //!< context name
	zmq::context_t *zcontext=nullptr; //!< pointer to the actual context
	zmqContextManager(const std::string &contextName);

public:
	static std::shared_ptr<zmqContextManager> getContextPointer(const std::string &contextName);
	static std::shared_ptr<zmqContextManager> getContextPointer();

	static zmq::context_t &getContext(const std::string &contextName);
	static zmq::context_t &getContext();

	static void closeContext(const std::string &contextName);
	static void closeContext();

	virtual ~zmqContextManager();

	const std::string &getName() const
	{
		return name;
	}

	zmq::context_t &getBaseContext() const
	{
		return *zcontext;
	}



};

#endif
