
OPTION(${PROJECT_NAME}_ENABLE_KLU "Enable KLU support (KLU is the primary spare linear solver, it can be disabled but it is not recommended)" ON)
OPTION(${PROJECT_NAME}_USE_KLU_SHARED "Use the KLU shared library instead of STATIC" OFF)

SHOW_VARIABLE(SuiteSparse_INSTALL_PATH PATH
  "path to the SuiteSparse libraries" "${AUTOBUILD_INSTALL_PATH}")

if(WIN32 AND NOT MSYS)
    OPTION(${PROJECT_NAME}_AUTOBUILD_KLU "enable Suitesparse to automatically download and build" ON)
else()
    OPTION(${PROJECT_NAME}_AUTOBUILD_KLU "enable Suitesparse to automatically download and build" OFF)
endif()

if (NOT DEFINED SuiteSparse_DIR)
	set(SuiteSparse_NOT_GIVEN TRUE)
	set(SuiteSparse_DIR ${SuiteSparse_INSTALL_PATH})
endif()

if(${PROJECT_NAME}_ENABLE_KLU)
  IF(MSVC)
    set(SuiteSparse_USE_LAPACK_BLAS ON)
  ENDIF(MSVC)

  set(SUITESPARSE_CMAKE_SUFFIXES
	lib/cmake/suitesparse-5.4.0
	cmake/suitesparse-5.4.0
	cmake
	lib/cmake)
set(LAPACK_DIR ${PROJECT_BINARY_DIR}/Download/suitesparse/lapack_windows/x64)
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

		if (${PROJECT_NAME}_AUTOBUILD_KLU)
			include(buildSuiteSparse)
			build_suitesparse(${SuiteSparse_INSTALL_PATH})
			message(STATUS "KLU DIR: ${SuiteSparse_DIR}")
			unset(SuiteSparse_DIR)
			message(STATUS "KLU DIR: ${SuiteSparse_DIR}")
			#set(SuiteSparse_DIR ${SuiteSparse_INSTALL_PATH})
			find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD
				HINTS
					${SuiteSparse_INSTALL_PATH}
				PATH_SUFFIXES ${SUITESPARSE_CMAKE_SUFFIXES}
			)
		else(${PROJECT_NAME}_AUTOBUILD_KLU)
			# For Unix OSes, search for a system copy of KLU
			# Try to avoid error from '-NOTFOUND' SuiteSparse_DIR cache entry after failed find_package
			if (SuiteSparse_NOT_GIVEN)
				set(SuiteSparse_DIR ${SuiteSparse_INSTALL_PATH})
				unset(SuiteSparse_NOT_GIVEN)
			endif()
			find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE UMFPACK CCOLAMD CAMD CHOLMOD)
			if (NOT SuiteSparse_FOUND)
				message(FATAL_ERROR "KLU was not found on the system and AUTOBUILD_KLU is set to OFF")
			endif()
		endif(${PROJECT_NAME}_AUTOBUILD_KLU)
	else()
		if(${PROJECT_NAME}_AUTOBUILD_KLU)
			option(FORCE_SuiteSparse_REBUILD "force rebuild of SuiteSparse" OFF)
			 if(FORCE_SuiteSparse_REBUILD )
				include(buildSuiteSparse)
				build_suitesparse(${SuiteSparse_INSTALL_PATH})
				set(FORCE_SuiteSparse_REBUILD OFF CACHE BOOL "force rebuild of SuiteSparse" FORCE)
			endif(FORCE_SuiteSparse_REBUILD)
      endif()
	endif()

endif(${PROJECT_NAME}_ENABLE_KLU)
