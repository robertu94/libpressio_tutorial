cmake_minimum_required(VERSION 3.18)
project(libpressio_gpu LANGUAGES C CXX CUDA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CUDAToolkit REQUIRED)
find_package(std_compat REQUIRED)
find_package(LibPressio REQUIRED)
find_package(ZFP REQUIRED)
set(DATADIR_REL "${CMAKE_CURRENT_SOURCE_DIR}/../../datasets")
file(REAL_PATH "${DATADIR_REL}" DATADIR)

add_executable(zfp_gpu zfp_gpu.cc)
target_link_libraries(zfp_gpu PRIVATE LibPressio::libpressio CUDA::cudart CUDA::cuda_driver zfp::zfp)
target_compile_definitions(zfp_gpu PRIVATE DATADIR="${DATADIR}")
