#include <libpressio_ext/cpp/libpressio.h>
#include <std_compat/memory.h>
#include <sstream>
#include "runlength.h"

struct run_encode {
  template <class T>
  pressio_data operator()(T const* first, T const* last) const noexcept {
    std::string s = compress(first, std::distance(first, last), static_cast<T>(value_to_encode));
    return pressio_data::copy(
        pressio_byte_dtype,
        s.data(),
        {s.size()}
        );
  }
  double value_to_encode;
};

struct run_decode {
  template <class T>
  void operator()(T* first, T* last) const noexcept {
    auto vec = decompress<T>(std::move(s), static_cast<T>(value_to_encode));
    std::copy(vec.begin(), vec.end(), first);
  }
  double value_to_encode;
  std::string && s;
};

struct pressio_runlength : public libpressio_compressor_plugin {

  int set_options_impl(pressio_options const& opts) override {
    get(opts, "runlength:value_to_encode", &value_to_encode);
    return 0;
  }
  pressio_options get_options_impl() const override {
    pressio_options opts;
    set(opts, "runlength:value_to_encode", value_to_encode);
    return opts;
  }
  pressio_options get_configuration_impl() const override {
    pressio_options opts;
    set(opts, "pressio:thread_safe", pressio_thread_safety_multiple);
    set(opts, "pressio:stability", "experimental");
    return opts;
  }
  pressio_options get_documentation_impl() const override {
    pressio_options opts;
    set(opts, "pressio:description", R"(runlength encoding
    Removes the specified value where it appears replacing it with a single value
    )");
    set(opts, "runlength:value_to_encode", "the value to remove");
    return opts;
  }

  int compress_impl(pressio_data const* real_input, pressio_data* output) override {
    pressio_data input = domain_manager().make_readable(domain_plugins().build("malloc"), *real_input);
    *output = pressio_data_for_each<pressio_data>(input, run_encode{value_to_encode});
    return 0;
  }
  int decompress_impl(pressio_data const* input, pressio_data* output) override {
    std::string str(static_cast<const char*>(input->data()), input->size_in_bytes());
    pressio_data_for_each<void>(*output, run_decode{value_to_encode, std::move(str)});
    return 0;
  }

  int major_version() const override {
    return RUNLEGTH_MAJOR_VERSION;
  }
  int minor_version() const override {
    return RUNLEGTH_MINOR_VERSION;
  }
  int patch_version() const override {
    return RUNLEGTH_PATCH_VERSION;
  }

  const char* prefix() const override {
    return "runlength";
  }
  const char* version() const override {
    static std::string s = []{
      std::stringstream ss;
      ss << RUNLEGTH_MAJOR_VERSION << '.' << RUNLEGTH_MINOR_VERSION << '.' << RUNLEGTH_PATCH_VERSION;
      return ss.str();
    }();
    return s.c_str();
  }

  std::shared_ptr<libpressio_compressor_plugin> clone() override {
    return std::make_unique<pressio_runlength>(*this);
  }

  double value_to_encode = 0.0;
};

pressio_register pressio_runlength_register(compressor_plugins(), "runlength", []{
      return compat::make_unique<pressio_runlength>();
    });
