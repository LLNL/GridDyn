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

#include "../common/def.h"
#include "../math/SVector.h"
#include "mpi.h"
#include <list>
#include <map>

namespace griddyn {
namespace paradae {
    enum NSType { sequential, braid };

    class NewtonStats {
        int iter;
        int level;
        int step;
        int max_iter;
        int max_level;
        MPI_Comm comm;
        Real GlobalStats_Seq();
        Real GlobalStats_Braid();

      public:
        NSType type;
        std::map<int, std::list<int>> map1;
        std::map<std::pair<int, int>, std::list<int>> map2;
        NewtonStats(): iter(0), level(0), step(0), max_iter(0), max_level(0), type(sequential){};
        NewtonStats(NSType t): iter(0), level(0), step(0), max_iter(0), max_level(0), type(t){};
        void SetMPIComm(MPI_Comm comm_) { comm = comm_; };
        void SetType(NSType t) { type = t; };
        void SetNextStep() { step++; };
        //void SetIL(int i, int lvl){iter=i;level=lvl;max_iter=(i<max_iter)?i:max_iter;max_level=(lvl<max_level)?lvl:max_level;};
        void SetIL(int i, int lvl);
        void Add(int nbiter);
        Real GlobalStats();
        void PerStepStats(SVector& min_, SVector& avg_, SVector& max_);
        void PerIterStats(SVector& min_, SVector& avg_, SVector& max_, SVector& nbcalls);
        void PerLevelStats(SVector& min_, SVector& avg_, SVector& max_, SVector& nbcalls);
        void ShowStats();
    };

    extern NewtonStats newton_stats;
}  // namespace paradae
}  // namespace griddyn
