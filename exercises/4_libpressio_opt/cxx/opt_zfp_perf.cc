#include <iostream>
#include <vector>
#include <stdlib.h>
#include <libpressio_meta.h>
#include <libpressio_ext/cpp/data.h>
#include <libpressio_ext/cpp/compressor.h>
#include <libpressio_ext/cpp/pressio.h>
#include <libpressio_ext/cpp/printers.h>
#include <libpressio_ext/io/posix.h>

using namespace std::string_literals;

int main(int argc, char *argv[])
{
  libpressio_register_all();

  //read in the dataset
  pressio_data metadata = pressio_data::empty(pressio_float_dtype, {100, 500, 500});
  pressio_data* input_data = pressio_io_data_path_read(&metadata, DATADIR "CLOUDf48.bin.f32");
  if(input_data == NULL) {
      std::cerr << "failed to file input_data at \"" DATADIR "CLOUDf48.bin.f32\" make sure it is there\n" \
        "you may need to install git-lfs to download the datasets" << std::endl;
    exit(1);
  }

  //create output locations
  pressio_data compressed = pressio_data::empty(pressio_byte_dtype, {});
  pressio_data output = pressio_data::clone(*input_data);

  //get the compressor
  pressio library;
  pressio_compressor comp = library.get_compressor("opt");
  if(!comp) {
      std::cerr << "failed to load compressor: " << library.err_msg() << std::endl;
      std::cerr << "checked to ensure that liblibpressio_opt.so was loaded into the binary using readelf -d ./opt_zfp_perf" << std::endl;
    exit(1);
  }

  //configure metrics for the compressor
  pressio_options search_configuration{
      {"composite:plugins", std::vector<std::string>{"time", "size", "error_stat"}},
      {"opt:do_decompress", 0},
      {"opt:compressor", "zfp"},
      {"opt:metric", "composite"},
      {"opt:objective_mode_name", "min"},
      {"opt:search", "fraz"},
      {"zfp:execution_name", "omp"},
      {"zfp:metric", "time"},
      {"opt:max_iterations", 12u},
      {"opt:inputs", std::vector<std::string>{"zfp:omp_threads", "zfp:omp_chunk_size"}},
      {"opt:output", std::vector<std::string>{"time:compress_many"}},
      {"opt:upper_bound", pressio_data{12.0, 512.0}},
      {"opt:lower_bound", pressio_data{1.0, 1.0}},
      {"opt:is_integral", pressio_data{1, 1}},
  };

  //set the new options for the compressor
  if(comp->set_options(search_configuration)) {
    std::cerr << comp->error_msg() << std::endl;
    return comp->error_code();
  }

  double bounds[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1};
  size_t n_bounds = sizeof(bounds)/sizeof(bounds[0]);
  bool first = true;
  for (size_t i = 0; i < n_bounds; ++i) {
    //configure the compressor error bound
    pressio_options options{
        {"pressio:abs", bounds[i]}
    };

    //verify that options passed exist
    if(comp->check_options(options)) {
        std::cerr << comp->error_msg() << std::endl;
        return comp->error_code();
    }

    //set the new options for the compressor
    if(comp->set_options(options)) {
        std::cerr << comp->error_msg() << std::endl;
        return comp->error_code();
    }

    //run the compression and decompression
    if(comp->compress(input_data, &compressed)) {
        std::cerr << comp->error_msg() << std::endl;
        return comp->error_code();
    }
    if(comp->decompress(&compressed, &output)) {
        std::cerr << comp->error_msg() << std::endl;
        return comp->error_code();
    }

    //print out the metrics results in a human readable form
    struct pressio_options metrics_results = comp->get_metrics_results();
    std::cout << "bound=" << bounds[i] << std::endl << metrics_results << std::endl;

    //if we haven't found a guess yet, apply the last solution
    if(first) {
      first=false;
      pressio_data guess;
      metrics_results.get("opt:input", &guess);

      pressio_options guess_ops{
          {"opt:search", "guess"s},
          {"opt:search_metrics", "composite_search"s},
          {"opt:prediction", guess},
      };
      if(comp->set_options(guess_ops)) {
        std::cerr << comp->error_msg() << std::endl;
        return comp->error_code();
      }
    }
  }

  delete input_data;
  return 0;
}
