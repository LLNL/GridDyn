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

#include "dimeCollector.h"
#include "dimeClientInterface.h"
#include "core/helperTemplates.h"
#include <string>
using namespace std;
dimeCollector::dimeCollector(coreTime time0, coreTime period):collector(time0,period)
{

}

dimeCollector::dimeCollector(const std::string &collectorName):collector(collectorName)
{

}

dimeCollector::~dimeCollector()
{
	if (dime)
	{
		dime->close();
	}
}

std::shared_ptr<collector> dimeCollector::clone(std::shared_ptr<collector> gr) const
{
	auto nrec = cloneBase<dimeCollector, collector>(this, gr);
	if (!nrec)
	{
		return gr;
	}

	nrec->server = server;
	nrec->processName = processName;

	return nrec;
}

change_code dimeCollector::trigger(coreTime time)
{
	double t = time;
	Json::Value Varvgs;
	Json::Value Varheader;
	if (!dime)
	{
		dime = std::make_unique<dimeClientInterface>(processName, server);
		dime->init();
	}
	auto out=collector::trigger(time);
	//figure out what to do with the data
	std::string strv = "V";
	std::vector<double> voltagec;
	std::vector<std::string> voltagename;

	std::string strang = "theta";
	std::vector<double> anglec;
	std::vector<std::string> anglename;

	std::string strfreq = "freq";
	std::vector<double> freqc;
	std::vector<std::string> freqname;

	std::string strreactive = "gen_reactive";
	std::vector<double> reactivec;
	std::vector<std::string> reactivename;

	std::string strreal = "gen_real";
	std::vector<double> realc;
	std::vector<std::string> realname;

	std::string strloadreac = "load_reactive";
	std::vector<double> loadreacc;
	std::vector<std::string> loadreacname;

	std::string strloadreal = "load_real";
	std::vector<double> loadrealc;
	std::vector<std::string> loadrealname;

	std::string stromega = "omega";
	std::vector<double> omegac;
	std::vector<std::string> omeganame;

	std::string strpf = "pf_";
	std::vector<double> pfc;
	std::vector<std::string> pfname;

	std::string stri = "I";
	std::vector<double> Ic;
	std::vector<std::string> Iname;

	std::string strlp = "line_p";
	std::vector<double> linepc;
	std::vector<std::string> linepname;

	std::string strlq = "line_q";
	std::vector<double> lineqc;
	std::vector<std::string> lineqname;

	std::string strexc = "exc_syn";
	std::vector<double> excc;
	std::vector<std::string> excname;

	std::string str1d = "e1d_syn";
	std::vector<double> e1dc;
	std::vector<std::string> e1dname;

	std::string str2d = "e2d_syn";
	std::vector<double> e2dc;
	std::vector<std::string> e2dname;

	std::string str1q = "e1q_syn";
	std::vector<double> e1qc;
	std::vector<std::string> e1qname;

	std::string str2q = "e2q_syn";
	std::vector<double> e2qc;
	std::vector<std::string> e2qname;


	for (size_t kk = 0; kk < points.size(); ++kk)
	{
		

		


		if (points[kk].colname.find(strv) != string::npos)
		{
			voltagec.push_back(data[kk]);
			voltagename.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strlp) != string::npos)
		{
			linepc.push_back(data[kk]);
			linepname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strang) != string::npos)
		{
			anglec.push_back(data[kk]);
			anglename.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strfreq) != string::npos)
		{
			freqc.push_back(data[kk]);
			freqname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strreactive) != string::npos)
		{
			reactivec.push_back(data[kk]);
			reactivename.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strreal) != string::npos)
		{
		    realc.push_back(data[kk]);
			realname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strloadreac) != string::npos)
		{
			loadreacc.push_back(data[kk]);
			loadreacname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strloadreal) != string::npos)
		{
			loadrealc.push_back(data[kk]);
			loadrealname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(stromega) != string::npos)
		{
			omegac.push_back(data[kk]);
			omeganame.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strpf) != string::npos)
		{
			pfc.push_back(data[kk]);
			pfname.push_back(points[kk].colname);
		}
		if (points[kk].colname.find(stri) != string::npos)
		{
			Ic.push_back(data[kk]);
			Iname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strlq) != string::npos)
		{
			lineqc.push_back(data[kk]);
			lineqname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(strexc) != string::npos)
		{
			excc.push_back(data[kk]);
			excname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(str1d) != string::npos)
		{
			e1dc.push_back(data[kk]);
			e1dname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(str2d) != string::npos)
		{
			e2dc.push_back(data[kk]);
			e2dname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(str1q) != string::npos)
		{
			e1qc.push_back(data[kk]);
			e1qname.push_back(points[kk].colname);
		}

		if (points[kk].colname.find(str2q) != string::npos)
		{
			e2qc.push_back(data[kk]);
			e2qname.push_back(points[kk].colname);
		}
	}
	std::vector<double> total;
	std::vector<std::string> totalname;
	total.insert(total.end(), voltagec.begin(), voltagec.end());
	total.insert(total.end(), linepc.begin(), linepc.end());
	total.insert(total.end(), freqc.begin(), freqc.end());
	total.insert(total.end(), anglec.begin(), anglec.end());
	total.insert(total.end(), reactivec.begin(), reactivec.end());
	total.insert(total.end(), realc.begin(), realc.end());
	total.insert(total.end(), loadreacc.begin(), loadreacc.end());
	total.insert(total.end(), loadrealc.begin(), loadrealc.end());
	total.insert(total.end(), omegac.begin(), omegac.end());
	total.insert(total.end(), pfc.begin(), pfc.end());
	total.insert(total.end(), Ic.begin(), Ic.end());
	total.insert(total.end(), lineqc.begin(), lineqc.end());
	total.insert(total.end(), excc.begin(), excc.end());
	total.insert(total.end(), e1dc.begin(), e1dc.end());
	total.insert(total.end(), e2dc.begin(), e2dc.end());
	total.insert(total.end(), e1qc.begin(), e1qc.end());
	total.insert(total.end(), e2qc.begin(), e2qc.end());
	
	totalname.insert(totalname.end(), voltagename.begin(), voltagename.end());
	totalname.insert(totalname.end(), freqname.begin(), freqname.end());
	totalname.insert(totalname.end(), linepname.begin(), linepname.end());
	totalname.insert(totalname.end(), anglename.begin(), anglename.end());
	totalname.insert(totalname.end(), reactivename.begin(), reactivename.end());
	totalname.insert(totalname.end(), realname.begin(), realname.end());
	totalname.insert(totalname.end(), loadreacname.begin(), loadreacname.end());
	totalname.insert(totalname.end(), loadrealname.begin(), loadrealname.end());
	totalname.insert(totalname.end(), omeganame.begin(), omeganame.end());
	totalname.insert(totalname.end(), pfname.begin(), pfname.end());
	totalname.insert(totalname.end(), Iname.begin(), Iname.end());
	totalname.insert(totalname.end(), lineqname.begin(), lineqname.end());
	totalname.insert(totalname.end(), excname.begin(), excname.end());
	totalname.insert(totalname.end(), e1dname.begin(), e1dname.end());
	totalname.insert(totalname.end(), e2dname.begin(), e2dname.end());
	totalname.insert(totalname.end(), e1qname.begin(), e1qname.end());
	totalname.insert(totalname.end(), e2qname.begin(), e2qname.end());
	
	for (size_t kk = 0; kk < points.size(); ++kk)
	{
		
		Json::Value wrvar;
		wrvar.append(total[kk]);
		Varvgs.append(wrvar);



		Json::Value wrvarname;
		Json::Value wagain;
		wrvarname.append(totalname[kk]);
		wagain.append(wrvarname);
		Varheader.append(wagain);
		
	}


	dime->send_var(t,Varvgs,"SE");
	dime->send_varname(Varheader,"SE");
	return out;
}



void dimeCollector::send_sysname(std::vector<std::string> sysname)
{

	std::unique_ptr<dimeClientInterface> dime=std::make_unique<dimeClientInterface>("", "");
	dime->init();

  Json::Value Sysname;
  for (size_t kk = 0; kk < sysname.size(); ++kk)
  {
   Json::Value wsysname;
   Json::Value wagain;
   wsysname.append(sysname[kk]);
   wagain.append(wsysname);
   Sysname.append(wagain);
  }
  dime->send_sysname(Sysname,"SE");
}

void dimeCollector::set(const std::string &param, double val)
{
	
	collector::set(param, val);
}

void dimeCollector::set(const std::string &param, const std::string &val)
{
	if (param == "server")
	{
		server = val;
	}
	else if (param == "processname")
	{
		processName = val;
	}
	else
	{
		collector::set(param, val);
	}
	
}

const std::string &dimeCollector::getSinkName() const
{
	return server;
}