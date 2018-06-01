# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

set(extra_solver_sources
	extraSolvers.cpp
	)
	
set(extra_solver_headers
	extraSolvers.h
	)

set(paradae_headers
   paradae/common/def.h
   paradae/equations/AllEquations.h
   paradae/equations/Equation.h
   paradae/equations/Equation_DAE.h
   paradae/equations/Equation_DAE_full.h
   paradae/equations/Equation_ODE.h
   paradae/equations/EqGridDyn.h
   paradae/math/DBlockTriMatrix.h
   paradae/math/DenseMatrix.h
   paradae/math/IPoly.h
   paradae/math/IVander.h
   paradae/math/IVanderExt.h
   paradae/math/PMultiVector.h
   paradae/math/PVector.h
   paradae/math/SBlockTriMatrix.h
   paradae/math/SMultiVector.h
   paradae/math/SVector.h
   paradae/math/SparseMatrix.h
   paradae/math/Vector.h
   paradae/math/VirtualMatrix.h
   paradae/math/VirtualVector.h
   paradae/math/paradaeArrayData.h
   paradae/problems/ODEProblem.h
   paradae/solvers/LineSearch.h
   paradae/solvers/Newton.h
   paradae/solvers/NewtonStats.h
   paradae/solvers/Solver.h
   paradae/timeintegrators/AllTimeIntegrators.h
   paradae/timeintegrators/BackwardDiff.h
   paradae/timeintegrators/BackwardEuler.h
   paradae/timeintegrators/Billington_23.h
   paradae/timeintegrators/BogaSham_23.h
   paradae/timeintegrators/Cash_24.h
   paradae/timeintegrators/Cash_34.h
   paradae/timeintegrators/DormPrince_45.h
   paradae/timeintegrators/ExpMidPoint.h
   paradae/timeintegrators/ExpTrapezoidal.h
   paradae/timeintegrators/FE_ExpTrap_12.h
   paradae/timeintegrators/ForwardEuler.h
   paradae/timeintegrators/Fudziah_45.h
   paradae/timeintegrators/Gauss4.h
   paradae/timeintegrators/Gauss6.h
   paradae/timeintegrators/ImpMidPoint.h
   paradae/timeintegrators/ImpTrapezoidal.h
   paradae/timeintegrators/ImpVarUnk_12.h
   paradae/timeintegrators/Kutta3.h
   paradae/timeintegrators/Kutta4.h
   paradae/timeintegrators/Radau3.h
   paradae/timeintegrators/Ralston.h
   paradae/timeintegrators/RungeKutta.h
   paradae/timeintegrators/RungeKutta_DIRK.h
   paradae/timeintegrators/RungeKutta_Explicit.h
   paradae/timeintegrators/RungeKutta_Implicit.h
   paradae/timeintegrators/RungeKutta_SDIRK.h
   paradae/timeintegrators/SDIRK_12.h
   paradae/timeintegrators/TimeIntegrator.h
   )

set(paradae_sources
   paradae/common/def.cxx
   paradae/equations/Equation.cxx
   paradae/equations/Equation_ODE.cxx
   paradae/equations/EqGridDyn.cxx
   paradae/math/DBlockTriMatrix.cxx
   paradae/math/DenseMatrix.cxx
   paradae/math/IPoly.cxx
   paradae/math/IVander.cxx
   paradae/math/IVanderExt.cxx
   paradae/math/PMultiVector.cxx
   paradae/math/PVector.cxx
   paradae/math/SBlockTriMatrix.cxx
   paradae/math/SMultiVector.cxx
   paradae/math/SVector.cxx
   paradae/math/SparseMatrix.cxx
   paradae/math/Vector.cxx
   paradae/math/VirtualMatrix.cxx
   paradae/math/paradaeArrayData.cxx
   paradae/problems/ODEProblem.cxx
   paradae/solvers/LineSearch.cxx
   paradae/solvers/Newton.cxx
   paradae/solvers/NewtonStats.cxx
   paradae/solvers/Solver.cxx
   paradae/timeintegrators/BackwardDiff.cxx
   paradae/timeintegrators/BackwardEuler.cxx
   paradae/timeintegrators/Billington_23.cxx
   paradae/timeintegrators/BogaSham_23.cxx
   paradae/timeintegrators/Cash_24.cxx
   paradae/timeintegrators/Cash_34.cxx
   paradae/timeintegrators/DormPrince_45.cxx
   paradae/timeintegrators/ExpMidPoint.cxx
   paradae/timeintegrators/ExpTrapezoidal.cxx
   paradae/timeintegrators/FE_ExpTrap_12.cxx
   paradae/timeintegrators/ForwardEuler.cxx
   paradae/timeintegrators/Fudziah_45.cxx
   paradae/timeintegrators/Gauss4.cxx
   paradae/timeintegrators/Gauss6.cxx
   paradae/timeintegrators/ImpMidPoint.cxx
   paradae/timeintegrators/ImpTrapezoidal.cxx
   paradae/timeintegrators/ImpVarUnk_12.cxx
   paradae/timeintegrators/Kutta3.cxx
   paradae/timeintegrators/Kutta4.cxx
   paradae/timeintegrators/Radau3.cxx
   paradae/timeintegrators/Ralston.cxx
   paradae/timeintegrators/RungeKutta.cxx
   paradae/timeintegrators/RungeKutta_DIRK.cxx
   paradae/timeintegrators/RungeKutta_Explicit.cxx
   paradae/timeintegrators/RungeKutta_Implicit.cxx
   paradae/timeintegrators/RungeKutta_SDIRK.cxx
   paradae/timeintegrators/SDIRK_12.cxx
   paradae/timeintegrators/TimeIntegrator.cxx
   )
   
