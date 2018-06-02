/*
* LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "stackInfo.h"

#ifdef __GNUC__
#ifndef _WIN32
#include <cxxabi.h>
#include <execinfo.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <memory>

template<typename T>
inline bool not_null(T* ptr) {
    return ptr != nullptr;
}

void printStackTrace (std::ostream &out, size_t max_frames)
{
    std::stringstream msg;
    msg << "stack trace:\n";
    msg << std::right; // all formatting is aligned right

    // storage array for stack trace address data
    std::vector<void *> addrlist (max_frames + 1);

    // retrieve current stack addresses
    size_t addrlen = backtrace (addrlist.data (), max_frames + 1);

    if (addrlen == 0)
    {
        msg << "  \n";
        return;
    }

    // resolve addresses into strings containing "fileName(function+address)",
    // Actually it will be ## program address function + offset
    // this array must be free()-ed
    auto symbollist = std::unique_ptr<char *, decltype(free)*> { backtrace_symbols (addrlist.data (), addrlen), free };

    size_t funcnamesize = 1024;
    char funcname[1024];

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (size_t i = 4; i < addrlen; i++)
    {
        char *begin_name = nullptr;
        char *begin_offset = nullptr;
        char *end_offset = nullptr;

// find parentheses and +address offset surrounding the mangled name
#ifdef DARWIN
        // OSX style stack trace
        for (char *p = symbollist.get()[i]; *p != 0; ++p)
        {
            if ((*p == '_') && (*(p - 1) == ' '))
            {
                begin_name = p - 1;
            }
            else if (*p == '+')
            {
                begin_offset = p - 1;
            }
        }

        if (not_null(begin_name) && not_null(begin_offset) && (begin_name < begin_offset))
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():
            int status;
            char *ret = abi::__cxa_demangle (begin_name, &funcname[0], &funcnamesize, &status);
            if (status == 0)
            {
                funcname = ret;  // use possibly realloc()-ed string
                //fprintf (out, "  %-30s %-40s %s\n", symbollist[i], funcname, begin_offset);
                msg << "  "
                    << std::setw(30) << symbollist.get()[i] << " "
                    << std::setw(40) << funcname            << " "
                    << std::setw(0)  << begin_offset        << "\n";
            }
            else
            {
                // demangling failed. Output function name as a C function with
                // no arguments.
                //fprintf (out, "  %-30s %-38s() %s\n", symbollist[i], begin_name, begin_offset);
                msg << "  " <<
                    << std::setw(30) << symbollist.get()[i] << ' '
                    << std::setw(38) << begin_name          << "() "
                    << std::setw(0)  << begin_offset        << "\n";

            }

#else  // !DARWIN - but is posix
        // not OSX style
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist.get()[i]; *p != 0; ++p)
        {
            if (*p == '(')
            {
                begin_name = p;
            }
            else if (*p == '+')
            {
                begin_offset = p;
            }
            else if (*p == ')' && (not_null(begin_offset) || not_null(begin_name)))
            {
                end_offset = p;
            }
        }

        if (not_null(begin_name) && not_null(end_offset) && (begin_name < end_offset))
        {
            *begin_name++ = '\0';
            *end_offset++ = '\0';
            if (not_null(begin_offset))
            {
                *begin_offset++ = '\0';
            }

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status = 0;
            char *ret = abi::__cxa_demangle (begin_name, &funcname[0], &funcnamesize, &status);
            char *fname = begin_name;
            if (status == 0)
            {
                fname = ret;
            }

            if (not_null(begin_offset))
            {
                //fprintf (out, "  %-30s ( %-40s  + %-6s) %s\n", symbollist[i], fname, begin_offset, end_offset);
                msg << "  "
                    << std::setw(30) << symbollist.get()[i] << " ( "
                    << std::setw(40) << fname               << "  + "
                    << std::setw(6)  << begin_offset        << ") "
                    << std::setw(0)  << end_offset          << "\n";
            }
            else
            {
                //fprintf (out, "  %-30s ( %-40s    %-6s) %s\n", symbollist[i], fname, "", end_offset);
                msg << "  "
                    << std::setw(30) << symbollist.get()[i] << " ( "
                    << std::setw(40) << fname               << "    "
                    << std::setw(6)  << ""                  << ") "
                    << std::setw(0)  << end_offset          << "\n";
            }
#endif  // !DARWIN - but is posix
        }
        else
        {
            // couldn't parse the line? print the whole line.
            //fprintf (out, "  %-40s\n", symbollist[i]);
            msg << "  "
                << std::setw(40) << symbollist.get()[i] << "\n";
        }
    }
    out << msg.str();
}
#else
void printStackTrace (std::ofstream /*out*/, size_t /*max_frames*/) {}

#endif
#else
void printStackTrace (std::ofstream /*out*/, size_t /*max_frames*/) {}

#endif
