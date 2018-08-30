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

#include "dimeClientInterface.h"
#include "zmqLibrary/zmqContextManager.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

dimeClientInterface::dimeClientInterface (const std::string &dimeName, const std::string &dimeAddress)
    : name (dimeName), address (dimeAddress)
{
    if (address.empty ())
    {
#ifdef WIN32
        address = "tcp://127.0.0.1:5000";
#else
        address = "ipc:///tmp/dime";
#endif
    }
}

dimeClientInterface::~dimeClientInterface () = default;

void dimeClientInterface::init ()
{
    auto context = zmqlib::zmqContextManager::getContextPointer ();

    socket = std::make_unique<zmq::socket_t> (context->getBaseContext (), zmq::socket_type::req);
    socket->connect (address);

    Json_gd::Value outgoing;
    outgoing["command"] = "connect";
    outgoing["name"] = name;
    outgoing["listen_to_events"] = false;

    std::stringstream ss;
    writer->write(outgoing, &ss);

    socket->send (ss.str());

    char buffer[3] = {};
    auto sz = socket->recv (buffer, 3, 0);
    if ((sz != 2) || (strncmp(buffer, "OK", 3) != 0))
    {
        throw initFailure ();
    }
}

void dimeClientInterface::close ()
{
    if (socket)
    {
        Json_gd::Value outgoing;
        outgoing["command"] = "exit";
        outgoing["name"] = name;

        std::stringstream ss;
        writer->write(outgoing, &ss);

        socket->send (ss.str ());

        socket->close ();
    }
    socket = nullptr;
}

void dimeClientInterface::sync () {}

void encodeVariableMessage (Json_gd::Value &data, double val)
{
    Json_gd::Value content;
    content["stdout"] = "";
    content["figures"] = "";
    content["datadir"] = "/tmp MatlabData/";

    Json_gd::Value response;
    response["content"] = content;
    response["result"] = val;
    response["success"] = true;
    data["args"] = response;
    // response = { 'content': {'stdout': '', 'figures' : [], 'datadir' : '/tmp MatlabData/'}, 'result' : value,
    // 'success' : True } 	outgoing = { 'command': 'response', 'name' : self.name, 'meta' : {'var_name':
    // var_name},
    //'args' : self.matlab.json_encode(response) }
}
void dimeClientInterface::send_var (const std::string &varName, double val, const std::string &recipient)
{
    // outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
    Json_gd::Value outgoing;

    outgoing["command"] = (recipient.empty ()) ? "broadcast" : "send";

    outgoing["name"] = name;
    outgoing["args"] = varName;

    std::stringstream ss;
    writer->write(outgoing, &ss);
    socket->send (ss.str());

    char buffer[3];
    auto sz = socket->recv (buffer, 3, 0);
    // TODO check recv value

    Json_gd::Value outgoingData;
    outgoingData["command"] = "response";
    outgoingData["name"] = name;
    if (!recipient.empty ())
    {
        outgoingData["meta"]["recipient_name"] = recipient;
    }

    outgoingData["meta"]["var_name"] = varName;
    encodeVariableMessage (outgoingData, val);

    std::stringstream().swap(ss); // reset ss
    writer->write(outgoingData, &ss);
    socket->send (ss.str ());

    sz = socket->recv (buffer, 3, 0);
    if (sz != 2) // TODO check for "OK"
    {
        throw (sendFailure ());
    }
}

void dimeClientInterface::broadcast (const std::string &varName, double val)
{
    send_var (varName, val);
}

void dimeClientInterface::get_devices () {}
