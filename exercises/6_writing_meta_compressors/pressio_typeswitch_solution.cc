#include <libpressio_ext/cpp/libpressio.h>
#include <std_compat/memory.h>
#include <sstream>

struct pressio_typeswitch : public libpressio_compressor_plugin {

  int set_options_impl(pressio_options const& opts) override {
    get_meta(opts, "typeswitch:integral", compressor_plugins(), integral_id, integral);
    get_meta(opts, "typeswitch:floating", compressor_plugins(), floating_id, floating);
    return 0;
  }
  pressio_options get_options_impl() const override {
    pressio_options opts;
    set_meta(opts, "typeswitch:integral", integral_id, integral);
    set_meta(opts, "typeswitch:floating", floating_id, floating);
    return opts;
  }
  pressio_options get_configuration_impl() const override {
    pressio_options opts;
    set_meta_configuration(opts, "typeswitch:integral", compressor_plugins(), integral);
    set_meta_configuration(opts, "typeswitch:floating", compressor_plugins(), floating);
    set(opts, "pressio:thread_safe", 
          std::min(
          pressio_configurable::get_threadsafe(*integral),
          pressio_configurable::get_threadsafe(*floating)
          ));
    set(opts, "pressio:stability", "experimental");
    return opts;
  }
  pressio_options get_documentation_impl() const override {
    pressio_options opts;
    set_meta_docs(opts, "typeswitch:integral", "compressor for integral data" , integral);
    set_meta_docs(opts, "typeswitch:floating", "compressor for floating data" , floating);
    set(opts, "pressio:description", R"(typeswitch
    Meta compressor which applies one compressor to floating point data and another to integral
    )");
    return opts;
  }

  int compress_impl(pressio_data const* input, pressio_data* output) override {
    switch(input->dtype()) {
      case pressio_float_dtype:
      case pressio_double_dtype:
        return floating->compress(input, output);
      default:
        return integral->compress(input, output);
    }
  }
  int decompress_impl(pressio_data const* input, pressio_data* output) override {
    switch(output->dtype()) {
      case pressio_float_dtype:
      case pressio_double_dtype:
        return floating->compress(input, output);
      default:
        return integral->compress(input, output);
    }
  }

  void set_name_impl(std::string const& name) override {
    if(!name.empty()) {
        integral->set_name(name + "/" + integral->prefix());
        floating->set_name(name + "/" + floating->prefix());
    } else {
        integral->set_name(name);
        floating->set_name(name);
    }
  }
  std::vector<std::string> children_impl() const final {
      return { floating->get_name(), integral->get_name() };
  }

  pressio_options get_metrics_results_impl() const override {
    pressio_options opts;
    opts.copy_from(integral->get_metrics_results());
    opts.copy_from(floating->get_metrics_results());
    return opts;
  }

  int major_version() const override {
    return 0;
  }
  int minor_version() const override {
    return 0;
  }
  int patch_version() const override {
    return 0;
  }

  const char* prefix() const override {
    return "typeswitch";
  }
  const char* version() const override {
    static const std::string s = [this]{
      std::stringstream ss;
      ss << major_version() << '.' << minor_version() << '.' << patch_version();
      return ss.str();
    }();
    return s.c_str();
  }

  std::shared_ptr<libpressio_compressor_plugin> clone() override {
    return compat::make_unique<pressio_typeswitch>(*this);
  }

  pressio_compressor integral = compressor_plugins().build("noop");
  pressio_compressor floating = compressor_plugins().build("noop");
  std::string integral_id = "noop";
  std::string floating_id = "noop";
};

pressio_register pressio_typeswitch_register(compressor_plugins(), "typeswitch", []{
      return compat::make_unique<pressio_typeswitch>();
    });