OPTION(BUILD_BRAID_SOLVER "enable the braid parallel in time solver integration" OFF)
OPTION(BUILD_SUPERLUMT_SOLVER "enable a superLU solver" OFF)
OPTION(BUILD_PARADAE_SOLVERS "enable inclusion of PARADAE solvers" OFF)

if (BUILD_BRAID_SOLVER)
find_package(Braid)
list(APPEND extra_solver_sources ${paradae_sources} braid/braidSolver.cpp braid/braid_driver.cxx)
list(APPEND extra_solver_headers ${paradae_headers} braid/braidInterface.h braid/braid_driver.h)
elseif (BUILD_PARADAE_SOLVERS)
list(APPEND extra_solver_sources ${paradae_sources})
list(APPEND extra_solver_headers ${paradae_headers})
endif()

IF (GRIDDYN_HAVE_MPI)
	list(APPEND extra_solver_sources 
		paradae/common/MapParam.cxx
		paradae/common/Timer.cxx
		)
	list(APPEND extra_solver_headers 
		paradae/common/def_mpi.h
		paradae/common/MapParam.h
		paradae/common/Timer.h
	)
ELSEIF(BUILD_BRAID_SOLVER)
	message(warning "Braid requires MPI to function")
ENDIF()

find_library(UMFPACK_LIBRARY umfpack HINTS ${SuiteSparse_DIRECT_LIBRARY_DIR})


#find_package(LAPACKE)
#if (LAPACKE_FOUND)
#INCLUDE_DIRECTORIES(${LAPACKE_INCLUDE_DIRS})
#endif()

add_library(extraSolverLibrary STATIC ${extra_solver_sources} ${extra_solver_headers})
set_target_properties (extraSolverLibrary PROPERTIES FOLDER libraries)

if (WIN32)
else(WIN32)
find_library(LAPACKE_LIBRARY lapacke HINTS /lib64 PATHS ${AUTOBUILD_INSTALL_PATH}/lib)
target_link_libraries(extraSolverLibrary PUBLIC ${LAPACKE_LIBRARY})
INCLUDE_DIRECTORIES(/usr/include/lapacke)
endif(WIN32)

target_link_libraries(extraSolverLibrary PUBLIC griddyn)
target_link_libraries(extraSolverLibrary PUBLIC ${LAPACKE_LIBRARIES})
target_link_libraries(extraSolverLibrary PUBLIC SuiteSparse::umfpack)

if (BUILD_PARADAE_SOLVERS OR BUILD_BRAID_SOLVER)
#source_group("paradae" FILES ${paradae_sources} ${paradae_headers})
source_group("paradae\\timeIntegrators" REGULAR_EXPRESSION "paradae/timeintegrators/.*")
source_group("paradae\\math" REGULAR_EXPRESSION "paradae/math/.*")
source_group("paradae\\common" REGULAR_EXPRESSION "paradae/common/.*")
source_group("paradae\\solvers" REGULAR_EXPRESSION "paradae/solvers/.*")
source_group("paradae\\equations" REGULAR_EXPRESSION "paradae/equations/.*")
endif()

if (BUILD_BRAID_SOLVER)
target_link_libraries(extraSolverLibrary PUBLIC braid::braid)
source_group("braid" REGULAR_EXPRESSION "braid/.*")
target_compile_definitions(extraSolverLibrary PUBLIC "-DENABLE_BRAID")
endif()

IF (GRIDDYN_HAVE_MPI)
	target_link_libraries(extraSolverLibrary PUBLIC MPI::MPI_C)
ENDIF(GRIDDYN_HAVE_MPI)

target_include_directories(extraSolverLibrary PRIVATE ${SuiteSparse_DIRECT_INCLUDE_DIR})

INSTALL(TARGETS extraSolverLibrary EXPORT griddyn-targets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)

INSTALL(FILES ${extra_solver_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/griddyn/extraSolver
 COMPONENT headers)


list (APPEND optional_library_key_headers extraSolvers/extraSolvers.h)
list (APPEND optional_library_functions loadExtraSolvers)

set (optional_library_key_headers ${optional_library_key_headers} PARENT_SCOPE)
set (optional_library_functions  ${optional_library_functions} PARENT_SCOPE)
