#ifndef SZ3_TRUNCATION_DECOMPOSITION
#define SZ3_TRUNCATION_DECOMPOSITION

/**
 */

#include "SZ3/utils/Config.hpp"

namespace SZ3 {

template <class T, uint N>
class TruncationDecomposition : public concepts::DecompositionInterface<T, int, N> {
public:
  TruncationDecomposition(const Config &conf) : num(conf.num) {}

  std::vector<int> compress(const Config &conf, T *data) override {
    std::vector<int> output(num);
    for (size_t i = 0; i < num; i++) {
      output[i] = data[i];
    }
    return output;
  }

  T *decompress(const Config &conf, std::vector<int> &quant_inds,
                T *dec_data) override {
    for (size_t i = 0; i < num; i++) {
      dec_data[i] = quant_inds[i];
    }
    return dec_data;
  }

  void save(uchar *&c) override { write(num, c); }

  void load(const uchar *&c, size_t &remaining_length) override {
    read(num, c, remaining_length);
  }

  std::pair<int, int> get_out_range() override { return {0, 0}; }

private:
  size_t num;
};
} // namespace SZ3
#endif
