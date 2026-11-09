if(QUANT_BUILD_SHARED)
  add_library(quant_core SHARED)
else()
  add_library(quant_core STATIC)
endif()

target_sources(quant_core PRIVATE
  src/stats.cpp
  src/indicators.cpp
  src/tickgen.cpp)

target_include_directories(quant_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

target_compile_options(quant_core PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS quant_core ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
