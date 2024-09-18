#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pressio_dtype.h>
#include <pressio_data.h>
#include <libpressio_ext/io/posix.h>

struct cmdline_args {
  bool any_arg;
  size_t api;
  size_t* dims;
  size_t dims_len;
  enum pressio_dtype type;
  const char *input, *output, *config_name;
};
struct cmdline_args* parse_args(int argc, char* argv[]);
struct cmdline_args* cmdline_args_new();
void cmdline_args_free(struct cmdline_args*);


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
  struct cmdline_args* args = parse_args(argc, argv);

  if (args->any_arg) {
      struct pressio_data* input_metadata = pressio_data_new_owning(args->type,  args->dims_len, args->dims);
      struct pressio_data* input = pressio_io_data_path_read(input_metadata, args->input);
      struct pressio_data* output_metadata = pressio_data_new_owning(args->type,  args->dims_len, args->dims);
      struct pressio_data* output = pressio_io_data_path_read(output_metadata, args->output);
      struct pressio_data* input_casted = pressio_data_cast(input, pressio_double_dtype);
      struct pressio_data* output_casted = pressio_data_cast(output, pressio_double_dtype);
      size_t total_dims = 1;
      for (int i = 0; i < args->dims_len; ++i) {
        total_dims *= args->dims[i];
      }

      double pre = nonzero_mean(pressio_data_ptr(input_casted, NULL), total_dims);
      double post = nonzero_mean(pressio_data_ptr(output_casted, NULL), total_dims);

      printf("pre=%le\n", pre);
      printf("post=%le\n", post);


      pressio_data_free(output_metadata);
      pressio_data_free(output);
      pressio_data_free(output_casted);
      pressio_data_free(input_metadata);
      pressio_data_free(input);
      pressio_data_free(input_casted);
  } else {
    //just return options
    printf("pre=0.0\n");
    printf("post=0.0\n");
  }

  cmdline_args_free(args);
  return 0;
}

// {{{ commandline arguments implementation
void cmdline_args_free(struct cmdline_args* args) {
  free(args->dims);
  free(args);
}
struct cmdline_args* cmdline_args_new() {
  struct cmdline_args* args = calloc(sizeof(struct cmdline_args), 1);
  if(args == NULL) {
    fprintf(stderr, "memory allocation failed\n");
    exit(1);
  }
  args->type = pressio_byte_dtype;
  args->any_arg = false;
  return args;
}

struct cmdline_args* parse_args(int argc, char* argv[]) {
  static struct option long_options[] = {
    {"api", required_argument, 0, 'a'},
    {"input", required_argument, 0, 'i'},
    {"decompressed", required_argument, 0, 'z'},
    {"dim", required_argument, 0, 'd'},
    {"type", required_argument, 0, 't'},
    {"config_name", required_argument, 0, 'c'},
    {0, 0, 0, 0}
  };
  struct cmdline_args* args = cmdline_args_new();

  int option_index = 0;
  int opt;
  //silence unknown argument warnings since future versions of libpressio may provide more arguments
  opterr = 0;
  while((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    switch(opt) {
      case 'c':
        args->config_name = optarg;
        args->any_arg = true;
        break;
      case 'a':
        args->api = atoll(optarg);
        args->any_arg = true;
        break;
      case 'z':
        args->output = optarg;
        args->any_arg = true;
        break;
      case 'i':
        args->input = optarg;
        args->any_arg = true;
        break;
      case 't':
        args->any_arg = true;
        if (strcmp(optarg, "int8") == 0) {
          args->type = pressio_int8_dtype;
        } else if(strcmp(optarg, "int16") == 0) {
          args->type = pressio_int16_dtype;
        } else if(strcmp(optarg, "int32") == 0) {
          args->type = pressio_int32_dtype;
        } else if(strcmp(optarg, "int64") == 0) {
          args->type = pressio_int64_dtype;
        } else if(strcmp(optarg, "uint8") == 0) {
          args->type = pressio_uint8_dtype;
        } else if(strcmp(optarg, "uint16") == 0) {
          args->type = pressio_uint16_dtype;
        } else if(strcmp(optarg, "uint32") == 0) {
          args->type = pressio_uint32_dtype;
        } else if(strcmp(optarg, "uint64") == 0) {
          args->type = pressio_uint64_dtype;
        } else if(strcmp(optarg, "float") == 0) {
          args->type = pressio_float_dtype;
        } else if(strcmp(optarg, "double") == 0) {
          args->type = pressio_double_dtype;
        } else if(strcmp(optarg, "byte") == 0) {
          args->type = pressio_byte_dtype;
        } else {
          fprintf(stderr, "unexpected type %s\n", optarg);
          exit(1);
        }
        break;
      case 'd':
        args->dims = realloc(args->dims, ++args->dims_len * sizeof(size_t));
        args->dims[args->dims_len-1] = atoll(optarg);
        args->any_arg = true;
        break;
      case '?':
        break;
      default:
        fprintf(stderr, "unexpected argument %c\n", opt);
        exit(1);
        break;
    }
  }
  return args;
}
// }}}

// vim: foldmethod=marker
