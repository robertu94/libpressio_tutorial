#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <cstddef>
#include <math.h>
#include <pressio_dtype.h>
#include <libpressio_ext/cpp/data.h>
#include <libpressio_ext/io/posix.h>

using namespace std::string_literals;
struct cmdline_args {
  bool any_arg;
  size_t api;
  std::vector<size_t> dims;
  enum pressio_dtype type;
  std::string input, output, config_name;
};
cmdline_args parse_args(int argc, char* argv[]);

double nonzero_mean(const double* data, const size_t n) {
  double mean = 0;
  size_t nnz = 0;
  for (size_t i = 0; i < n; ++i) {
    if(data[i] == 0) continue;
    mean += data[i];
    ++nnz;
  }
  mean /= nnz;
  return mean;
}


int main(int argc, char *argv[])
{
  printf("external:api=5\n");
  cmdline_args args = parse_args(argc, argv);

  if (args.any_arg) {
      pressio_data input_metadata = pressio_data::owning(args.type,  args.dims);
      pressio_data output_metadata = pressio_data::owning(args.type,  args.dims);
      pressio_data* input = pressio_io_data_path_read(&input_metadata, args.input.c_str());
      pressio_data* output = pressio_io_data_path_read(&output_metadata, args.output.c_str());
      pressio_data input_casted = input->cast(pressio_double_dtype);
      pressio_data output_casted = output->cast(pressio_double_dtype);

      double pre = nonzero_mean((double*)input_casted.data(), input_casted.num_elements());
      double post = nonzero_mean((double*)output_casted.data(), output_casted.num_elements());

      std::cout << "pre=" << pre << std::endl;
      std::cout << "post=" << post << std::endl;

      delete output;
      delete input;
  } else {
      std::cout << "pre=0.0" << std::endl;
      std::cout << "post=0.0" << std::endl;
  }

  return 0;
}

// {{{ commandline arguments implementation
struct cmdline_args cmdline_args_new() {
  cmdline_args args;
  args.type = pressio_byte_dtype;
  args.any_arg = false;
  return args;
}

cmdline_args parse_args(int argc, char* argv[]) {
  static struct option long_options[] = {
    {"api", required_argument, 0, 'a'},
    {"input", required_argument, 0, 'i'},
    {"decompressed", required_argument, 0, 'z'},
    {"dim", required_argument, 0, 'd'},
    {"type", required_argument, 0, 't'},
    {"config_name", required_argument, 0, 'c'},
    {0, 0, 0, 0}
  };
  struct cmdline_args args;

  int option_index = 0;
  int opt;
  //silence unknown argument warnings since future versions of libpressio may provide more arguments
  opterr = 0;
  while((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    switch(opt) {
      case 'c':
        args.config_name = optarg;
        args.any_arg = true;
        break;
      case 'a':
        args.api = atoll(optarg);
        args.any_arg = true;
        break;
      case 'z':
        args.output = optarg;
        args.any_arg = true;
        break;
      case 'i':
        args.input = optarg;
        args.any_arg = true;
        break;
      case 't':
        args.any_arg = true;
        if (optarg == "int8"s) {
          args.type = pressio_int8_dtype;
        } else if(optarg == "int16"s) {
          args.type = pressio_int16_dtype;
        } else if(optarg == "int32"s) {
          args.type = pressio_int32_dtype;
        } else if(optarg == "int64"s) {
          args.type = pressio_int64_dtype;
        } else if(optarg == "uint8"s) {
          args.type = pressio_uint8_dtype;
        } else if(optarg == "uint16"s) {
          args.type = pressio_uint16_dtype;
        } else if(optarg == "uint32"s) {
          args.type = pressio_uint32_dtype;
        } else if(optarg == "uint64"s) {
          args.type = pressio_uint64_dtype;
        } else if(optarg == "float"s) {
          args.type = pressio_float_dtype;
        } else if(optarg == "double"s) {
          args.type = pressio_double_dtype;
        } else if(optarg == "byte"s) {
          args.type = pressio_byte_dtype;
        } else {
            std::cerr << "unexpected type" << optarg << std::endl;
          exit(1);
        }
        break;
      case 'd':
        args.dims.emplace_back(atoll(optarg));
        args.any_arg = true;
        break;
      case '?':
        break;
      default:
        std::cerr << "unexpected argument " << opt << std::endl;
        exit(1);
        break;
    }
  }
  return args;
}
// }}}

// vim: foldmethod=marker
