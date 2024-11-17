cmake_minimum_required(VERSION 3.18)
project(exercise2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(std_compat REQUIRED)
find_package(LibPressio REQUIRED)
find_package(LibDistributed REQUIRED)

get_filename_component(DATADIR "${CMAKE_CURRENT_SOURCE_DIR}/../../datasets/" ABSOLUTE)
get_filename_component(SCRIPTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../" ABSOLUTE)
message(WARNING ${DATADIR})

function(make_exercise src)
    get_filename_component(target "${src}" NAME_WLE)
    add_executable("${target}" "${src}")
    target_link_libraries("${target}" PRIVATE LibPressio::libpressio LibDistributed::libdistributed)
    target_compile_definitions("${target}" PRIVATE DATADIR="${DATADIR}/")
    target_compile_definitions("${target}" PRIVATE SCRIPTDIR="${SCRIPTDIR}/")
endfunction()

make_exercise(comparing_compressors.cc)
