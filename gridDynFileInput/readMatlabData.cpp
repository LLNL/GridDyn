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


#include "gridDynFileInput.h"
#include "readerHelper.h"


#include "stringOps.h"
#include <fstream>
#include <iostream>
#include <sstream>

void loadMFile(gridCoreObject *parentObject, const std::string &filename, const basicReaderInfo &bri)
{
	std::ifstream infile(filename.c_str(), std::ios::in);
	std::stringstream strStream;
	strStream << infile.rdbuf();
	std::string filetext = strStream.str();
	infile.close();
	if (filetext.empty())
	{
		std::cout << "Warning file " << filename << "is invalid or empty\n";
		return;
	}
	removeMatlabComments(filetext);
	size_t func = filetext.find("function");

	size_t mpc = filetext.find("mpc");
	if ((func != std::string::npos)||(mpc!=std::string::npos))
	{
		if (func == std::string::npos)
		{
			func = 0;
		}
		size_t A = filetext.find_first_of('=', func + 9);
		std::string bname = trim(filetext.substr(func + 9, A - func - 9));
		
		size_t B = filetext.find(bname + ".bus");
		if (B != std::string::npos)
		{
			loadMatPower(parentObject, filetext, bname, bri);
		}
		else
		{
			A = filetext.find("MatDyn");
			if (A != std::string::npos)
			{
				A = filetext.find("event");
				if (A != std::string::npos)
				{
					loadMatDynEvent(parentObject, filetext, bri);
				}
				else
				{
					loadMatDyn(parentObject, filetext, bri);
				}
			}
			else
			{
				A = filetext.find("exc");
				if (A != std::string::npos)
				{
					B = filetext.find("gov");
					if (B != std::string::npos)
					{
						loadMatDyn(parentObject, filetext, bri);
					}
					else
					{
						std::cout << "I don't know what this file is\n";
					}

				}
				else
				{
					A = filetext.find("event");
					if (A != std::string::npos)
					{
						loadMatDynEvent(parentObject, filetext, bri);
					}
					else
					{
						std::cout << "I don't know what this file is\n";
					}
				}
			}
		}
	}
	else
	{
		size_t A = filetext.find("Bus.con");         //look for the Psat bus configuration array
		if (A != std::string::npos)
		{
			loadPSAT(parentObject, filetext, bri);
		}
	}
}

void removeMatlabComments(std::string &text)
{
	size_t A = text.find_first_of('%');
	while (A != std::string::npos)
	{
		size_t B = text.find_first_of('\n', A);
		text.erase(A, B - A + 1);
		A = text.find_first_of('%');
	}
}

bool readMatlabArray(const std::string &Name, const std::string &text, mArray &matA)
{
	size_t A = text.find(Name);
	if (A != std::string::npos)
	{
		size_t B = text.find_first_of('=', A);
		readMatlabArray(text, B + 1, matA);
		return true;
	}
	return false;
}

void readMatlabArray(const std::string &text, size_t start, mArray &matA)
{
	size_t A = text.find_first_of('[', start);
	size_t B = text.find_first_of(']', A);
	std::string Adat = text.substr(A + 1, B - A);
	matA.resize(0);

	std::vector<double> M;
	size_t D = 0;
	size_t C = Adat.find_first_of("];");
	while (C != std::string::npos)
	{
		std::string line = Adat.substr(D, C - D);
		trimString(line);
		if (line.empty())
		{
			D = C + 1;
			C = Adat.find_first_of(";]", D);
			continue;
		}
		stringVec Tline = splitline(line, "\t\n ",delimiter_compression::on);
		M.resize(Tline.size());
		size_t offset = 0;
		for (size_t kk = 0; kk < Tline.size(); ++kk)
		{
			if (Tline[kk] == "...")
			{
				offset++;
				continue;
			}
			if (Tline[kk].empty())
			{
				offset++;
				continue;
			}
			M[kk - offset] = doubleRead(Tline[kk],0.0);
		}
		M.resize(Tline.size() - offset);
		matA.push_back(M);
		D = C + 1;
		C = Adat.find_first_of(";]", D);
	}
}

stringVec readMatlabCellArray(const std::string &text, size_t start)
{

	stringVec cell;

	size_t A = text.find_first_of('{', start);
	size_t B = text.find_first_of('}', A);
	std::string Adat = text.substr(A + 1, B - A);
	size_t C = Adat.find_first_of('\'', 0);
	while (C != std::string::npos)
	{
		size_t D = Adat.find_first_of(";,}", C + 1);
		if (D != std::string::npos)
		{
			auto line = Adat.substr(C, D - C);
			trimString(line);
			if (line[0] == '\'')
			{
				line = line.substr(1, line.size() - 2);
				trimString(line);
			}
			cell.push_back(line);
		}
		C = Adat.find_first_of('\'', D + 1);
	}
	return cell;
}