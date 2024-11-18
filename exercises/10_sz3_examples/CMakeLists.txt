cmake_minimum_required(VERSION 3.18)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(sz3demo)

find_package(SZ3 REQUIRED)

add_executable(sz3demo sz3demo.cpp)

find_package(OpenMP)
if (OpenMP_FOUND)
    target_link_libraries(sz3demo PUBLIC OpenMP::OpenMP_CXX)
endif ()

find_package(PkgConfig)
pkg_search_module(ZSTD IMPORTED_TARGET libzstd)
if (ZSTD_FOUND)
    target_link_libraries(sz3demo PUBLIC PkgConfig::ZSTD)
else ()
    target_link_libraries(sz3demo PUBLIC SZ3::zstd)
endif ()

target_link_libraries(sz3demo PUBLIC SZ3::SZ3)