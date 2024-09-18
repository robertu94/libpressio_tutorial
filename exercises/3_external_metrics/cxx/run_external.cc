#include <iostream>
#include <string>
#include <libpressio.h>
#include <libpressio_ext/cpp/data.h>
#include <libpressio_ext/cpp/pressio.h>
#include <libpressio_ext/cpp/compressor.h>
#include <libpressio_ext/cpp/printers.h>
#include <libpressio_ext/io/posix.h>

using namespace std::string_literals;

int main(int argc, char *argv[])
{
  //read in the dataset
  std::vector<size_t> dims {100, 500, 500};
  pressio_data metadata = pressio_data::empty(pressio_float_dtype, dims);
  pressio_data* input_data = pressio_io_data_path_read(&metadata, DATADIR "CLOUDf48.bin.f32");

  //create output locations
  pressio_data compressed = pressio_data::empty(pressio_byte_dtype, {});
  pressio_data output = pressio_data::clone(*input_data);

  //get the compressor
  pressio library;
  pressio_compressor comp = library.get_compressor("sz");


  //---------------------------------NEW----------------------------------------------

  //configure metrics for the compressor
  struct pressio_options metrics_options{
      { "pressio:metric", "composite"s},
      { "composite:plugins", std::vector<std::string>{"external"}},
      { "external:command", SCRIPTDIR "/external_metric"}
  };
  const char* metrics_ids[] = {"external"};

  //---------------------------------END NEW----------------------------------------------


  //verify that options passed exist
  if(comp->check_options(metrics_options)) {
    std::cerr << comp->error_msg() << std::endl;
    return comp->error_code();
  }

  //set the new options for the compressor
  if(comp->set_options(metrics_options)) {
    std::cerr << comp->error_msg() << std::endl;
    return comp->error_code();
  }

  double bounds[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1};
  size_t n_bounds = sizeof(bounds)/sizeof(bounds[0]);
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
    pressio_options metrics_results = comp->get_metrics_results();
    std::cout << "bound=" << bounds[i] << std::endl << metrics_results << std::endl;
  }


  delete input_data;
  return 0;
}
