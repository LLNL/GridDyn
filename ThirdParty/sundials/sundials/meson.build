sundials_config = configuration_data({
    'F77_MANGLE_MACRO1' : '',
    'F77_MANGLE_MACRO2' : '',
    'PRECISION_LEVEL' : '#define SUNDIALS_DOUBLE_PRECISION 1',
    'INDEX_TYPE' : '#define SUNDIALS_INT64_T 1',
    'SUNDIALS_USE_GENERIC_MATH' : '',
    'SUNDIALS_HAVE_POSIX_TIMERS' : '',
    'SUNDIALS_BLAS_LAPACK' : '',
    'SUNDIALS_SUPERLUMT' : '',
    'SUNDIALS_SUPERLUMT_THREAD_TYPE' : '',
    'SUPERLUMT_THREAD_TYPE' : '',
    'SUNDIALS_KLU' : '',
    'F77_MPI_COMM_F2C' : '',
    'SUNDIALS_EXPORT' : '#define SUNDIALS_EXPORT',
    'PACKAGE_VERSION' : '',
    'PACKAGE_VERSION_LABEL' : '',
    'PACKAGE_VERSION_MAJOR' : '',
    'PACKAGE_VERSION_MINOR' : '',
    'PACKAGE_VERSION_PATCH' : '',
    })

sundials_config_file = configure_file(
    input : '../sundials-3.1.1/include/sundials/sundials_config.in',
    output : 'sundials_config.h',
    format : 'cmake@',
    configuration : sundials_config,
    install : true,
    install_dir : 'include/sundials')
