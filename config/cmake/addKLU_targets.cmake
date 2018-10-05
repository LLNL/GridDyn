
OPTION(ENABLE_KLU "Enable KLU support (KLU is the primary spare linear solver, it can be disabled but it is not recommended)" ON)
OPTION(USE_KLU_SHARED "Use the KLU shared library instead of STATIC" OFF)

SHOW_VARIABLE(SuiteSparse_INSTALL_PATH PATH
  "path to the SuiteSparse libraries" "${AUTOBUILD_INSTALL_PATH}")

if(WIN32 AND NOT MSYS)
    OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" ON)
else()
    OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" OFF)
endif()

if (NOT DEFINED SuiteSparse_DIR)
	set(SuiteSparse_DIR ${SuiteSparse_INSTALL_PATH})
endif()

if(ENABLE_KLU)
  IF(MSVC)
    set(SuiteSparse_USE_LAPACK_BLAS ON)
  ENDIF(MSVC)

  set(SUITESPARSE_CMAKE_SUFFIXES
	lib/cmake/suitesparse-5.1.0
	cmake/suitesparse-5.1.0
	cmake
	lib/cmake)

	if (WIN32 AND NOT MSYS)
      find_package(SuiteSparse QUIET COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD
		HINTS
			${SuiteSparse_INSTALL_PATH}
			${AUTOBUILD_INSTALL_PATH}
		PATH_SUFFIXES ${SUITESPARSE_CMAKE_SUFFIXES}
		)
	else()
   find_package(SuiteSparse QUIET COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD
		HINTS
			${SuiteSparse_INSTALL_PATH}
			${AUTOBUILD_INSTALL_PATH}
		PATH_SUFFIXES ${SUITESPARSE_CMAKE_SUFFIXES}
		NO_SYSTEM_ENVIRONMENT_PATH
		NO_CMAKE_PACKAGE_REGISTRY
		NO_CMAKE_SYSTEM_PATH
		NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
		)
	endif()


	if (NOT SuiteSparse_FOUND)

		if (AUTOBUILD_KLU)
			include(buildSuiteSparse)
			build_suitesparse(${SuiteSparse_INSTALL_PATH})
			find_package(suitesparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD
				HINTS
					${SuiteSparse_INSTALL_PATH}
				PATH_SUFFIXES ${SUITESPARSE_CMAKE_SUFFIXES}
			)
		else(AUTOBUILD_KLU)
			find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD)
		if (NOT SuiteSparse_FOUND)
			if (WIN32 AND NOT MSYS)
				OPTION(AUTOBUILD_KLU "enable KLU to automatically download and build" ON)
			else()
				OPTION(AUTOBUILD_KLU "enable KLU to automatically download and build" OFF)
			endif()
			if (AUTOBUILD_KLU)
				include(buildSuiteSparse)
				build_suitesparse(${SuiteSparse_INSTALL_PATH})
				find_package(suitesparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD
					HINTS
						${SuiteSparse_INSTALL_PATH}
					PATH_SUFFIXES ${SUITESPARSE_CMAKE_SUFFIXES}
				)
			endif(AUTOBUILD_KLU)
		endif()
		endif(AUTOBUILD_KLU)
	else()
		if(AUTOBUILD_KLU)
			option(FORCE_SuiteSparse_REBUILD "force rebuild of SuiteSparse" OFF)
			 if(FORCE_SuiteSparse_REBUILD )
				include(buildSuiteSparse)
				build_suitesparse(${SuiteSparse_INSTALL_PATH})
				set(FORCE_SuiteSparse_REBUILD OFF CACHE BOOL "force rebuild of SuiteSparse" FORCE)
			endif(FORCE_SuiteSparse_REBUILD)
      endif()
	endif()

endif(ENABLE_KLU)
