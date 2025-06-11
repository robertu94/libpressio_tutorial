#include <stdio.h>
#include <stdlib.h>
#include <libpressio.h>
#include <libpressio_ext/io/posix.h>

int main(int argc, char *argv[])
{
  //read in the dataset
  size_t dims[] = {100, 500, 500};
  size_t ndims = sizeof(dims)/sizeof(dims[0]);
  struct pressio_data* metadata = pressio_data_new_empty(pressio_float_dtype, ndims, dims);
  struct pressio_data* input_data = pressio_io_data_path_read(metadata, DATADIR "CLOUDf48.bin.f32");

  //create output locations
  struct pressio_data* compressed = pressio_data_new_empty(pressio_byte_dtype, 0, NULL);
  struct pressio_data* output = pressio_data_new_clone(input_data);

  //get the compressor
  struct pressio* library = pressio_instance();
  struct pressio_compressor* comp = pressio_get_compressor(library, "sz");

  //configure metrics for the compressor
  struct pressio_options* metrics_options = pressio_options_new();
  const char* metrics_ids[] = {"size", "error_stat", "time",};
  size_t n_metrics_ids = sizeof(metrics_ids)/sizeof(metrics_ids[0]);
  pressio_options_set_string(metrics_options, "pressio:metric", "composite");
  pressio_options_set_strings(metrics_options, "composite:plugins", n_metrics_ids, metrics_ids);
  //verify that options passed exist
  if(pressio_compressor_check_options(comp, metrics_options)) {
    fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
    return pressio_compressor_error_code(comp);
  }

  //set the new options for the compressor
  if(pressio_compressor_set_options(comp, metrics_options)) {
    fprintf(stderr, "%s\n", pressio_compressor_error_msg(comp));
    return pressio_compressor_error_code(comp);
  }
  pressio_options_free(metrics_options);

  double bounds[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1};
  size_t n_bounds = sizeof(bounds)/sizeof(bounds[0]);
  for (size_t i = 0; i < n_bounds; ++i) {
    //configure the compressor error bound
    struct pressio_options* options = pressio_options_new();
    pressio_options_set_double(options, "pressio:abs", bounds[i]);

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
    pressio_options_free(metrics_results);
  }


  pressio_data_free(metadata);
  pressio_data_free(input_data);
  pressio_data_free(compressed);
  pressio_data_free(output);
  pressio_compressor_release(comp);
  pressio_release(library);
  return 0;
}
