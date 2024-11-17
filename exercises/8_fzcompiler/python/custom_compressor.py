#!/usr/bin/env python
import libpressio as lp
import numpy as np
from pprint import pprint
from pathlib import Path
# template source {{{
SOURCE = """
INCLUDES
#include <boost/config.hpp>
#include <chrono>
#include "std_compat/memory.h"
#include "libpressio_ext/cpp/compressor.h"
#include "libpressio_ext/cpp/data.h"
#include "libpressio_ext/cpp/options.h"
#include "libpressio_ext/cpp/pressio.h"
#include "libpressio_ext/cpp/domain_manager.h"
namespace libpressio { namespace fzdemo_ns {
class fzdemo_compressor_plugin : public libpressio_compressor_plugin {
public:
  struct pressio_options get_options_impl() const override
  {
    struct pressio_options options;
    set_meta(options, "fzdemo:compressor", compressor_id, compressor);
    return options;
  }
  struct pressio_options get_configuration_impl() const override
  {
    struct pressio_options options;
    set(options, "pressio:thread_safe", pressio_thread_safety_multiple);
    set(options, "pressio:stability", "experimental");
    std::vector<std::string> invalidations {}; 
    std::vector<pressio_configurable const*> invalidation_children {&*compressor}; 
    set(options, "predictors:error_dependent", get_accumulate_configuration("predictors:error_dependent", invalidation_children, invalidations));
    set(options, "predictors:error_agnostic", get_accumulate_configuration("predictors:error_agnostic", invalidation_children, invalidations));
    set(options, "predictors:runtime", get_accumulate_configuration("predictors:runtime", invalidation_children, invalidations));
    set(options, "pressio:highlevel", get_accumulate_configuration("pressio:highlevel", invalidation_children, std::vector<std::string>{}));
    return options;
  }
  struct pressio_options get_documentation_impl() const override
  {
    struct pressio_options options;
    set_meta_docs(options, "fzdemo:compressor", "compressor to use after preprocessing", compressor);
    set(options, "pressio:description", R"()");
    return options;
  }
  int set_options_impl(struct pressio_options const& options) override
  {
    get_meta(options, "fzdemo:compressor", compressor_plugins(), compressor_id, compressor);
    return 0;
  }
  int compress_impl(const pressio_data* real_input,
                    struct pressio_data* output) override
  {
      if(real_input->dtype() != pressio_float_dtype) return set_error(1, "only  float supported in this demo");
      pressio_data input = domain_manager().make_readable(domain_plugins().build("malloc"), *real_input);
      const auto N = input.num_elements();
      auto ptr = static_cast<float*>(input.data());
      auto begin = std::chrono::steady_clock::now();
      {
      PREPROCESS
      }
      auto end = std::chrono::steady_clock::now();
      compressor->compress(&input, output);
      preprocess_ms = std::chrono::duration<double,std::milli>(end-begin).count();
      return 0;
  }
  int decompress_impl(const pressio_data* input,
                      struct pressio_data* output) override
  {
      compressor->compress(input, output);
      *output = domain_manager().make_readable(domain_plugins().build("malloc"), std::move(*output));
      const auto N = output->num_elements();
      auto ptr = static_cast<float*>(output->data());
      auto begin = std::chrono::steady_clock::now();
      {
      POSTPROCESS
      }
      auto end = std::chrono::steady_clock::now();
      postprocess_ms = std::chrono::duration<double,std::milli>(end-begin).count();
      return 0;
  }
  int major_version() const override { return 0; }
  int minor_version() const override { return 0; }
  int patch_version() const override { return 1; }
  const char* version() const override { return "0.0.1"; }
  const char* prefix() const override { return "fzdemo"; }
  pressio_options get_metrics_results_impl() const override {
    return {
        {"fzdemo:preprocess_ms", preprocess_ms},
        {"fzdemo:postprocess_ms", postprocess_ms}
    };
  }
  std::shared_ptr<libpressio_compressor_plugin> clone() override
  {
    return compat::make_unique<fzdemo_compressor_plugin>(*this);
  }
  double preprocess_ms = 0;
  double postprocess_ms = 0;
  std::string compressor_id = "noop";
  pressio_compressor compressor = compressor_plugins().build(compressor_id);
};
extern "C" BOOST_SYMBOL_EXPORT fzdemo_compressor_plugin plugin;
fzdemo_compressor_plugin plugin;
} }
"""
# }}}
compressor = lp.PressioCompressor.from_config(
    {
        "compressor_id": "pressio",
        "early_config": {
            "pressio:compressor": "poorjit",
            "poorjit:metric": "composite",
            "fzdemo:compressor": "zfp",
            "poorjit:generator": "template",
            "poorjit:pkgconfig": ["libpressio_cxx"],
            "poorjit:extra_args": ["-fopenmp", "-O3"],
            "composite:plugins": ["error_stat", "size", "time"],
            "template:source": SOURCE,
            "template:keys": ["INCLUDES", "PREPROCESS", "POSTPROCESS"],
            "template:values": [
                #includes
                """#include<cmath>""",
                #pre-processing
                """
                #pragma omp parallel for
                for (size_t i = 0; i < N; i++) {
                    ptr[i] = std::sqrt(ptr[i]);
                }""",
                #post-processing
                """
                #pragma omp parallel for
                for (size_t i = 0; i < N; i++) {
                    ptr[i] = std::pow(ptr[i],2);
                }""",
            ],
        },
        "compressor_config": {
            "pressio:abs": 1e-5
        },
    }
)
options = compressor.get_options()
options["template:source"] = "...snip..." #omit long text for clarity
pprint(options)

#testing portion
input = np.fromfile(
    Path(__file__).parent / "../../datasets/CLOUDf48.bin.f32",
    dtype=np.float32,
).reshape(100, 500, 500)[1]
decompressed = input.copy()
compressed = compressor.encode(input)
decompressed = compressor.decode(compressed, decompressed)
pprint(compressor.get_metrics())

# vim: foldmethod=marker :
