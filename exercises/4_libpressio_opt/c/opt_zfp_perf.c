#include <stdio.h>
#include <stdlib.h>
#include <libpressio.h>
#include <libpressio_meta.h>
#include <libpressio_ext/io/posix.h>

int main(int argc, char *argv[])
{
  libpressio_register_all();

  //read in the dataset
  size_t dims[] = {100, 500, 500};
  size_t ndims = sizeof(dims)/sizeof(dims[0]);
  struct pressio_data* metadata = pressio_data_new_empty(pressio_float_dtype, ndims, dims);
  struct pressio_data* input_data = pressio_io_data_path_read(metadata, DATADIR "CLOUDf48.bin.f32");
  if(input_data == NULL) {
    fprintf(stderr, "failed to file input_data at \"" DATADIR "CLOUDf48.bin.f32\" make sure it is there\n" \
        "you may need to install git-lfs to download the datasets\n");
    exit(1);
  }

  //create output locations
  struct pressio_data* compressed = pressio_data_new_empty(pressio_byte_dtype, 0, NULL);
  struct pressio_data* output = pressio_data_new_clone(input_data);

  //get the compressor
  struct pressio* library = pressio_instance();
  struct pressio_compressor* comp = pressio_get_compressor(library, "opt");
  if(comp == NULL) {
    fprintf(stderr, "failed to load compressor: %s\n", pressio_error_msg(library));
    fprintf(stderr, "checked to ensure that liblibpressio_opt.so was loaded into the binary using readelf -d ./opt_zfp_perf\n");
    exit(1);
  }

  //configure metrics for the compressor
  struct pressio_options* search_configuration = pressio_options_new();
  const char* metrics_ids[] = {"time", "size", "error_stat"};
  size_t n_metrics_ids = sizeof(metrics_ids)/sizeof(metrics_ids[0]);
  pressio_options_set_strings(search_configuration, "composite:plugins", n_metrics_ids, metrics_ids);

  pressio_options_set_integer(search_configuration, "opt:do_decompress", 0);
  pressio_options_set_string(search_configuration, "opt:compressor", "zfp");
  pressio_options_set_string(search_configuration, "opt:metric", "composite");
  pressio_options_set_string(search_configuration, "opt:objective_mode_name", "min");
  pressio_options_set_string(search_configuration, "opt:search", "fraz");
  pressio_options_set_string(search_configuration, "zfp:execution_name", "omp");
  pressio_options_set_string(search_configuration, "zfp:metric", "time");
  pressio_options_set_uinteger(search_configuration, "opt:max_iterations", 12);

  const char* inputs[] = {"zfp:omp_threads", "zfp:omp_chunk_size"};
  pressio_options_set_strings(search_configuration, "opt:inputs", sizeof(inputs)/sizeof(inputs[0]), inputs);

  const char* outputs[] = {"time:compress_many"};
  pressio_options_set_strings(search_configuration, "opt:output", sizeof(outputs)/sizeof(outputs[0]), outputs);

  double upper_bound_arr[] = {12, 512};
  size_t upper_bound_arr_size = sizeof(upper_bound_arr)/sizeof(upper_bound_arr[0]);
  struct pressio_data* upper_bound = pressio_data_new_copy(pressio_double_dtype, upper_bound_arr, 1, &upper_bound_arr_size);
  pressio_options_set_data(search_configuration, "opt:upper_bound", upper_bound);

  double lower_bound_arr[] = {1, 1};
  size_t lower_bound_arr_size = sizeof(lower_bound_arr)/sizeof(lower_bound_arr[0]);
  struct pressio_data* lower_bound = pressio_data_new_copy(pressio_double_dtype, lower_bound_arr, 1, &lower_bound_arr_size);
  pressio_options_set_data(search_configuration, "opt:lower_bound", lower_bound);
  
  int32_t is_integral_arr[] = {1, 1};
  size_t is_integral_arr_size = sizeof(is_integral_arr)/sizeof(is_integral_arr[0]);
  struct pressio_data* is_integral = pressio_data_new_copy(pressio_int32_dtype, is_integral_arr, 1, &lower_bound_arr_size);
  pressio_options_set_data(search_configuration, "opt:is_integral", is_integral);


  //set the new options for the compressor
  if(pressio_compressor_set_options(comp, search_configuration)) {
    fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
    return pressio_compressor_error_code(comp);
  }
  pressio_options_free(search_configuration);

  double bounds[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1};
  size_t n_bounds = sizeof(bounds)/sizeof(bounds[0]);
  bool first = true;
  for (size_t i = 0; i < n_bounds; ++i) {
    //configure the compressor error bound
    struct pressio_options* options = pressio_options_new();
    pressio_options_set_double(options, "zfp:accuracy", bounds[i]);

    //verify that options passed exist
    if(pressio_compressor_check_options(comp, options)) {
      fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
      return pressio_compressor_error_code(comp);
    }

    //set the new options for the compressor
    if(pressio_compressor_set_options(comp, options)) {
      fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
      return pressio_compressor_error_code(comp);
    }
    pressio_options_free(options);

    //run the compression and decompression
    if(pressio_compressor_compress(comp, input_data, compressed)) {
      fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
      return pressio_compressor_error_code(comp);
    }
    if(pressio_compressor_decompress(comp, compressed, output)) {
      fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
      return pressio_compressor_error_code(comp);
    }

    //print out the metrics results in a human readable form
    struct pressio_options* metrics_results = pressio_compressor_get_metrics_results(comp);
    char* metrics_results_str = pressio_options_to_string(metrics_results);
    printf("bound=%1.1e\n%s\n", bounds[i], metrics_results_str);
    free(metrics_results_str);

    //if we haven't found a guess yet, apply the last solution
    if(first) {
      first=false;
      struct pressio_data* guess = pressio_data_new_empty(pressio_byte_dtype, 0, NULL);
      pressio_options_get_data(metrics_results, "opt:input", &guess);

      struct pressio_options* guess_ops = pressio_options_new();
      pressio_options_set_string(guess_ops, "opt:search", "guess");
      pressio_options_set_string(guess_ops, "opt:search_metrics", "composite_search");
      pressio_options_set_data(guess_ops, "opt:prediction", guess);
      if(pressio_compressor_set_options(comp, guess_ops)) {
        fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
        return pressio_compressor_error_code(comp);
      }
      pressio_data_free(guess);
      pressio_options_free(guess_ops);
    }


    pressio_options_free(metrics_results);
  }


  pressio_data_free(upper_bound);
  pressio_data_free(lower_bound);
  pressio_data_free(is_integral);
  pressio_data_free(metadata);
  pressio_data_free(input_data);
  pressio_data_free(compressed);
  pressio_data_free(output);
  pressio_compressor_release(comp);
  pressio_release(library);
  return 0;
}
