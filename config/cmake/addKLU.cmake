
OPTION(KLU_ENABLE "Enable KLU support" ON)

SET(SuiteSparse_Autobuild_DIR ${PROJECT_BINARY_DIR}/libs)

if(NOT DEFINED SuiteSparse_DIR)
    set(SuiteSparse_DIR ${SuiteSparse_Autobuild_DIR} CACHE PATH "path to SuiteSparse/KLU")
endif()

SHOW_VARIABLE(SuiteSparse_DIR PATH
  "KLU library directory" "${SuiteSparse_DIR}")

if(WIN32 AND NOT MSYS)
    OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" ON)
else()
    OPTION(AUTOBUILD_KLU "enable Suitesparse to automatically download and build" OFF)
endif()

if(KLU_ENABLE)
  IF(MSVC)
    set(SuiteSparse_USE_LAPACK_BLAS ON)
  ENDIF(MSVC)
  FILE(GLOB_RECURSE SUITESPARSE_CONFIG_FILE ${SuiteSparse_DIR}/*/suitesparse-config.cmake)

  # Check if SuiteSparse_DIR is the autobuild location 
  if("${SuiteSparse_DIR}" STREQUAL "${SuiteSparse_Autobuild_DIR}")

    # Handle autobuilding (and rebuilding)
    if(AUTOBUILD_KLU)
      option(FORCE_KLU_REBUILD "force rebuild of KLU" OFF)
      file(GLOB_RECURSE SUITESPARSE_CONFIG_FILE ${SuiteSparse_DIR}/*/suitesparse-config.cmake)
      # If the autobuild hasn't been done before, or a forced rebuild was requested
      if(FORCE_KLU_REBUILD OR (NOT SUITESPARSE_CONFIG_FILE))
        include(buildSuiteSparse)
        build_suitesparse(${SuiteSparse_Autobuild_DIR})
        set(FORCE_KLU_REBUILD OFF CACHE BOOL "force rebuild of KLU" FORCE)
      endif()
      unset(SUITESPARSE_CONFIG_FILE)
    endif()

    # Check for autobuild suitesparse install
    file(GLOB_RECURSE SUITESPARSE_CONFIG_FILE ${SuiteSparse_DIR}/*/suitesparse-config.cmake)
    if(SUITESPARSE_CONFIG_FILE)
      message(STATUS "loading suitesparse config a ${SUITESPARSE_CONFIG_FILE}")
      include(${SUITESPARSE_CONFIG_FILE})
      set(KLU_CMAKE TRUE)
      set(SuiteSparse_DIRECT_LIBRARY_DIR ${SuiteSparse_DIR}/lib)
      set(SuiteSparse_DIRECT_INCLUDE_DIR ${SuiteSparse_INCLUDE_DIRS}/suitesparse)
 
    # No suitesparse cmake file found, error during autobuild or the user wants to use the system copy
    else(SUITESPARSE_CONFIG_FILE)
      if(AUTOBUILD_KLU)
        message(WARNING "suitesparse not found, check for errors during autobuilding")
      else(AUTOBUILD_KLU)
        # Autobuild isn't enabled and the user didn't supply a location, search for a system copy
        find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
	    if(SuiteSparse_FOUND) 
	      set(KLU_CMAKE FALSE)
          get_filename_component(SuiteSparse_DIRECT_LIBRARY_DIR ${SuiteSparse_KLU_LIBRARY_RELEASE} DIRECTORY)
	      set(SuiteSparse_DIRECT_INCLUDE_DIR ${SuiteSparse_KLU_INCLUDE_DIR})
          if("${SuiteSparse_DIRECT_LIBRARY_DIR}" MATCHES "^${SuiteSparse_DIR}.*")
          else()
            message(STATUS "using system suitesparse")
          endif()

        # SuiteSparse was not found on the system 
        else(SuiteSparse_FOUND)
          message(WARNING "system suitesparse not found")
        endif(SuiteSparse_FOUND)
      endif(AUTOBUILD_KLU)
    endif(SUITESPARSE_CONFIG_FILE)

  # The given directory isn't the autobuild location; autobuilding won't affect what gets found
  else()
    # Check for suitesparse cmake file first
    file(GLOB_RECURSE SUITESPARSE_CONFIG_FILE ${SuiteSparse_DIR}/*/suitesparse-config.cmake)
    if(SUITESPARSE_CONFIG_FILE)
      message(STATUS "loading suitesparse config b ${SUITESPARSE_CONFIG_FILE}")
      include(${SUITESPARSE_CONFIG_FILE})
      set(KLU_CMAKE TRUE)
      set(SuiteSparse_DIRECT_LIBRARY_DIR ${SuiteSparse_DIR}/lib)
      set(SuiteSparse_DIRECT_INCLUDE_DIR ${SuiteSparse_INCLUDE_DIRS}/suitesparse)

    # No suitesparse cmake file was found, search the system for files
    else(SUITESPARSE_CONFIG_FILE)
	  find_package(SuiteSparse COMPONENTS KLU AMD COLAMD BTF SUITESPARSECONFIG CXSPARSE)
	  if(SuiteSparse_FOUND) 
	    set(KLU_CMAKE FALSE)
        get_filename_component(SuiteSparse_DIRECT_LIBRARY_DIR ${SuiteSparse_KLU_LIBRARY_RELEASE} DIRECTORY)
	    set(SuiteSparse_DIRECT_INCLUDE_DIR ${SuiteSparse_KLU_INCLUDE_DIR})
        if("${SuiteSparse_DIRECT_LIBRARY_DIR}" MATCHES "^${SuiteSparse_DIR}.*")
          message(STATUS "found suitesparse at ${SuiteSparse_DIR}")
        else()
            message(WARNING "could not find suitesparse at ${SuiteSparse_DIR}, using system suitesparse")
        endif()

      # SuiteSparse wasn't found at user location or on system, error (autobuilding as a fall-back produced some mixed results)
      else(SuiteSparse_FOUND)
        # This used to try setting SuiteSparse_DIR and autobuilding
        # On systems with a system-wide install of SuiteSparse, find package resulted in an odd mix of paths
        if(AUTOBUILD_KLU)
		message(FATAL_ERROR "unset SuiteSparse_DIR using cmake's -USuiteSparse_DIR option to autobuild SuiteSparse, or ensure that SuiteSparse_DIR is set to the location of a SuiteSparse installation")
        else()
          message(WARNING "suitesparse not found")
        endif()
      endif(SuiteSparse_FOUND)
    endif(SUITESPARSE_CONFIG_FILE)
  endif()

endif(KLU_ENABLE)
