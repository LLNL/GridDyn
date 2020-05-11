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

#include <cstdlib>
#include <string>

namespace griddyn {
namespace paradae {
#define REAL_ 2

#if REAL_ == 1
    typedef float Real; /*!< Define the type Real */
#else
    typedef double Real; /*!< Define the type Real */
#endif

    extern const Real pi;
    class SVector;
    extern SVector dummy;

#define VEXT
#ifdef VEXT
    class IVanderExt;
    extern const IVanderExt iVan_2;
    extern const IVanderExt iVan_3;
    extern const IVanderExt iVan_4;
    extern const IVanderExt iVan_5;
    extern const IVanderExt iVan_6;
#else
    class IVander;
    extern const IVander iVan_2;
    extern const IVander iVan_3;
    extern const IVander iVan_4;
    extern const IVander iVan_5;
    extern const IVander iVan_6;
#endif
    extern int debug_int;
    std::string& SSS_debug(const char* s);

#ifdef CHECK_MEM_ALLOC
    extern size_t total_mem;
    extern size_t current_mem;
    extern size_t max_mem;

    void* operator new(std::size_t size);
    void* operator new[](std::size_t size);
    void* operator new(std::size_t size, const std::nothrow_t&);
    void* operator new[](std::size_t size, const std::nothrow_t&);
    void operator delete(void* ptr) noexcept;
    void operator delete[](void* ptr) noexcept;
    void operator delete(void* ptr, const std::nothrow_t&);
    void operator delete[](void* ptr, const std::nothrow_t&);

#endif
}  // namespace paradae
}  // namespace griddyn
