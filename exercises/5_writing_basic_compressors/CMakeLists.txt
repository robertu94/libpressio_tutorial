cmake_minimum_required(VERSION 3.18)
project(exercise5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(std_compat REQUIRED)
find_package(LibPressio REQUIRED)

add_library(runlength INTERFACE)
target_include_directories(runlength INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_library(libpressio_runlength_solution SHARED pressio_runlength_solution.cc)
target_link_libraries(libpressio_runlength_solution PUBLIC LibPressio::libpressio runlength)
add_library(libpressio_runlength_base SHARED pressio_runlength_base.cc)
target_link_libraries(libpressio_runlength_base PUBLIC LibPressio::libpressio runlength)

add_executable(test_pressio_base test_pressio_base.cc)
target_link_libraries(test_pressio_base PRIVATE libpressio_runlength_base)
add_executable(test_pressio_solution test_pressio_base.cc)
target_link_libraries(test_pressio_solution PRIVATE libpressio_runlength_solution)

add_executable(test_runlength test_runlength.cc)
target_link_libraries(test_runlength PRIVATE runlength)

