#include <libpressio.h>
#include <libpressio_ext/io/posix.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

void lp_cuda_free(void* ptr, void* metadata) {
  cudaFree(ptr);
}
void lp_cuda_freehost(void* ptr, void* metadata) {
  cudaFreeHost(ptr);
}

void lp_check_cuda(cudaError_t err) {
  if(err != cudaSuccess) {
    fprintf(stderr, "%s\n", cudaGetErrorString(err));
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  struct pressio* library = pressio_instance();
  struct pressio_compressor* comp = pressio_get_compressor(library, "zfp");

  printf("%s\n", DATADIR "/CLOUDf48.bin.f32");

  //read in the dataset
  size_t dims[] = {500, 500, 100}; //fortran order
  size_t ndims = sizeof(dims)/sizeof(dims[0]);
  size_t buf_size = sizeof(float);
  for (size_t i = 0; i < ndims; ++i) {
    buf_size *= dims[i];
  }

  //The C++ bindings provide an experimental mechanism to hide the details of using CUDA here.
  float* h_input;
  float* d_input;
  cudaMallocHost((void**)&h_input, buf_size);
  cudaMalloc((void**)&d_input, buf_size);
  struct pressio_data* metadata = pressio_data_new_move(pressio_float_dtype, h_input, ndims, dims, lp_cuda_freehost, NULL);
  struct pressio_data* input_data = pressio_io_data_path_read(metadata, DATADIR "/CLOUDf48.bin.f32");
  cudaMemcpy(d_input, pressio_data_ptr(input_data, NULL), buf_size, cudaMemcpyHostToDevice);
  struct pressio_data* input_data_shared = pressio_data_new_move(pressio_float_dtype, d_input, ndims, dims, lp_cuda_free, NULL);

  float* d_output;
  cudaMalloc((void**)&d_output, buf_size);
  struct pressio_data* output_shared = pressio_data_new_move(pressio_float_dtype, d_output, ndims, dims, lp_cuda_free, NULL);


  //configure compressor
  double rate = 16.0;
  struct pressio_options* options = pressio_options_new();
  pressio_options_set_double(options, "zfp:rate", rate);
  pressio_options_set_string(options, "zfp:execution_name", "cuda");
  pressio_options_set_string(options, "pressio:metric", "composite");
  const char* plugins[] ={"time","size"};
  size_t n_plugins = sizeof(plugins)/sizeof(plugins[0]);
  pressio_options_set_strings(options, "composite:plugins", n_plugins, plugins);
  pressio_compressor_set_options(comp, options);

  //determine the size of the compressed buffer
  struct pressio_data* to_compress_size =  pressio_data_new_empty(pressio_float_dtype, ndims, dims);
  struct pressio_data* compressed_estimate =  pressio_data_new_empty(pressio_byte_dtype, 0, NULL);
  if(pressio_compressor_compress(comp, to_compress_size, compressed_estimate)) {
    fprintf(stderr, "failed to estimate size: %s\n", pressio_compressor_error_msg(comp));
    exit(1);
  }
  size_t comp_buf_size = pressio_data_get_bytes(compressed_estimate);
  pressio_data_free(to_compress_size);
  pressio_data_free(compressed_estimate);

  //allocate the compressed buffer
  float* d_compressed;
  cudaMalloc((void**)&d_compressed, comp_buf_size);
  struct pressio_data* comp_shared = pressio_data_new_move(pressio_byte_dtype, d_compressed, 1, &comp_buf_size, lp_cuda_free, NULL);
  // it is also possible to use gpu direct storage here, but my experience is it's buggy
  // and requires every part of the stack to support it
  // libpressio provides a helper io module called "cufile"
  
  if(pressio_compressor_compress(comp, input_data_shared, comp_shared)) {
    fprintf(stderr, "failed to compress: %s\n", pressio_compressor_error_msg(comp));
    exit(1);
  }
  if(pressio_compressor_decompress(comp, comp_shared, output_shared)) {
    fprintf(stderr, "failed to decompress: %s\n", pressio_compressor_error_msg(comp));
    exit(1);
  }

  struct pressio_options* metrics_results = pressio_compressor_get_metrics_results(comp);
  char* metrics_results_str = pressio_options_to_string(metrics_results);
  printf("%s\n", metrics_results_str);
  free(metrics_results_str);

  pressio_data_free(metadata);
  pressio_data_free(input_data);
  pressio_data_free(comp_shared);
  pressio_data_free(output_shared);
  pressio_data_free(input_data_shared);
  pressio_options_free(options);
  pressio_compressor_release(comp);
  pressio_release(library);

  return 0;
}
