# FZ Tutorial

This repository contains a number of example applications to help you learn how
to use FZ via LibPressio for lossy compression.  The exercises are located in `exercises/`
and have their own instructions in the README.md file.

When cloning the repo, be sure to clone the submodules

```bash
git clone --recursive https://github.com/robertu94/libpressio_tutorial
```

The easiest way to get started is with the container image.
You will need one of them installed.  If you are Linux, you may already have
podman installed

+ [installing docker](https://docs.docker.com/engine/install/)
+ [installing podman](https://podman.io/getting-started/installation.html)


```bash
docker run -it ghcr.io/robertu94/libpressio_tutorial:latest
```

## Following the Tutorial


The tutorial has 7 exercises that build on one another (in the `exercises` subdirectory).
Each tutorial has a `README.md` that explains the exercise and offers some suggestsions on some further exercises to test each topic.

Then there are subdirectories under each exercise that show how to run the exercise with each programming language.
Not all exercies are implemented in all languages and may have slightly different output depending on what was easy to implement.
For C and C++ there are both Makefile and CMake based build systems.
Feel free to use either of them.
PRs are accepted to add additional languages to the exercies.

Here are the major topics covered by the tutorial.
Topics 1-3 are more introductory, and topics 4-7 are more advanced.

+ [`1_basics`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/1_basics) how to initalize, query options, and use compressors
+ [`2_comparing_compressors`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/2_comparing_compressors) how to compare multiple compressors simultainously
+ [`3_external_metrics`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/3_external_metrics) how to connect metrics from a user's code not written in C/C++
+ [`4_libpressio_opt`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/4_libpressio_opt) how to use libpressio_opt to tune a compressors settings automatically
+ [`5_writiing_basic_compressors`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/5_writing_basic_compressors) how to write your own compressor
+ [`6_writiing_meta_compressors`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/6_writing_meta_compressors) how to write a meta-compressor
+ [`7_gpu_compressors`](https://github.com/robertu94/libpressio_tutorial/tree/master/exercises/7_gpu_copmressors) how to compress memory that was allocated on a Nvidia GPU.


## Manual installation

The container is [build with spack](https://github.com/robertu94/libpressio/blob/master/docker/spack.yaml) and based on the [libpressio container](https://github.com/robertu94/libpressio/blob/master/docker/Dockerfile).

To build and update the container image, run:

```bash
# clean tutorial directories and build image
./build_container.py

#update image
docker login ghcr.io
docker push ghcr.io/robertu94/libpressio_tutorial:latest
```
