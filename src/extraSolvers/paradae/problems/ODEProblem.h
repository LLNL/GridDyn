/*
* LLNS Copyright Start
* Copyright (c) 2018, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#pragma once
#include "mpi.h"
#include "../common/def.h"
#include "../common/MapParam.h"
#include "../equations/Equation.h"
#include "../timeintegrators/TimeIntegrator.h"
#include <fstream>

enum ODE_error {ODE_FAILED};
namespace griddyn
{
namespace paradae
{
class ODEProblem
{
    Equation* equation;
    TimeIntegrator* TI;
    std::string output_filename;
    std::ofstream* output;

    MPI_Comm comm;
    int mpi_rank;
    bool print_solution;
    bool print_all_iter_solution;

public:
    ODEProblem(MPI_Comm comm_);
    ~ODEProblem();
    void SetEquation(const MapParam& param);
    void SetEquation(Equation* equation_, Real t0, Real Tmax, int Nsteps);
    void SetTimeIntegrator(const MapParam& param);
    void SetOutputFile(const MapParam& param, std::string file = "");
    inline int GetM() { return equation->GetM(); };
    inline Equation* GetEq() { return equation; };
    inline TimeIntegrator* GetTI() { return TI; };
    inline std::ofstream* GetOutput() { return output; };
    inline std::string GetOutputFilename() { return output_filename; };
    inline MPI_Comm GetComm() { return comm; };
    inline bool PrintSolution() { return print_solution; };
    inline bool PrintAllITERSolution() { return print_all_iter_solution; };
};
} //namespace paradae
} //namespace griddyn
