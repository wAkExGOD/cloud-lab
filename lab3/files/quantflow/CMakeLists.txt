cmake_minimum_required(VERSION 3.20)
project(quantflow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QUANT_BUILD_SHARED "Собирать quant_core как динамическую библиотеку" OFF)
option(QUANT_BUILD_TESTS  "Собирать модульные тесты" ON)

add_subdirectory(core)
add_subdirectory(apps/quant-cli)

if(QUANT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
