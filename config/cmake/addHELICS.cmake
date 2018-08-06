
IF (MSVC)
	set(HELICS_PATH_HINTS
		C:/local/helics_1_3_0
		C:/local/helics_1_2_1
		)
ENDIF(MSVC)

include(GNUInstallDirs)

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")


set(HELICS_CMAKE_SUFFIXES 
	lib/cmake/HELICS/
			cmake/HELICS/)
	
find_package(HELICS QUIET
	HINTS 
		${HELICS_INSTALL_PATH}
		$ENV{HELICS_INSTALL_PATH}
		${HELICS_PATH_HINTS}
	PATH_SUFFIXES ${HELICS_CMAKE_SUFFIXES}
	)
				
if (HELICS_FOUND)
		IF (AUTOBUILD_HELICS)
			OPTION(FORCE_HELICS_REBUILD "force rebuild of helics" OFF)
			IF(FORCE_HELICS_REBUILD)
				include(buildHELICS)
				build_helics()
				set(FORCE_HELICS_REBUILD OFF CACHE BOOL "force rebuild of helics" FORCE)
			ENDIF(FORCE_HELICS_REBUILD)
		ENDIF(AUTOBUILD_HELICS)
else()
	OPTION(AUTOBUILD_HELICS "enable HELICS to automatically download and build" OFF)
	IF(AUTOBUILD_HELICS)
		include(buildHELICS)
		build_helics()
		find_package(HELICS
			HINTS 
				${HELICS_INSTALL_PATH}
			PATH_SUFFIXES ${HELICS_CMAKE_SUFFIXES}
		)

	ENDIF(AUTOBUILD_HELICS)
endif()
