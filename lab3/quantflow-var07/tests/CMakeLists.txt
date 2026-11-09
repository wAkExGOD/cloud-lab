add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE quant_core)
add_test(NAME core_unit_tests COMMAND test_core)
