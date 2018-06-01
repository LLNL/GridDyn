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
#ifndef braid_driver_h
#define braid_driver_h

#include <list>
#include <map>
#include "braid.h"
#include "../paradae/math/Vector.h"
#include "../paradae/math/SMultiVector.h"
#include "../paradae/problems/ODEProblem.h"

namespace griddyn {
namespace braid {

enum BDF_STRAT { nobdf, usual, usual_c, uni0, uni0_c, uni1, uni1_c, inject, inject_c, extrap, extrap_c };

/*!< Basic Vector structure for braid driver
 */
typedef struct _braid_Vector_struct
{
    // Shell part
    SMultiVector tprev;
    paradae::SVector state;        // JBS:  This "state" is I believe related either only to BDF or to root-finding.  Not a well named variable.

    // Vector part (this part contains actual "state" information)
    paradae::SMultiVector xprev;   /*!< This is only a list of Vectors when multistepping, otherwise it's just one vector */
    paradae::SVector dxprev;
} my_Vector;

/*!< Basic Application structure for braid driver
 * Simply wrap the ODEProblem class
 */
typedef struct _braid_App_struct
{
    ODEProblem* ode; /*!< Pointer to the main class ODEProblem */
    int nb_multisteps;
    int size_x; /*!< Size of the problem */
    int size_state;
    BDF_STRAT bdf_strat;
    //bool do_bdf_uniform;
    //bool do_lowerorder; /*!< Do we lower the order of BDF method ? */
    int lowered_by_level; /*!< Lowered by how much each level ? */
    int min_order;
    //std::string do_interp;
    int nb_initial;
    Real* grid_initial;
    Real* braid_grid_initial;
    int prevlvl;
    std::map<Real, my_Vector> initial_vector;
    DATA_Struct alloc_data;
    my_Vector *solution_tfinal;

    _braid_App_struct(ODEProblem* ode_);
    void SetAllToDataStruct(braid_Vector u);
    void SetLastToDataStruct(braid_Vector u);
    void SetAllFromDataStruct(braid_Vector u);
    void SetLastFromDataStruct(braid_Vector u);
    void DumpDataStruct();
} Braid_App;

Real IntegrationLoop(braid_App app, std::list<Real>& tprev, std::list<Vector>& xprev, const std::list<Real>& tstops, std::list<Vector>& xstops, Vector& dxprev);
void my_Step_OnAllPoints(braid_App app, braid_Vector ustop, braid_Vector fstop, braid_Vector u, braid_StepStatus status, int level);
void my_Step_OnOnePoint(braid_App app, braid_Vector ustop, braid_Vector fstop, braid_Vector u, braid_StepStatus status, int level);
int my_Step(braid_App app, braid_Vector ustop, braid_Vector fstop, braid_Vector u, braid_StepStatus status);
int my_SpatialRefine(braid_App app, braid_Vector cu, braid_Vector *fu_ptr, braid_CoarsenRefStatus status);
int my_SpatialCoarsen(braid_App app, braid_Vector fu, braid_Vector *cu_ptr, braid_CoarsenRefStatus status);
int my_Init(braid_App app, Real t, braid_Vector *u_ptr);
int my_InitShell(braid_App app, Real t, braid_Vector *u_ptr);
int my_Clone(braid_App app, braid_Vector u, braid_Vector *v_ptr);
int my_CloneShell(braid_App app, braid_Vector u, braid_Vector *v_ptr);
int my_Free(braid_App app, braid_Vector u);
int my_FreeShell(braid_App app, braid_Vector u);
int my_PropagateShell(braid_App app, braid_Vector x, braid_Vector y);
int my_Sum(braid_App app, Real alpha, braid_Vector x, Real beta, braid_Vector y);
int my_SpatialNorm(braid_App app, braid_Vector u, Real *norm_ptr);
int my_Access(braid_App app, braid_Vector u, braid_AccessStatus astatus);
int my_BufSize(braid_App app, int *size_ptr, braid_BufferStatus bstatus);
int my_BufPack(braid_App app, braid_Vector u, void *buffer, braid_BufferStatus bstatus);
int my_BufUnpack(braid_App app, void *buffer, braid_Vector *u_ptr, braid_BufferStatus bstatus);
int my_TimeGrid(braid_App app, braid_Real *ta, braid_Int *ilower, braid_Int *iupper);

} //namespace braid
} //namespace griddyn
#endif
