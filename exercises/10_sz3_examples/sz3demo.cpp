#include "SZ3/api/sz.hpp"
#include "Decomposition/TruncationDecomposition.hpp"
#include "Decomposition/SZBioMDDecomposition.hpp"

using namespace SZ3;

enum DEMO_MODE { TRUNCATION, BIO };

//Please chage the demo mode here
DEMO_MODE demo = TRUNCATION;

template <class T, uint N>
size_t SZ3_customized_compress(Config &conf, T *data, char *dst,
                               size_t &outSize) {

  calAbsErrorBound(conf, data);

  if (demo == TRUNCATION) {
    auto sz = make_compressor_sz_generic<T, N>(
        TruncationDecomposition<T, N>(conf), HuffmanEncoder<int>(), Lossless_zstd());
    return sz->compress(conf, data, (uchar *)dst, outSize);
  } else if (demo == BIO) {
    auto sz = make_compressor_sz_generic<T, N>(
        SZBioMDDecomposition<T, N, LinearQuantizer<T>>(
            conf, LinearQuantizer<T>(conf.absErrorBound, 32768)),
        HuffmanEncoder<int>(), Lossless_zstd());
    return sz->compress(conf, data, (uchar *)dst, outSize);
  }
}

template <class T, uint N>
void SZ3_customized_decompress(const Config &conf, const char *cmpData,
                               size_t cmpSize, T *decData) {
  uchar const *cmpDataPos = (uchar *)cmpData;

  if (demo == TRUNCATION) {
    auto sz = make_compressor_sz_generic<T, N>(
        TruncationDecomposition<T, N>(conf), HuffmanEncoder<int>(), Lossless_zstd());
    sz->decompress(conf, cmpDataPos, cmpSize, decData);
  } else if (demo == BIO) {
    auto sz = make_compressor_sz_generic<T, N>(
        SZBioMDDecomposition<T, N, LinearQuantizer<T>>(
            conf, LinearQuantizer<T>(conf.absErrorBound, 32768)),
        HuffmanEncoder<int>(), Lossless_zstd());
    sz->decompress(conf, cmpDataPos, cmpSize, decData);
  }
}

int main(int argc, char **argv) {

  std::vector<size_t> dims({100, 200, 300});
  Config conf({dims[0], dims[1], dims[2]});
  // conf.cmprAlgo = ALGO_INTERP_LORENZO;
  conf.errorBoundMode =
      EB_ABS; // refer to def.hpp for all supported error bound mode
  conf.absErrorBound = 1E-3; // absolute error bound 1e-3

  std::vector<float> input_data(conf.num);
  std::vector<float> dec_data(conf.num);
  std::vector<size_t> stride({dims[1] * dims[2], dims[2], 1});

  for (size_t i = 0; i < dims[0]; ++i) {
    for (size_t j = 0; j < dims[1]; ++j) {
      for (size_t k = 0; k < dims[2]; ++k) {
        double x = static_cast<double>(i) - static_cast<double>(dims[0]) / 2.0;
        double y = static_cast<double>(j) - static_cast<double>(dims[1]) / 2.0;
        double z = static_cast<double>(k) - static_cast<double>(dims[2]) / 2.0;
        input_data[i * stride[0] + j * stride[1] + k] = static_cast<float>(
            .0001 * y * sin(y) + .0005 * cos(pow(x, 2) + x) + z);
      }
    }
  }
  std::vector<float> input_data_copy(input_data);
  auto dec_data_p = dec_data.data();
  std::vector<char> cmpData(conf.num * 2);
  size_t cmpSize = cmpData.size();

  cmpSize = SZ3_customized_compress<float, 3>(conf, input_data.data(),
                                              cmpData.data(), cmpSize);
  SZ3_customized_decompress<float, 3>(conf, cmpData.data(), cmpSize,
                                      dec_data_p);

  std::cout << "Compression Ratio: " << conf.num * sizeof(float) * 1.0 / cmpSize
            << std::endl;
  verify(input_data_copy.data(), dec_data.data(), conf.num);
  return 0;
}
