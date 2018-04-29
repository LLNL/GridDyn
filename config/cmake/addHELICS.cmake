
IF (MSVC)
	set(HELICS_PATH_HINTS
		C:/local/helics_1_0_3
		C:/local/helics_1_0_0)
ENDIF(MSVC)

SHOW_VARIABLE(HELICS_INSTALL_PATH PATH "path to the helics installation" "${PROJECT_BINARY_DIR}/libs")
find_file(HELICS_IMPORT_FILE HELICSImport.cmake
	HINTS ${HELICS_INSTALL_PATH}
		${HELICS_PATH_HINTS}
	PATH_SUFFIXES 
			lib/cmake/HELICS/
			cmake/HELICS/
				)
				
	message(status ${HELICS_IMPORT_FILE})

	if (EXISTS ${HELICS_IMPORT_FILE})
		IF (AUTOBUILD_HELICS)
			OPTION(FORCE_HELICS_REBUILD "force rebuild of helics" OFF)
			IF(FORCE_HELICS_REBUILD)
				include(buildHELICS)
				build_helics()
				set(HELICS_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
				set(FORCE_HELICS_REBUILD OFF CACHE BOOL "force rebuild of helics" FORCE)
			ENDIF(FORCE_HELICS_REBUILD)
		ENDIF(AUTOBUILD_HELICS)
		message(STATUS "loading ${HELICS_IMPORT_FILE}")
		include("${HELICS_IMPORT_FILE}")
	else()
		message(STATUS "couldn't find ${HELICS_IMPORT_FILE}")
		OPTION(AUTOBUILD_HELICS "enable HELICS to automatically download and build" OFF)
		IF(AUTOBUILD_HELICS)
			include(buildHELICS)
			build_helics()
			include(${HELICS_INSTALL_PATH}/lib/cmake/HELICS/HELICSImport.cmake)
		ENDIF(AUTOBUILD_HELICS)
	endif()
