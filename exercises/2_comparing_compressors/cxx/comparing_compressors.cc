#include <mpi.h>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <variant>
#include <libdistributed/libdistributed_work_queue.h>
#include <libpressio_ext/cpp/libpressio.h>
#include <libpressio_ext/cpp/serializable.h>
#include <libpressio_ext/cpp/printers.h>
#include <fmt/std.h>
#include <fmt/ostream.h>
#include <fmt/base.h>
#include <fmt/format.h>


template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
using namespace std::string_literals;
using success = pressio_options;
using failure = std::pair<int, std::string>;
using eval_result = std::variant<success, failure>;

pressio_data* input;


eval_result
run_compressor(std::tuple<double, std::string> const& args) {
    auto const& [bound, compressor_id] = args;
    pressio library;
    pressio_data compressed = pressio_data::empty(pressio_byte_dtype, {});
    pressio_data output = pressio_data::clone(*input);
    pressio_compressor compressor = library.get_compressor(compressor_id);
    compressor->set_options({
        {"pressio:metric", "composite"s},
        {"composite:plugins", std::vector{"time"s, "size"s, "error_stat"s, "external"s}},
        {"external:config_name", std::format("{}-{}", compressor_id, bound)},
        {"external:command", SCRIPTDIR "visualize.py"s}
    });
    if(compressor->compress(input, &compressed) < 0) {
        return failure{compressor->error_code(), compressor->error_msg()};
    }
    if(compressor->decompress(&compressed, &output) < 0) {
        return failure{compressor->error_code(), compressor->error_msg()};
    }
    return success{compressor->get_metrics_results()};
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    std::vector<std::tuple<double, std::string>> configurations;
    for(auto compressor: std::vector{"sz", "zfp"}) {
    for(auto exp = -6; exp < -1; ++exp) {
        double bound = std::pow(10, exp);
        configurations.emplace_back(bound, compressor);
    }}

    pressio library;
    auto io = library.get_io("posix");
    io->set_options({
        {"io:path", DATADIR "CLOUDf48.bin.f32"s}
    });
    auto metadata = pressio_data::empty(pressio_float_dtype, {500,500,100});
    input = io->read(&metadata);
    if(!input) {
        fmt::println("failed to read input {}", io->error_msg());
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    distributed::queue::work_queue(
        MPI_COMM_WORLD,
        std::begin(configurations),
        std::end(configurations),
        run_compressor,
        [](eval_result const& result, auto& stop_token) {
            std::visit(overloaded{
                [](success const& s) {
                    std::cout << s << std::endl;
                },
                [&](failure const& f) {
                    fmt::println("failed: ({}): {}", f.first, f.second);
                    stop_token.request_stop();
                }
            }, result);
    });
    
    MPI_Finalize();
}
