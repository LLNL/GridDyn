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

#ifndef ZMQREACTOR_H_
#define ZMQREACTOR_H_
#include "zmqSocketDescriptor.h"
#include "utilities/simpleQueue.hpp"
#include <memory>
#include <thread>
#include <atomic>

namespace zmqlib
{
class zmqContextManager;


/** class that manages receive sockets and triggers callbacks
@detail the class starts up a thread that listens for */
class zmqReactor
{
private:
	enum class reactorInstruction :int
	{
		newSocket,
		close,
		modify,
		terminate,
	};
	static std::vector<std::shared_ptr<zmqReactor>> reactors; //!< container for pointers to all the available contexts

	std::string name;
	std::shared_ptr<zmqContextManager> contextManager;  //!< pointer the context the reactor is using

	simpleQueue<std::pair<reactorInstruction, zmqSocketDescriptor>> updates; //!< the modifications to make the reactor sockets

	std::unique_ptr<zmq::socket_t> notifier;
	std::thread loopThread;
	/** private constructor*/
	zmqReactor(const std::string &reactorName, const std::string &context);
	std::atomic<bool> reactorLoopRunning{ false };
public:
	static std::shared_ptr<zmqReactor> getReactorInstance(const std::string &reactorName, const std::string &context="");


	~zmqReactor();

	void addSocket(const zmqSocketDescriptor &desc);
	void modifySocket(const zmqSocketDescriptor &desc);
	void closeSocket(const std::string &name);

	void addSocketBlocking(const zmqSocketDescriptor &desc);
	void modifySocketBlocking(const zmqSocketDescriptor &desc);
	void closeSocketBlocking(const std::string &name);

	const std::string &getName() const
	{
		return name;
	}
	void terminateReactor();

private:
	void reactorLoop();

};

} //zmqlib
#endif

