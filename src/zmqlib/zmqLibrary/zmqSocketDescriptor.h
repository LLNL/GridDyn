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

#ifndef ZMQ_SOCKET_DESCRIPTOR_H_
#define ZMQ_SOCKET_DESCRIPTOR_H_

#include <string>
#include <vector>
#include <functional>
#include <zmqlib/cppzmq/zmq_addon.hpp>
#include <memory>

namespace zmqlib
{
/** enumeration of possible operations on a socket*/
enum class socket_ops
{
	bind,
	connect,
	unbind,
	disconnect,
	subscribe,
	unsubscribe,
};


/** data class describing a socket and some operations on it*/
class zmqSocketDescriptor
{
public:
	std::string name;  //!< name of the socket for later reference
	zmq::socket_type type = zmq::socket_type::sub;  //!< the socket type
private:
	using socketOperation = std::pair<socket_ops, std::string>; //!< easy definition for operation instruction

	std::vector<socketOperation> ops;	//!< a list of connections of make through bind
public:
	std::function<void(const zmq::multipart_t &res)> callback; //!< the message handler
	/** constuctor that takes a socketName
	@param[in] socketName the name of the socket*/
	/*IMPLICIT*/ zmqSocketDescriptor(const std::string &socketName = "") :name(socketName) {};  //purposefully implicit
	/** alternate constructor with name and type
	@param[in] socketName the name of the socket
	@param[in] stype the type of the socket
	*/
	zmqSocketDescriptor(const std::string &socketName, zmq::socket_type stype) :name(socketName), type(stype) {};
	/** add an operation to the descriptor
	@param[in] op the type of operation to add
	@param[in] desc a description of the operation */
	inline void addOperation(socket_ops op, const std::string &desc)
	{
		ops.emplace_back(op, desc);
	}
	/** generate the socket in a particular context with the actions specified already executed on the newly created socket
	@param[in] ctx the zmq context in which to generate the socket
	@return a socket object
	*/
	zmq::socket_t makeSocket(zmq::context_t &ctx) const;
	/** make a pointer to the socket with the actions specified already executed on the newly created socket
	@param[in] ctx the zmq context in which to generate the socket
	@return a unique pointer to a socket
	*/
	std::unique_ptr<zmq::socket_t> makeSocketPtr(zmq::context_t &ctx) const;
	/** use the descriptor to modify an existing socket
	@param[in] sock the socket to modify
	*/
	void modifySocket(zmq::socket_t &sock) const;
};

} //zmqlib
#endif
