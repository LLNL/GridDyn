# this is in griddyn/meson.build rather than the root meson.build so it gets put in the correct build subdirectory

# TODO: this should absolutely not be installed. It's required by (at least) the following:
# * gridDynRunner
griddyn_compiler_config = configure_file(
    input : '../config/cmake/compiler-config.h.in',
    output : 'compiler-config.h',
    format : 'cmake',
    configuration : griddyn_conf,
    install : true,
    install_dir : 'include')

griddyn_config = configure_file(
    input : '../config/griddyn-config.h.in',
    output : 'griddyn-config.h',
    format : 'cmake',
    configuration : griddyn_conf,
    install : true,
    install_dir : 'include/griddyn')
