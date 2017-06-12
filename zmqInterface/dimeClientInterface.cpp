/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
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
#include "zmqContextManager.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "formatInterpreters/json/json.h"
#pragma warning(pop)
#else
#include "formatInterpreters/json/json.h"
#endif

dimeClientInterface::dimeClientInterface(const std::string &dimeName, const std::string &dimeAddress):name(dimeName),address(dimeAddress)
{
	if (address.empty())
	{

#ifdef WIN32
		address = "tcp://127.0.0.1:5000";
#else
		address = "ipc:///tmp/dime";
#endif
	}
}

dimeClientInterface::~dimeClientInterface()
{

}

void dimeClientInterface::init()
{
	auto context = zmqContextManager::getContextPointer();

	char buffer[10];

	socket = std::make_unique<zmq::socket_t>(context->getBaseContext(),zmq::socket_type::req);
	socket->connect(address);
		
	Json::Value outgoing;
	outgoing["command"] = "connect";
	outgoing["name"] = "griddyn";
	outgoing["listen_to_events"] = false;
		
	Json::FastWriter fw;

	std::string out = fw.write(outgoing);
	socket->send(out.c_str(), out.size());

	auto sz=socket->recv(buffer, 10, 0);
	if ((sz != 2) || (buffer[0] != 'O') || (buffer[1] != 'K'))
	{
		throw initFailure();
	}
	
}
	
void dimeClientInterface::close()
{
	if (socket)
	{
		Json::Value outgoing;
		outgoing["command"] = "exit";
		outgoing["name"] = name;

		Json::FastWriter fw;

		std::string out = fw.write(outgoing);
		socket->send(out.c_str(), out.size());

		socket->close();
	}
	socket = nullptr;
}
	
void dimeClientInterface::sync()
{

}
	

void encodeVariableMessage(Json::Value &data, Json::Value Varvgs,double t)
{
	Json::Value wr;
	wr["k"] = "";
	wr["t"] = t;
	wr["vars"] = Varvgs;
	wr["accurate"] = "";
	Json::FastWriter fw;
	Json::Value content;
	content["stdout"] = "";
	content["figures"] = "";
	content["datadir"] = "/tmp MatlabData/";
	
	Json::Value response;
	response["content"] = content;
	response["result"] = wr;
	response["success"] = true;
	data["args"] =fw.write(response);	
}
void encodeVarheader(Json::Value &data, Json::Value Varheader)
{
	Json::FastWriter fw;
	Json::Value content;
	content["stdout"] = "";
	content["figures"] = "";
	content["datadir"] = "/tmp MatlabData/";

	Json::Value response;
	response["content"] = content;
	response["result"] = Varheader;
	response["success"] = true;
	data["args"] = fw.write(response);
}
void encodesysname(Json::Value &data, Json::Value sysname)
{
	Json::Value re;
	re["Bus"] = sysname;
	Json::FastWriter fw;
	Json::Value content;
	content["stdout"] = "";
	content["figures"] = "";
	content["datadir"] = "/tmp MatlabData/";

	Json::Value response;
	response["content"] = content;
	response["result"] = re;
	response["success"] = true;
	data["args"] = fw.write(response);
}
void encodesysparam(Json::Value &data, Json::Value BUSd, Json::Value PQd, Json::Value PVd, Json::Value lined,int nbus,int nline, Json::Value Genroud, Json::Value Fsd, Json::Value Swd)
{



	Json::Value re1;
	re1["Bus"] = BUSd;
	re1["PQ"] = PQd;
	re1["PV"] = PVd;
	re1["line"] = lined;
	re1["nbus"] = nbus;
	re1["nline"] = nline;
	re1["syn"] = Genroud;
	re1["Fixshunt"] = Fsd;
	re1["Sw"] = Swd;

	Json::FastWriter fw;
	Json::Value content;
	content["stdout"] = "";
	content["figures"] = "";
	content["datadir"] = "/tmp MatlabData/";

	Json::Value response;
	response["content"] = content;
	response["result"] = re1;
	response["success"] = true;
	data["args"] = fw.write(response);
}
void dimeClientInterface::send_var(double t,Json::Value Varvgs, const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;
Json::FastWriter fw;
    outgoing["command"] = (recipient.empty())?"broadcast":"send";

	outgoing["name"] = "griddyn";
	outgoing["args"] = "";
	

	std::string out = fw.write(outgoing);

	socket->send(out.c_str(), out.size());

	auto sz = socket->recv(buffer, 10, 0);

	Json::Value outgoingData;
	outgoingData["command"] = "response";
	outgoingData["name"] = "griddyn";
	if (!recipient.empty())
	{
		outgoingData["meta"]["recipient_name"] = "SE";
	}
	
	outgoingData["meta"]["var_name"] = "Varvgs";
	encodeVariableMessage(outgoingData,Varvgs,t);

	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}

}
void dimeClientInterface::send_varname(Json::Value Varheader,  const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;

	outgoing["command"] = (recipient.empty()) ? "broadcast" : "send";

	outgoing["name"] = "griddyn";
	outgoing["args"] = "";
	Json::FastWriter fw;

	std::string out = fw.write(outgoing);

	socket->send(out.c_str(), out.size());

	auto sz = socket->recv(buffer, 10, 0);

	Json::Value outgoingData;
	outgoingData["command"] = "response";
	outgoingData["name"] = "griddyn";
	if (!recipient.empty())
	{
		outgoingData["meta"]["recipient_name"] = "SE";
	}

	outgoingData["meta"]["var_name"] = "Varheader";
	encodeVarheader(outgoingData, Varheader);

	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}

}
void dimeClientInterface::send_sysname(Json::Value Sysname,  const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;

	outgoing["command"] = (recipient.empty()) ? "broadcast" : "send";

	outgoing["name"] = "griddyn";
	outgoing["args"] = "";
	Json::FastWriter fw;

	std::string out = fw.write(outgoing);

	socket->send(out.c_str(), out.size());

	auto sz = socket->recv(buffer, 10, 0);

	Json::Value outgoingData;
	outgoingData["command"] = "response";
	outgoingData["name"] = "griddyn";
	if (!recipient.empty())
	{
		outgoingData["meta"]["recipient_name"] = "SE";
	}

	outgoingData["meta"]["var_name"] = "Sysname";
	encodesysname(outgoingData, Sysname);

	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}

}
void dimeClientInterface::send_sysparam(Json::Value BUSd, Json::Value PQd, Json::Value PVd, Json::Value lined,int nbus,int nline, Json::Value Genroud, Json::Value Fsd, Json::Value Swd, const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;

	outgoing["command"] = (recipient.empty()) ? "broadcast" : "send";

	outgoing["name"] = "griddyn";
	outgoing["args"] = "";
	Json::FastWriter fw;

	std::string out = fw.write(outgoing);

	socket->send(out.c_str(), out.size());

	auto sz = socket->recv(buffer, 10, 0);

	Json::Value outgoingData;
	outgoingData["command"] = "response";
	outgoingData["name"] = "griddyn";
	if (!recipient.empty())
	{
		outgoingData["meta"]["recipient_name"] = "SE";
	}

	outgoingData["meta"]["var_name"] = "Sysparam";
	encodesysparam(outgoingData, BUSd,PQd,PVd,lined,nbus,nline,Genroud,Fsd,Swd);

	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}

}
void dimeClientInterface::get_devices()
{

}