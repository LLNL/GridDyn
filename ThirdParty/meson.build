# TODO: use internal zlib
zlib_dep = dependency('zlib')


minizip_inc = include_directories('Minizip')
minizip_lib = library('minizip', [
        'Minizip/minizip.c',
        'Minizip/miniunz.c',
        'Minizip/zip.c',
        'Minizip/ioapi.c',
        'Minizip/unzip.c',
        'Minizip/argv.c',
    ],
    dependencies : zlib_dep,
    install : true)
minizip_dep = declare_dependency(link_with : minizip_lib, include_directories : minizip_inc)

jsoncpp_inc = include_directories('json')
jsoncpp_lib = library('jsoncpp',
    'json/jsoncpp.cpp',
    install : true)
jsoncpp_dep = declare_dependency(link_with : jsoncpp_lib, include_directories : jsoncpp_inc)

toml_inc = include_directories('toml')
toml_dep = declare_dependency(include_directories : toml_inc)

ticpp_inc = include_directories('ticpp')
ticpp_lib = library('ticpp', [
        'ticpp/ticpp.cpp',
        'ticpp/tinystr.cpp',
        'ticpp/tinyxml.cpp',
        'ticpp/tinyxmlerror.cpp',
        'ticpp/tinyxmlparser.cpp',
    ],
    cpp_args : '-DTIXML_USE_TICPP',
    include_directories : include_directories(['ticpp/ticpp', 'ticpp']),
    install : true)
ticpp_dep = declare_dependency(link_with : ticpp_lib, include_directories : ticpp_inc)

inih_inc = include_directories('inih')
inih_dep = declare_dependency(include_directories : inih_inc)

tinyxml2_inc = include_directories('tinyxml2')
tinyxml2_lib = library('tinyxml2',
    'tinyxml2/tinyxml2.cpp',
    include_directories : include_directories('tinyxml2/tinyxml2'),
    install : true)
tinyxml2_dep = declare_dependency(link_with : tinyxml2_lib, include_directories : tinyxml2_inc)

cereal_inc = include_directories('cereal')
cereal_dep = declare_dependency(include_directories : cereal_inc)

yaml_cpp_inc = include_directories('yaml-cpp/include')
yaml_cpp_lib = library('yaml_cpp', [
        'yaml-cpp/src/binary.cpp',
        'yaml-cpp/src/convert.cpp',
        'yaml-cpp/src/directives.cpp',
        'yaml-cpp/src/emit.cpp',
        'yaml-cpp/src/emitfromevents.cpp',
        'yaml-cpp/src/emitter.cpp',
        'yaml-cpp/src/emitterstate.cpp',
        'yaml-cpp/src/emitterutils.cpp',
        'yaml-cpp/src/exceptions.cpp',
        'yaml-cpp/src/exp.cpp',
        'yaml-cpp/src/memory.cpp',
        'yaml-cpp/src/nodebuilder.cpp',
        'yaml-cpp/src/node.cpp',
        'yaml-cpp/src/node_data.cpp',
        'yaml-cpp/src/nodeevents.cpp',
        'yaml-cpp/src/null.cpp',
        'yaml-cpp/src/ostream_wrapper.cpp',
        'yaml-cpp/src/parse.cpp',
        'yaml-cpp/src/parser.cpp',
        'yaml-cpp/src/regex_yaml.cpp',
        'yaml-cpp/src/scanner.cpp',
        'yaml-cpp/src/scanscalar.cpp',
        'yaml-cpp/src/scantag.cpp',
        'yaml-cpp/src/scantoken.cpp',
        'yaml-cpp/src/simplekey.cpp',
        'yaml-cpp/src/singledocparser.cpp',
        'yaml-cpp/src/stream.cpp',
        'yaml-cpp/src/tag.cpp',
    ],
    include_directories : yaml_cpp_inc,
    install : true)
