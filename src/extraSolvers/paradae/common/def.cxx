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
#include "def.h"

#include "../math/IVander.h"
#include "../math/IVanderExt.h"
#include "../math/SVector.h"
#include <string>
namespace griddyn {
namespace paradae {

    using namespace std;

    const Real pi = 3.14159265358979323846;
    SVector dummy;

#ifdef VEXT
    const IVanderExt iVan_2 = IVanderExt(2);
    const IVanderExt iVan_3 = IVanderExt(3);
    const IVanderExt iVan_4 = IVanderExt(4);
    const IVanderExt iVan_5 = IVanderExt(5);
    const IVanderExt iVan_6 = IVanderExt(6);
#else
    const IVander iVan_2 = IVander(2);
    const IVander iVan_3 = IVander(3);
    const IVander iVan_4 = IVander(4);
    const IVander iVan_5 = IVander(5);
    const IVander iVan_6 = IVander(6);
#endif
    int debug_int = 0;

    string& SSS_debug(const char* s) { return *(new string(s)); }

#ifdef CHECK_MEM_ALLOC
    size_t total_mem = 0;
    size_t current_mem = 0;
    size_t max_mem = 0;

    void* operator new(size_t size)
    {
        current_mem += size;
        total_mem += size;
        max_mem = (max_mem < current_mem) ? current_mem : max_mem;
        size_t* p = (size_t*)malloc(size + sizeof(size_t));
        if (!p) throw bad_alloc();
        p[0] = size;
        return (void*)(&p[1]);
    }

    void* operator new[](size_t size)
    {
        current_mem += size;
        total_mem += size;
        max_mem = (max_mem < current_mem) ? current_mem : max_mem;
        size_t* p = (size_t*)malloc(size + sizeof(size_t));
        if (!p) throw bad_alloc();
        p[0] = size;
        return (void*)(&p[1]);
    }

    void* operator new(size_t size, const nothrow_t&)
    {
        current_mem += size;
        total_mem += size;
        max_mem = (max_mem < current_mem) ? current_mem : max_mem;
        size_t* p = (size_t*)malloc(size + sizeof(size_t));
        p[0] = size;
        return (void*)(&p[1]);
    }

    void* operator new[](size_t size, const nothrow_t&)
    {
        current_mem += size;
        total_mem += size;
        max_mem = (max_mem < current_mem) ? current_mem : max_mem;
        size_t* p = (size_t*)malloc(size + sizeof(size_t));
        p[0] = size;
        return (void*)(&p[1]);
    }

    void operator delete(void* ptr) noexcept
    {
        size_t* p = (size_t*)ptr;
        current_mem -= p[-1];
        free((void*)(&p[-1]));
    }

    void operator delete[](void* ptr) noexcept
    {
        size_t* p = (size_t*)ptr;
        current_mem -= p[-1];
        free((void*)(&p[-1]));
    }

    void operator delete(void* ptr, const nothrow_t&)
    {
        size_t* p = (size_t*)ptr;
        current_mem -= p[-1];
        free((void*)(&p[-1]));
    }

    void operator delete[](void* ptr, const nothrow_t&)
    {
        size_t* p = (size_t*)ptr;
        current_mem -= p[-1];
        free((void*)(&p[-1]));
    }
#endif

}  // namespace paradae
}  // namespace griddyn
