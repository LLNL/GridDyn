#ifndef LIBGRIDDYN_BITS_GRIDDYN_CONSTANTS_H
#define LIBGRIDDYN_BITS_GRIDDYN_CONSTANTS_H

#include <stdint.h> // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t  griddyn_flag_t;
typedef int       griddyn_result_t;
typedef int       griddyn_status_t;

#define GRIDDYN_AUTOALLOCATE                        0x00000001
#define GRIDDYN_POWER_ADJUST                        0x00000002
#define GRIDDYN_USE_SPARSE_SOLVER                   0x00000004
#define GRIDDYN_USE_DENSE_SOLVER                    0x00000008
#define GRIDDYN_NO_AUTO_AUTOGEN                     0x00000010
#define GRIDDYN_AUTO_BUS_DISCONNECT                 0x00000020
#define GRIDDYN_ROOTS_DISABLED                      0x00000040
#define GRIDDYN_VOLTAGECONSTRAINTS                  0x00000080
#define GRIDDYN_RECORD_ON_HALT                      0x00000100
#define GRIDDYN_CONSTRAINTS_DISABLED                0x00000200
#define GRIDDYN_DC_MODE                             0x00000400
#define GRIDDYN_DCFLOW_INITIALIZATION               0x00000800
#define GRIDDYN_NO_LINK_ADJUSTMENTS                 0x00001000
#define GRIDDYN_DISABLE_LINK_ADJUSTMENTS            0x00002000
#define GRIDDYN_IGNORE_BUS_LIMITS                   0x00004000
#define GRIDDYN_POWERFLOW_ONLY                      0x00008000
#define GRIDDYN_NO_POWERFLOW_ADJUSTMENTS            0x00010000
#define GRIDDYN_SAVEPOWERFLOW                       0x00020000
#define GRIDDYN_LOW_VOLTAGE_CHECK                   0x00040000
#define GRIDDYN_NO_POWERFLOW_ERROR_RECOVERY         0x00080000
#define GRIDDYN_DAE_INITIALIZATION_FOR_PARTITIONED  0x00100000
#define GRIDDYN_FORCE_POWERFLOW                     0x00200000
#define GRIDDYN_FORCE_EXTRA_POWERFLOW               0x00400000
#define GRIDDYN_DROOP_POWER_FLOW                    0x00800000
#define GRIDDYN_USE_THREADS                         0x01000000

#define GRIDDYN_RESULT_OK                           0

// See: griddyn/simulation/gridSimulation.h
#define GRIDDYN_STATUS_HALTED                      -2
#define GRIDDYN_STATUS_GD_ERROR                    -1
#define GRIDDYN_STATUS_STARTUP                      0
#define GRIDDYN_STATUS_INITIALIZED                  1
#define GRIDDYN_STATUS_POWERFLOW_COMPLETE           2
#define GRIDDYN_STATUS_DYNAMIC_INITIALIZED          3
#define GRIDDYN_STATUS_DYNAMIC_COMPLETE             4
#define GRIDDYN_STATUS_DYNAMIC_PARTIAL              5

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LIBGRIDDYN_BITS_GRIDDYN_CONSTANTS_H
