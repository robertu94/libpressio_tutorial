cmake_minimum_required(VERSION 3.18)
project(exercise3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(std_compat REQUIRED)
find_package(LibPressio REQUIRED)

get_filename_component(DATADIR "${CMAKE_CURRENT_SOURCE_DIR}/../../datasets/" ABSOLUTE)
message(WARNING ${DATADIR})

function(make_exercise src)
    get_filename_component(target "${src}" NAME_WLE)
    add_executable("${target}" "${src}")
    target_link_libraries("${target}" PRIVATE LibPressio::libpressio)
    target_compile_definitions("${target}" PRIVATE DATADIR="${DATADIR}/")
    target_compile_definitions("${target}" PRIVATE SCRIPTDIR="${CMAKE_CURRENT_BINARY_DIR}/")
endfunction()

make_exercise(external_metric.cc)
make_exercise(run_external.cc)
