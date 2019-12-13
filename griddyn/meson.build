# this is here so it gets put in build/griddyn

griddyn_compiler_config = configure_file(
    input : '../config/cmake/compiler-config.h.in',
    output : 'compiler-config.h',
    format : 'cmake',
    configuration : griddyn_conf)

griddyn_config = configure_file(
    input : '../config/griddyn-config.h.in',
    output : 'griddyn-config.h',
    format : 'cmake',
    configuration : griddyn_conf)
