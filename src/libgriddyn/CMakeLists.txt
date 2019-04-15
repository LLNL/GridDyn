add_library(libgriddyn SHARED)

set_target_properties(
  libgriddyn PROPERTIES
    # GridDyn might have a different setting for PIC, so force it on for libgriddyn
    POSITION_INDEPENDENT_CODE ON
    # This library is already prefixed with "lib", so don't prepend another one
    PREFIX ""
    # Hide all non-C functions
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

target_link_libraries(libgriddyn PRIVATE griddyn)
target_link_libraries(libgriddyn PRIVATE formatInterpreter fileInput)

target_include_directories(libgriddyn PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
  $<INSTALL_INTERFACE:include>)

add_subdirectory(src)

set(libgriddyn_headers
  griddyn/libgriddyn.h
  griddyn/bits/griddyn_constants.h
  griddyn/bits/griddyn_export.h
  griddyn/bits/griddyn_structs.h
  griddyn/bits/griddyn_types.h
)

install(
    TARGETS libgriddyn
    EXPORT libgriddyn-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

install(
    FILES ${libgriddyn_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
)
