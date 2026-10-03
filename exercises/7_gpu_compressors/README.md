# Using GPU Compressors

The goal of this exercise is to use a libpressio GPU compressor with both

1. with memory allocated on host memory
2. with memory allocate on the GPU in advance without first copying it to the GPU.

Right now this only supports Nvidia cuda powered compressors (mostly due to a lack of implementations for other platforms).

## Automatic-Copy Compresion: Allocating Memory and Registering it with LibPressio

Allocate host memory as normal, but use a compressor designed or configured for use on the GPU.  Everything else is the same as before.

## Zero-Copy Compresion: Allocating Memory and Registering it with LibPressio

Currently this is only supported in the low-level APIs and requires calling certain CUDA functions directly.

First allocate the memory with `cudaMallocManaged`, then register it with libpressio using either `pressio_data_new_nonowning` or `pressio_data_new_move`.
If you register the data with `pressio_data_new_move` be sure to provide a free function that de-allocates the memory.

Right now, you will need to call certain compressor specific functions to get the size of the data needed -- this will be addressed in a future release.

Lastly, call compress and decompress.

This is illustrated in `c/zfp_gpu.c`

## Using FSZ

`c/fsz_gpu.c`, `cxx/fsz_gpu.cc`, and `python/fsz_gpu.py` repeat the zero-copy exercise with [FSZ](https://github.com/JiajunHuang1999/FSZ), a GPU error-bounded lossy compressor.
They need LibPressio built with `-DLIBPRESSIO_HAS_CUDA=ON -DLIBPRESSIO_HAS_FSZ=ON` (see `docs/UsingTheFSZCompressor.md` in LibPressio).

FSZ is configured with the absolute error bound `pressio:abs`.
Compressing an input without data reports the size of the compressed buffer to allocate, so no compressor specific functions are needed.
Compressed and decompressed data stay in GPU memory. Host inputs also work and are copied to the GPU automatically.

```bash
cmake -S c -B build-c -DCMAKE_PREFIX_PATH=/path/to/install
cmake --build build-c
./build-c/fsz_gpu
```