yaml_cpp_dep = declare_dependency(link_with : yaml_cpp_lib, include_directories : yaml_cpp_inc)

zmq_dep = dependency('libzmq')
cppzmq_inc = include_directories('cppzmq')
cppzmq_dep = declare_dependency(dependencies : zmq_dep, include_directories : cppzmq_inc)

subdir('sundials/sundials') # put sundials_config in the right place

cc = meson.get_compiler('c')
m_lib = cc.find_library('m')

type_options = {
    'd' : '-DNULLDEFINE',
    'z' : '-DCOMPLEX',
    'l' : '-DDLONG',
    'lz' : '-DCOMPLEX -DDLONG',
}

ss_inc = include_directories('SuiteSparse/SuiteSparse-5.3.0/SuiteSparse_config')
ss_lib = library('suitesparse', [
        'SuiteSparse/SuiteSparse-5.3.0/SuiteSparse_config/SuiteSparse_config.c',
    ],
    include_directories : ss_inc,
    dependencies : m_lib,
    install : true)
ss_dep = declare_dependency(include_directories : ss_inc, link_with : ss_lib)

amd_inc = include_directories('SuiteSparse/SuiteSparse-5.3.0/AMD/Include')
amd_libs = []
foreach type, define : type_options
    amd_lib = library('amd_' + type, [
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_1.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_2.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_aat.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_control.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_defaults.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_dump.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_global.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_info.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_order.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_postorder.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_post_tree.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_preprocess.c',
            'SuiteSparse/SuiteSparse-5.3.0/AMD/Source/amd_valid.c',
        ],
        c_args : define,
        include_directories : amd_inc,
        dependencies : [
            ss_dep,
            m_lib
        ],
        install : true)
    amd_libs += [amd_lib]
endforeach
amd_dep = declare_dependency(
    include_directories : amd_inc,
    dependencies : ss_dep,
    link_with : amd_libs)

colamd_inc = include_directories('SuiteSparse/SuiteSparse-5.3.0/COLAMD/Include')
colamd_libs = []
foreach type, define : type_options
    colamd_lib = library('colamd_' + type,
        'SuiteSparse/SuiteSparse-5.3.0/COLAMD/Source/colamd.c',
        c_args : define,
        include_directories : colamd_inc,
        dependencies : [
            ss_dep,
            m_lib
        ],
        install : true)
    colamd_libs += [colamd_lib]
endforeach
colamd_dep = declare_dependency(include_directories : colamd_inc, link_with : colamd_libs)

btf_inc = include_directories('SuiteSparse/SuiteSparse-5.3.0/BTF/Include')
btf_libs = []
foreach type, define : type_options
    btf_lib = library('btf_' + type, [
            'SuiteSparse/SuiteSparse-5.3.0/BTF/Source/btf_maxtrans.c',
            'SuiteSparse/SuiteSparse-5.3.0/BTF/Source/btf_order.c',
            'SuiteSparse/SuiteSparse-5.3.0/BTF/Source/btf_strongcomp.c',
        ],
        c_args : define,
        include_directories : btf_inc,
        dependencies : ss_dep,
        install : true)
    btf_libs += [btf_lib]
endforeach
btf_dep = declare_dependency(include_directories : btf_inc, link_with : btf_libs)

klu_inc = include_directories('SuiteSparse/SuiteSparse-5.3.0/KLU/Include')
klu_libs = []
foreach type, define : type_options
    klu_lib = library('klu_' + type, [
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_analyze.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_analyze_given.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_defaults.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_diagnostics.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_dump.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_extract.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_factor.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_free_numeric.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_free_symbolic.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_kernel.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_memory.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_refactor.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_scale.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_solve.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_sort.c',
            'SuiteSparse/SuiteSparse-5.3.0/KLU/Source/klu_tsolve.c',
        ],
        include_directories : klu_inc,
        c_args : define,
        dependencies : [
            amd_dep,
            colamd_dep,
            btf_dep,
            m_lib,
        ],
        install : true)
    klu_libs += [klu_lib]
