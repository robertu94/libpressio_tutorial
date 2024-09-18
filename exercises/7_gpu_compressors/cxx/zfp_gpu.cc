#include <iostream>
#include <vector>
#include <cstddef>
#include <string>
#include <libpressio.h>
#include <libpressio_ext/io/posix.h>
#include <libpressio_ext/cpp/compressor.h>
#include <libpressio_ext/cpp/domain.h>
#include <libpressio_ext/cpp/domain_manager.h>
#include <libpressio_ext/cpp/data.h>
#include <libpressio_ext/cpp/pressio.h>
#include <libpressio_ext/cpp/printers.h>

using namespace std::string_literals;

int main(int argc, char *argv[])
{
  pressio library;
  pressio_compressor comp = library.get_compressor("zfp");

  printf("%s\n", DATADIR "/CLOUDf48.bin.f32");

  //read in the dataset
  std::vector<size_t> dims{500, 500, 100};
  domain_manager().set_options({{"domain:metrics", {"print"s}}});
  pressio_data metadata = pressio_data::owning(pressio_float_dtype, dims, domain_plugins().build("cudamallochost"));
  pressio_data* input_data = pressio_io_data_path_read(&metadata, DATADIR "/CLOUDf48.bin.f32");
  pressio_data input_data_shared = domain_manager().make_readable(domain_plugins().build("cudamalloc"), std::move(*input_data));
  pressio_data output_shared = pressio_data::clone(input_data_shared);

  //configure compressor
  double rate = 16.0;
  pressio_options options{
      {"zfp:rate", rate},
      {"zfp:execution_name", "cuda"s},
      {"pressio:metric", "composite"s},
      {"composite:plugins", std::vector{"time"s,"size"s}},
  };
  comp->set_options(options);

  //determine the size of the compressed buffer
  pressio_data to_compress_size =  pressio_data::empty(pressio_float_dtype, dims);
  pressio_data compressed_estimate =  pressio_data::empty(pressio_byte_dtype, {});
  if(comp->compress(&to_compress_size, &compressed_estimate)) {
      std::cerr << "failed to estimate size: " << comp->error_msg() << std::endl;
    exit(1);
  }

  //pre-allocate the compressed buffer
  pressio_data comp_shared = pressio_data::owning(pressio_byte_dtype, {compressed_estimate.size_in_bytes()}, domain_plugins().build("cudamalloc"));
  
  //now run the actual compression
  if(comp->compress(&input_data_shared, &comp_shared)) {
      std::cerr << "failed to compress: " << comp->error_msg() << std::endl;
    exit(1);
  }
  if(comp->decompress(&comp_shared, &output_shared)) {
      std::cerr << "failed to decompress: " << comp->error_msg() << std::endl;
    exit(1);
  }

  //copy back to the host from the GPU
  pressio_data host_output = domain_manager().make_readable(domain_plugins().build("malloc"), std::move(output_shared));

  std::cout << comp->get_metrics_results() << std::endl;

  return 0;
}
