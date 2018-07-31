/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
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

#ifndef ZMQ_SOCKET_DESCRIPTOR_H_
#define ZMQ_SOCKET_DESCRIPTOR_H_

#include "../cppzmq/zmq_addon.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

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

typedef std::pair<socket_ops, std::string> socketOperation;  //!< easy definition for operation instruction

/** data class describing a socket and some operations on it*/
class zmqSocketDescriptor
{
  public:
    std::string name;  //!< name of the socket for later reference
    zmq::socket_type type = zmq::socket_type::sub;  //!< the socket type
    std::vector<socketOperation> ops;  //!< a list of connections of make through bind
    std::function<void(const zmq::multipart_t &res)> callback;  //!< the message handler
    zmqSocketDescriptor (const std::string &socketName = "") : name (socketName){};  // purposefully implicit
    zmqSocketDescriptor (const std::string &socketName, zmq::socket_type stype)
        : name (socketName), type (stype){};
    inline void addOperation (socket_ops op, const std::string &desc) { ops.emplace_back (op, desc); }
    zmq::socket_t makeSocket (zmq::context_t &ctx) const;
    std::unique_ptr<zmq::socket_t> makeSocketPtr (zmq::context_t &ctx) const;
    void modifySocket (zmq::socket_t &sock) const;
};

}  // namespace zmqlib
#endif
