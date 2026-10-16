cmake_minimum_required(VERSION 3.16)
project(mc_var VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Одноконфигурационные генераторы (Makefile, Ninja): по умолчанию собираем Release
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Тип сборки" FORCE)
endif()

find_package(Threads REQUIRED)
add_executable(mc-var main.cpp)
target_link_libraries(mc-var PRIVATE Threads::Threads)
