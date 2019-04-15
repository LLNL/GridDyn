function(libgriddyn_add_test test_name test_source_file)
  add_executable(${test_name} ${test_source_file})
  target_link_libraries(${test_name} PRIVATE Boostlibs::test)
  target_link_libraries(${test_name} PRIVATE libgriddyn)

  target_compile_definitions(${test_name} PRIVATE "-DGRIDDYN_TEST_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files\"")

  # Create test
  add_test(NAME ${test_name}
    COMMAND $<TARGET_FILE:${test_name}> -s)
endfunction()

libgriddyn_add_test(sim_management test_sim_management.cpp)
libgriddyn_add_test(bus_management test_bus_management.cpp)
libgriddyn_add_test(link_management test_link_management.cpp)
libgriddyn_add_test(load_management test_load_management.cpp)
libgriddyn_add_test(generator_management test_generator_management.cpp)
libgriddyn_add_test(powerflow test_powerflow.cpp)
