#include <libpressio_ext/cpp/libpressio.h>
#include <iostream>

int main(int argc, char *argv[])
{

    pressio library;

    pressio_data metadata = pressio_data::empty(pressio_float_dtype, {500,500,100});
    pressio_io posix = library.get_io("posix");
    posix->set_options({{"io:path", DATADIR "CLOUDf48.bin.f32"}});
    pressio_data* input_data = posix->read(&metadata);
    pressio_data compressed = pressio_data::empty(pressio_byte_dtype, {});
    pressio_data output = pressio_data::owning(input_data->dtype(), input_data->dimensions());


    double bounds[] = {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1};
    for (auto bound : bounds) {
        pressio_compressor compressor = library.get_compressor("sz");
        compressor->set_options({
            {"pressio:abs", bound},
            {"pressio:metric", "composite"},
            {"composite:plugins", std::vector<std::string>{"size", "error_stat", "time"}}
        });

        if(compressor->compress(input_data, &compressed)) {
            std::cerr << compressor->error_msg() << std::endl;
            return compressor->error_code();
        }
        if(compressor->decompress(&compressed, &output)) {
            std::cerr << compressor->error_msg() << std::endl;
            return compressor->error_code();
        }

        std::cout << "bound=" << bound << " metrics=" << compressor->get_metrics_results() << std::endl;
    }

    delete input_data;
    return 0;
}
