add_executable(quant-cli main.cpp)
target_link_libraries(quant-cli PRIVATE quant_core)
install(TARGETS quant-cli RUNTIME DESTINATION bin)
