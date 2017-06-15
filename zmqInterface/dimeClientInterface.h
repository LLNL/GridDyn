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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "formatInterpreters/json/json.h"
#pragma warning(pop)
#else
#include "formatInterpreters/json/json.h"
#endif

#ifndef DIME_CLIENT_INTERFACE_HEADER_
#define DIME_CLIENT_INTERFACE_HEADER_
#include <cppzmq/zmq_addon.hpp>
#include <string>
#include <exception>
#include <memory>

class initFailure :public std::exception
{
public:
		initFailure() {};
};

class sendFailure :public std::exception
{
public:
	sendFailure() {};
};

class dimeClientInterface
{
private:
	std::string name;
	std::string address;
public:
	dimeClientInterface(const std::string &name, const std::string &address = "");

	~dimeClientInterface();
	/** initialize the connection*/
	void init();
	/** close the connection*/
	void close();
	/** sync with the server*/
	void sync();
	void send_var(double t,Json::Value Varvgs, const std::string & recipient);
	void send_varname(Json::Value Varheader, const std::string & recipient);
	void send_sysname(Json::Value Sysname, const std::string & recipient);
	void send_sysparam(Json::Value Busd, Json::Value PQd, Json::Value PVd, Json::Value lined,int nbus,int nline, Json::Value Genroud, Json::Value Fsd, Json::Value Swd, const std::string & recipient);
	/** send a variable to server*/
	void send_Idxvgs(Json::Value nbusvolk, Json::Value nlinepk, Json::Value nbusfreqk, Json::Value nbusthetak, Json::Value nbusgenreactivek, Json::Value nbusgenrealk, Json::Value nbusloadreactivelk, Json::Value nbusloadrealk, Json::Value nsynomegaj, Json::Value nsyndeltaj, Json::Value nlineij, Json::Value nlineqj, Json::Value nexc, Json::Value ne1d, Json::Value ne2d, Json::Value ne1q, Json::Value ne2q, const std::string &recipient);


	void get_devices();
private:
	std::unique_ptr<zmq::socket_t> socket;
};
#endif