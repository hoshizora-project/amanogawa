#ifndef AMANOGAWA_MEASURE_H
#define AMANOGAWA_MEASURE_H

#include <mecab.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace to_graph {
template <class _data_t, class _sim_t> struct Measure {
  using data_t = _data_t;
  using sim_t = _sim_t;
  virtual sim_t operator()(const data_t &l, const data_t &r) const = 0;
};

struct CosineMeasure : Measure<std::vector<double>, double> {
  explicit CosineMeasure() {}

  // FIXME: Vectorize
  inline sim_t operator()(const std::vector<double> &l,
                          const std::vector<double> &r) const override {
    auto ll = 0.;
    for (const auto &el : l) {
      ll += el * el;
    }
    auto rr = 0.;
    for (const auto &el : r) {
      rr += el * el;
    }
    auto lr = 0.;
    for (size_t i = 0, end = l.size(); i < end; ++i) {
      lr += l[i] * r[i];
    }

    return lr / (std::pow(ll, 0.5) * std::pow(rr, 0.5));
  }
};

struct Doc2VecMeasure : Measure<std::string, double> {
  using sim_t = double;
  const std::string model;

  explicit Doc2VecMeasure(const std::string model) : model(std::move(model)) {}

  inline sim_t operator()(const std::string &l,
                          const std::string &r) const override {
    return 0; // not impl
  }
};

struct BoWMeasure : Measure<std::unordered_map<std::string, int>, double> {
  using sim_t = double;
  const double p;

  explicit BoWMeasure(const double &p) : p(p) {}

  // FIXME
  inline sim_t operator()(const data_t &l, const data_t &r) const override {
    data_t bag(l);
    bag.insert(r.begin(), r.end());

    double diff = 0;
    for (const auto &el : bag) {
      const auto word = el.first;
      const auto li = l.find(word);
      const auto ri = r.find(word);
      const auto lv = li != l.end() ? li->second : 0;
      const auto rv = ri != r.end() ? ri->second : 0;
      diff += std::pow(std::abs(lv - rv), p);
    }
    return std::pow(diff, 1 / p);
  }

  static std::unique_ptr<MeCab::Tagger> tagger;

  static inline std::vector<std::string> wakati(const std::string &sentence) {
    const auto result = tagger->parse(sentence.c_str());
    return split(result, ' ');
  }
};
auto BoWMeasure::tagger =
    std::unique_ptr<MeCab::Tagger>(MeCab::createTagger("-Owakati"));
} // namespace to_graph
} // namespace flow
} // namespace plugin
} // namespace amanogawa
#endif // AMANOGAWA_MEASURE_H
