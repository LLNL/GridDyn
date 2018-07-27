if(NOT DEFINED SUNDIALS_DIR)
  set(SUNDIALS_DIR ${AUTOBUILD_INSTALL_PATH} CACHE PATH "path to SUNDIALS")
endif()

SHOW_VARIABLE(SUNDIALS_DIR PATH
  "SUNDIALS library directory" "${SUNDIALS_DIR}")

  set(SUNDIALS_FIND_QUIETLY ON)

  set(SUNDIALS_REQUIRED IDA KINSOL SUNMATRIXDENSE SUNLINSOLDENSE)
  if (OPENMP_FOUND)
	list(APPEND SUNDIALS_REQUIRED nvecopenmp)
  endif(OPENMP_FOUND)

  if (KLU_ENABLE)
	list(APPEND SUNDIALS_REQUIRED sunmatrixsparse sunlinsolklu)
  endif(KLU_ENABLE)

find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)

  if(SUNDIALS_FOUND)
	list(INSERT external_library_list 0 ${SUNDIALS_LIBRARIES})
  OPTION(FORCE_SUNDIALS_REBUILD "force rebuild of sundials" OFF)
	IF (AUTOBUILD_SUNDIALS)

		IF(FORCE_SUNDIALS_REBUILD)
			include(buildSundials)
			if (MSVC)
				build_sundials_msvc()
			elseif(MINGW)
			build_sundials_mingw()
		else()
			build_sundials()
		endif()
			set(FORCE_SUNDIALS_REBUILD OFF CACHE BOOL "force rebuild of sundials" FORCE)
		ENDIF(FORCE_SUNDIALS_REBUILD)
	ELSE (AUTOBUILD_SUNDIALS)
	   IF(FORCE_SUNDIALS_REBUILD)
			include(buildSundials)
			if (MSVC)
				build_sundials_msvc()
			elseif(MINGW)
			build_sundials_mingw()
		else()
			build_sundials()
		endif()
			set(SUNDIALS_FOUND OFF CACHE BOOL "sundials not found" FORCE)
			set(SUNDIALS_LIBRARIES NOT_FOUND FORCE)
			set(FORCE_SUNDIALS_REBUILD OFF CACHE BOOL "force rebuild of sundials" FORCE)
			set(SUNDIALS_DIR ${PROJECT_BINARY_DIR}/libs)
			find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)
		ENDIF(FORCE_SUNDIALS_REBUILD)
	ENDIF(AUTOBUILD_SUNDIALS)
  else(SUNDIALS_FOUND)
    OPTION(AUTOBUILD_SUNDIALS "enable Sundials to automatically download and build" ON)
    IF (AUTOBUILD_SUNDIALS)
      include(buildSundials)
      if (MSVC)
			build_sundials_msvc()
		elseif(MINGW)
			build_sundials_mingw()
		else()
			build_sundials()
		endif()
      set(SUNDIALS_DIR ${PROJECT_BINARY_DIR}/libs)
      find_package(SUNDIALS REQUIRED COMPONENTS ${SUNDIALS_REQUIRED} OPTIONAL_COMPONENTS CVODE ARKODE)
    ENDIF(AUTOBUILD_SUNDIALS)
  if (SUNDIALS_FOUND)
    list(INSERT external_library_list 0 ${SUNDIALS_LIBRARIES})
  else (SUNDIALS_FOUND)
	message( FATAL_ERROR "sundials not found - unable to continue")
	message( "Double check spelling specified libraries (search is case sensitive)")
  endif(SUNDIALS_FOUND)
  endif(SUNDIALS_FOUND)

if (SUNDIALS_ARKODE_FOUND)
set(LOAD_ARKODE TRUE)
endif(SUNDIALS_ARKODE_FOUND)

if (SUNDIALS_CVODE_FOUND)
set(LOAD_CVODE TRUE)
endif(SUNDIALS_CVODE_FOUND)
