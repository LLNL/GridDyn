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

#include "dimeCollector.h"
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
	
std::vector<std::string> dimeClientInterface::get_devices()
{
	std::vector<std::string> dev_list;
	char buffer[100];
	Json::Value outgoing;
	outgoing["command"] = "get_devices";
	outgoing["name"] = "griddyn";

	Json::FastWriter fw;

	std::string out = fw.write(outgoing);
	socket->send(out.c_str(), out.size());

	socket->recv(buffer, 100, 0);
	std::string devlist(buffer);
	int nu = devlist.find_last_of('}');
	std::string tempc = devlist.substr(0, nu);

	Json::Reader re;
	Json::Value devlistj;
	re.parse(tempc, devlistj);
	std::string finallist = fw.write(devlistj["response"]);
	std::cout << finallist << std::endl;

	while (1)
	{
		try
		{
			finallist.replace(finallist.find("["), 1, "");
			finallist.replace(finallist.find("]"), 1, "");

		}
		catch (const std::exception&)
		{
		}

		try
		{
			finallist.replace(finallist.find("\""), 1, "");

		}
		catch (const std::exception&)
		{
			finallist.replace(finallist.find("\n"), 1, "");
			break;
		}
	}

	while (1)
	{

		nu = finallist.find_first_of(',');
		if (nu != -1)
		{
			tempc = finallist.substr(0, nu);
			if (tempc=="griddyn")
			{
				finallist = finallist.substr(nu + 1, finallist.size());
				continue;
			}
			dev_list.push_back(tempc);
			finallist = finallist.substr(nu + 1, finallist.size());
		}
		else
		{
			dev_list.push_back(finallist);
			break;
		}

	}



	return dev_list;


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
void encodeIdxvgs(Json::Value &data, Json::Value nbusvolk, Json::Value nlinepk, Json::Value nbusfreqk, Json::Value nbusthetak, Json::Value nbusgenreactivek, Json::Value nbusgenrealk, Json::Value nbusloadreactivelk, Json::Value nbusloadrealk, Json::Value nsynomegaj, Json::Value nsyndeltaj, Json::Value nlineij, Json::Value nlineqj, Json::Value nexc, Json::Value ne1d, Json::Value ne2d, Json::Value ne1q, Json::Value ne2q)
{
	Json::Value wr;

	Json::Value exc;
	Json::Value syni;
	Json::Value line;
	Json::Value bus;

	bus["V"] = nbusvolk;
	bus["freq"] = nbusfreqk;
	bus["loadp"] = nbusloadrealk;
	bus["loadq"] = nbusloadreactivelk;
	bus["genp"] = nbusgenrealk;
	bus["genq"] = nbusgenreactivek;

	line["I"] = nlineij;
	line["p"] = nlinepk;
	line["q"] = nlineqj;

	syni["e1d"] = ne1d;
	syni["e2d"] = ne2d;
	syni["e1q"] = ne1q;
	syni["e2q"] = ne2q;
	syni["delta"] = nsyndeltaj;
	syni["omega"] = nsynomegaj;

	exc["vm"] = nexc;

	wr["Syn"] = syni;
	wr["Line"] = line;
	wr["Bus"] = bus;
	wr["exc"] = exc;


	Json::FastWriter fw;
	Json::Value content;
	content["stdout"] = "";
	content["figures"] = "";
	content["datadir"] = "/tmp MatlabData/";

	Json::Value response;
	response["content"] = content;
	response["result"] = wr;
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
void dimeClientInterface::send_Idxvgs(Json::Value nbusvolk, Json::Value nlinepk, Json::Value nbusfreqk, Json::Value nbusthetak, Json::Value nbusgenreactivek, Json::Value nbusgenrealk, Json::Value nbusloadreactivelk, Json::Value nbusloadrealk, Json::Value nsynomegaj, Json::Value nsyndeltaj, Json::Value nlineij, Json::Value nlineqj, Json::Value nexc, Json::Value ne1d, Json::Value ne2d, Json::Value ne1q, Json::Value ne2q, const std::string &recipient)
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

	outgoingData["meta"]["var_name"] = "Idxvgs";
	encodeIdxvgs(outgoingData, nbusvolk, nlinepk, nbusfreqk, nbusthetak, nbusgenreactivek, nbusgenrealk, nbusloadreactivelk, nbusloadrealk, nsynomegaj, nsyndeltaj, nlineij, nlineqj, nexc, ne1d, ne2d, ne1q, ne2q);
	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}
	
	Sleep(1000);

	std::vector<std::string> dev_list = get_devices();



}