endforeach
klu_dep = declare_dependency(
    include_directories : klu_inc,
    link_with : klu_libs,
    dependencies : [
        amd_dep,
        colamd_dep,
        btf_dep,
    ])

sundials_inc = include_directories('sundials/sundials-3.1.1/include')
sundials_config_file_inc = include_directories('sundials')
sundials_lib = library('sundials', [
        sundials_config_file,
        'sundials/sundials-3.1.1/src/kinsol/kinsol_io.c',
        'sundials/sundials-3.1.1/src/kinsol/kinsol.c',
        'sundials/sundials-3.1.1/src/nvec_ser/nvector_serial.c',
        'sundials/sundials-3.1.1/src/sundials/sundials_math.c',
        'sundials/sundials-3.1.1/src/sundials/sundials_nvector.c',
        'sundials/sundials-3.1.1/src/kinsol/kinsol_direct.c',
        'sundials/sundials-3.1.1/src/sundials/sundials_linearsolver.c',
        'sundials/sundials-3.1.1/src/sundials/sundials_matrix.c',
        'sundials/sundials-3.1.1/src/sunmat_dense/sunmatrix_dense.c',
        'sundials/sundials-3.1.1/src/sunmat_band/sunmatrix_band.c',
        'sundials/sundials-3.1.1/src/arkode/arkode.c',
        'sundials/sundials-3.1.1/src/arkode/arkode_io.c',
        'sundials/sundials-3.1.1/src/arkode/arkode_butcher.c',
        'sundials/sundials-3.1.1/src/arkode/arkode_direct.c',
        'sundials/sundials-3.1.1/src/ida/ida.c',
        'sundials/sundials-3.1.1/src/ida/ida_io.c',
        'sundials/sundials-3.1.1/src/ida/ida_direct.c',
        'sundials/sundials-3.1.1/src/ida/ida_ic.c',
        'sundials/sundials-3.1.1/src/sunlinsol_dense/sunlinsol_dense.c',
        'sundials/sundials-3.1.1/src/sundials/sundials_dense.c',
        'sundials/sundials-3.1.1/src/cvode/cvode.c',
        'sundials/sundials-3.1.1/src/cvode/cvode_direct.c',
        'sundials/sundials-3.1.1/src/cvode/cvode_io.c',
        'sundials/sundials-3.1.1/src/sunmat_sparse/sunmatrix_sparse.c',
        'sundials/sundials-3.1.1/src/sunlinsol_klu/sunlinsol_klu.c',
    ],
    dependencies : [
        m_lib,
        klu_dep,
    ],
    include_directories : [ sundials_inc, sundials_config_file_inc ],
    install : true)

sundials_dep = declare_dependency(
    link_with : [
        sundials_lib,
    ],
    include_directories : [
        sundials_inc,
        sundials_config_file_inc
    ],
    dependencies : klu_dep)

gmlc_inc = include_directories('utilities', 'containers', 'concurrency')
gmlc_lib = library('gmlc', [
        'utilities/gmlc/utilities/stringOps.cpp',
        'utilities/gmlc/utilities/charMapper.cpp',
        'utilities/gmlc/utilities/stringConversion.cpp',
        'utilities/gmlc/utilities/string_viewOps.cpp',
        'utilities/gmlc/utilities/vectorOps.cpp',
    ],
    install : true)
gmlc_dep = declare_dependency(include_directories : gmlc_inc, link_with : gmlc_lib)

units_inc = include_directories('units')
units_lib = library('units', [
        'units/units/units.cpp',
        'units/units/commodities.cpp',
    ],
    install : true)
units_dep = declare_dependency(include_directories : units_inc, link_with : units_lib)

cli11_inc = include_directories('cli11')
cli11_dep = declare_dependency(include_directories : cli11_inc)

subdir('headers')
