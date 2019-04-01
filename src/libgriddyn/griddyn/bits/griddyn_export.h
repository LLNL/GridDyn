#ifndef LIBGRIDDYN_BITS_GRIDDYN_EXPORT_H
#define LIBGRIDDYN_BITS_GRIDDYN_EXPORT_H

#if defined(_MSC_VER)
    #define LIBGRIDDYN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
    #define LIBGRIDDYN_EXPORT __attribute__((visibility("default")))
#else
    #error "Please fill in LIBGRIDDYN_EXPORT for your compiler in bits/griddyn_export.h"
#endif

#endif // LIBGRIDDYN_BITS_GRIDDYN_EXPORT_H
