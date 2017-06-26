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
#include "base64.h"  
#include "dimeCollector.h"
#include "dimeClientInterface.h"
<<<<<<< HEAD:zmqInterface/dimeClientInterface.cpp
#include "zmqContextManager.h"
#include <stdio.h>


=======
#include "zmqLibrary/zmqContextManager.h"
>>>>>>> 935e202b8aa221f7f91286dd2837674c4aa82028:src/zmqlib/dimeClientInterface.cpp

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

static std::vector<std::string> param;
std::vector<std::vector<double>> idxreq;
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

dimeClientInterface::~dimeClientInterface() = default;


void dimeClientInterface::init()
{
	auto context = zmqlib::zmqContextManager::getContextPointer();

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


	
std::string dimeClientInterface::sync()
{
	
	std::string devname;
	int flg=1;
	while (flg)
	{
		
		char buffer[100000];
		Json::Value outgoing;
		outgoing["command"] = "sync";
		outgoing["name"] = "griddyn";
		outgoing["args"] = ' ';

		Json::FastWriter fw;

		std::string out = fw.write(outgoing);
		socket->send(out.c_str(), out.size());
		auto sz = socket->recv(buffer, 100000, 0);
		if ((sz != 2) || (buffer[0] != 'O') || (buffer[1] != 'K'))
		{
			std::string req(buffer);
			Json::Value request;
			Json::Reader readreq;
			readreq.parse(req, request);

			devname = request["func_args"][1].asString();

			std::vector<std::string>().swap(param);
			for (int ii = 0; ii < request["func_args"][2]["param"].size(); ++ii)
			{
				param.push_back(request["func_args"][2]["param"][ii].asString());
			}
			std::vector<double> idxreqinter;
			std::string vgsidx = request["func_args"][2]["vgsvaridx"]["data"].asString();
			std::string &v = vgsidx;
			std::vector<uint8_t> xx = base64_decode(v);
			const int s = xx.size();

			for (int ii = 0; ii < xx.size() / 8; ++ii)
				idxreqinter.push_back(0);
			int size = xx.size() * 8;
			int k = 0;

			for (int ii = 0; ii < xx.size() / 8; ++ii)
			{
				uint8_t *b = &xx[ii * 8];
				memcpy(&idxreqinter[k], b, sizeof(b));
				++k;
			}
			
			idxreq.push_back(idxreqinter);
			flg = 0;
			std::cout << "request for" << devname << " is received" << std::endl;
		}
		if (flg == 1)
		{
			std::cout << "no clients sending request to griddyn" << std::endl;
			std::cout << "sleep 2 sceconds then keep receiving" << std::endl;
			Sleep(2000);
		}

	}

	



	return devname;
}
	



std::vector<std::string> dimeClientInterface::get_devices()
{
re0:
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
	std::cout << finallist+" are connected with server" << std::endl;
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
			tempc = finallist.substr(0, nu);
			if (tempc == "griddyn")
			{
			   break;
			}
			dev_list.push_back(finallist);

			break;
		}
		
	}
	if (dev_list.empty())
	{
		std::cout << "no client is connected" << std::endl;
		std::cout << "sleep 2 second then keep calling" << std::endl;
		Sleep(2000);
		goto re0;	
	}

	return dev_list;


}





void encodereqVariableMessage(Json::Value &data, Json::Value reqvar,Json::Value reqvarheader, double t)
{
	Json::Value wr;
	wr["k"] = "";
	wr["t"] = t;
	wr["vars"] = reqvar;
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
	data["args"] = fw.write(response);
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
	for (int ii = 0; ii < param.size(); ++ii)
	{
		if (param[ii] == "Bus")
		{
			re1["Bus"] = BUSd;
		}
		if (param[ii] == "PQ")
		{
			re1["PQ"] = PQd;
		}
		if (param[ii] == "PV")
		{
			re1["PV"] = PVd;
		}
		if (param[ii] == "Line")
		{
			re1["line"] = lined;
		}
		if (param[ii] == "nbus")
		{
			re1["nbus"] = nbus;
		}
		if (param[ii] == "nline")
		{
			re1["nline"] = nline;
		}
		if (param[ii] == "Syn")
		{
			re1["syn"] = Genroud;
		}
		if (param[ii] == "Fixshunt")
		{
			re1["Fixshunt"] = Fsd;
		}
		if (param[ii] == "Sw")
		{
			re1["Sw"] = Swd;
		}
	}

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
		outgoingData["meta"]["recipient_name"] = recipient;
	}

	outgoingData["meta"]["var_name"] = "SysParam";
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
		outgoingData["meta"]["recipient_name"] = recipient;
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
	

<<<<<<< HEAD:zmqInterface/dimeClientInterface.cpp



}
void dimeClientInterface::send_reqvar(double t,Json::Value reqvar,Json::Value reqvarheader, const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;
	Json::FastWriter fw;
	outgoing["command"] = (recipient.empty()) ? "broadcast" : "send";

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
		outgoingData["meta"]["recipient_name"] = recipient;
	}

	outgoingData["meta"]["var_name"] = "Varvgs";
	encodereqVariableMessage(outgoingData,reqvar,reqvarheader, t);

=======
>>>>>>> 935e202b8aa221f7f91286dd2837674c4aa82028:src/zmqlib/dimeClientInterface.cpp
	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}


}


//send all var
/*
void encodeVariableMessage(Json::Value &data, Json::Value Varvgs, double t)
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
	data["args"] = fw.write(response);
}
void dimeClientInterface::send_var(double t, Json::Value Varvgs, const std::string &recipient)
{
	//outgoing = { 'command': 'send', 'name' : self.name, 'args' : var_name }
	char buffer[10];

	Json::Value outgoing;
	Json::FastWriter fw;
	outgoing["command"] = (recipient.empty()) ? "broadcast" : "send";

	outgoing["name"] = "griddyn";
	outgoing["args"] = "";


	std::string out = fw.write(outgoing);

	socket->send(out.c_str(), out.size());

<<<<<<< HEAD:zmqInterface/dimeClientInterface.cpp
	auto sz = socket->recv(buffer, 10, 0);

	Json::Value outgoingData;
	outgoingData["command"] = "response";
	outgoingData["name"] = "griddyn";
	if (!recipient.empty())
	{
		outgoingData["meta"]["recipient_name"] = "SE";
	}

	outgoingData["meta"]["var_name"] = "Varvgs";
	encodeVariableMessage(outgoingData, Varvgs, t);

	out = fw.write(outgoingData);

	socket->send(out.c_str(), out.size());
	sz = socket->recv(buffer, 10, 0);
	if (sz != 2)
	{
		throw(sendFailure());
	}

}
*/
=======
}
>>>>>>> 935e202b8aa221f7f91286dd2837674c4aa82028:src/zmqlib/dimeClientInterface.cpp
