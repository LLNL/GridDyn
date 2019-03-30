#ifndef GRIDDYN_BITS_GRIDDYN_CONSTANTS_H
#define GRIDDYN_BITS_GRIDDYN_CONSTANTS_H

#include <stdint.h> // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t  griddyn_flag_t;
typedef int       griddyn_result_t;
typedef int       griddyn_status_t;

#define GRIDDYN_DENSE_SOLVER                        0x00000001
#define GRIDDYN_POWER_ADJUST_ENABLED                0x00000002
#define GRIDDYN_DCFLOW_INITIALIZATION               0x00000004
#define GRIDDYN_PARALLEL_RESIDUAL_ENABLED           0x00000008
#define GRIDDYN_PARALLEL_JACOBIAN_ENABLED           0x00000010
#define GRIDDYN_PARALLEL_CONTINGENCY_ENABLED        0x00000020
#define GRIDDYN_MPI_CONTINGENCY_ENABLED             0x00000040
#define GRIDDYN_FIRST_RUN_LIMITS_ONLY               0x00000080
#define GRIDDYN_NO_RESET                            0x00000100
#define GRIDDYN_VOLTAGE_CONSTRAINTS_FLAG            0x00000200
#define GRIDDYN_RECORD_ON_HALT_FLAG                 0x00000400
#define GRIDDYN_NO_AUTO_SLACK_BUS                   0x00000800
#define GRIDDYN_NO_AUTO_DISCONNECT                  0x00001000
#define GRIDDYN_SINGLE_STEP_MODE                    0x00002000
#define GRIDDYN_DC_MODE                             0x00004000
#define GRIDDYN_FORCE_POWER_FLOW                    0x00008000
#define GRIDDYN_POWER_FLOW_ONLY                     0x00010000
#define GRIDDYN_NO_POWERFLOW_ADJUSTMENTS            0x00020000
#define GRIDDYN_SAVE_POWER_FLOW_DATA                0x00040000
#define GRIDDYN_NO_POWERFLOW_ERROR_RECOVERY         0x00080000
#define GRIDDYN_DAE_INITIALIZATION_FOR_PARTITIONED  0x00100000
#define GRIDDYN_FORCE_EXTRA_POWERFLOW               0x00200000
#define GRIDDYN_DROOP_POWER_FLOW                    0x00400000
#define GRIDDYN_DCJACCOMP_FLAG                      0x00800000
#define GRIDDYN_RESET_VOLTAGE_FLAG                  0x01000000
#define GRIDDYN_PREV_SETALL_PQVLIMIT                0x02000000
#define GRIDDYN_INVALID_STATE_FLAG                  0x04000000
#define GRIDDYN_CHECK_RESET_VOLTAGE_FLAG            0x08000000
#define GRIDDYN_POWERFLOW_SAVED                     0x10000000
#define GRIDDYN_LOW_BUS_VOLTAGE                     0x20000000

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GRIDDYN_BITS_GRIDDYN_CONSTANTS_H
