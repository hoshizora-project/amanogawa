#ifndef AMANOGAWA_NNDESCENT_H
#define AMANOGAWA_NNDESCENT_H

#include "amanogawa/include/api.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <thread>
#include <unordered_set>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

// This code is based on https://github.com/TatsuyaShirakawa/nndescent
namespace amanogawa {
namespace plugin {
namespace flow {
namespace to_graph {

template <class data_t, class measure_t> struct NNDescent {

public:
  using sim_t = typename measure_t::sim_t;

  struct node_t {
    explicit node_t(const data_t &data) : data(data) {}
    node_t() : data() {}
    data_t data;
  };

  struct ngh_t {
    ngh_t() : node_id(), sim(std::numeric_limits<sim_t>::min()) {}
    ngh_t(const std::size_t node_id, const sim_t sim)
        : node_id(node_id), sim(sim) {}
    std::size_t node_id;
    sim_t sim;
    bool operator>(const ngh_t &ngh) const { return sim > ngh.sim; }
  };

  using nghs_t = std::vector<ngh_t>;

  // FIXME: Slow
  const std::function<bool(ngh_t, ngh_t)> comp_dist =
      [](const auto &l, const auto &r) { return l.sim > r.sim; };

  using rand_t = std::mt19937;

public:
  explicit NNDescent(const uint32_t K, const measure_t &measure,
                     const double rho = 0.5, const double perturb_rate = 0,
                     const uint32_t num_random_join = 10)
      : K(K), num_random_join(num_random_join), rho(rho),
        perturb_rate(perturb_rate), nodes(), nghs(), rand(std::mt19937(0)),
        measure(measure), checked() {}

public:
  void init_graph(const std::vector<data_t> &datas) {
    if (datas.empty()) {
      return;
    }

    // clear
    clear();

    // set datas
    for (const auto &data : datas) {
      nodes.emplace_back(node_t(data));
    }

    // construct random neighborhoods
    init_random_nghs();
  }

  std::size_t update() {
    if (nodes.size() < 2 || K == 0) {
      return 0;
    } // cannot create nbd

    const std::size_t N = this->nodes.size();

    std::vector<nghs_t> old_nbds(N), new_nbds(N), old_rnbds(N), new_rnbds(N);

    // Process 1
    // set old_nbds / new_nbds
    for (std::size_t i = 0; i < N; ++i) { // ToDo: parallel for i
      this->prep_ngh(i, new_nbds[i], old_nbds[i]);
    }

    // Process 2
    // set old_rnbds / new_rnbds
    for (std::size_t i = 0; i < N; ++i) {
      const auto &new_nbd(new_nbds[i]), &old_nbd(old_nbds[i]);
      for (const ngh_t &el : old_nbd) {
        assert(el.node_id != i);
        old_rnbds[el.node_id].emplace_back(ngh_t(i, el.sim));
      }
      for (const ngh_t &el : new_nbd) {
        assert(el.node_id != i);
        new_rnbds[el.node_id].emplace_back(ngh_t(i, el.sim));
      }
    }

    // Process 3
    // local join
    std::size_t num_updates = 0;
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (std::size_t i = 0; i < N; ++i) { // ToDo: parallel for i
      num_updates +=
          local_join(i, new_nbds[i], old_nbds[i], new_rnbds[i], old_rnbds[i]);
    }

    return num_updates;
  }

  void exec(const std::vector<data_t> &datas,
            const std::size_t &max_epoch = 100, const double &delta = 0.001) {
    logger->info("init graph");
    init_graph(datas);
    logger->info("inited graph");

    for (std::size_t epoch = 0; epoch < max_epoch; ++epoch) {
      logger->info("update [{}/{}] ...", epoch + 1, max_epoch);
      const auto update_count = update();
      const auto KN = this->rho * this->K * this->nodes.size();
      logger->info(" {}/{}", update_count, KN);
      if (update_count <= delta * KN) {
        logger->info("converged");
        break;
      }
    }
  }

private:
  logger_t logger = get_logger("nndescent");

  void clear() {
    nodes.clear();
    nghs.clear();
    checked.clear();
  }

  void init_random_nghs() {
    const auto N = nodes.size();

    //      const uint32_t K2 = (K <= N-1)? K : static_cast<uint32_t>(N-1);
    uint32_t K2 = K;
    if (K2 > N - 1) {
      K2 = N - 1;
    }
    assert(K2 <= K && K2 <= N - 1);
    nghs.clear();
    nghs.resize(N);
    checked.clear();
    checked.resize(N);

    if (N < 2 || K == 0) {
      return;
    } // cannot create nbd

    for (std::size_t i = 0; i < N; ++i) { // ToDo: parallel for i

      std::unordered_set<std::size_t> chosen;

      // assumed K << N
      for (std::size_t j = 0; j < K2; ++j) {
        std::size_t n = rand() % (N - 1);
        if (n >= i) {
          ++n;
        }
        assert(i != n);
        chosen.insert(n);
      }
      assert(chosen.size() <= K2);
      // set neighborhood
      const auto &node(this->nodes[i]);
      auto &nbd(this->nghs[i]);
      nbd.resize(chosen.size());
      std::size_t count = 0;
      for (auto j : chosen) {
        assert(i != j);
        nbd[count++] = ngh_t(j, this->measure(node.data, this->nodes[j].data));
      }
      std::sort(nbd.begin(), nbd.end(), comp_dist);

      checked[i].resize(nbd.size(), false);

      assert(nbd.size() > 0); // because K > 0 and N > 1
      assert(nbd.size() <= this->K);
    }
  }

  typedef enum { NOT_INSERTED, INSERTED } join_result_type;

  std::size_t compute_ub(const nghs_t &ngh, const std::size_t joiner,
                         const sim_t s, const std::size_t K2) const {
    assert(K2 > 0);
    if (ngh.back().sim >= s) {
      return K2;
    } else {
      return std::upper_bound(ngh.begin(), ngh.end(), ngh_t(joiner, s),
                              comp_dist) -
             ngh.begin();
    }
  }

  join_result_type join(const std::size_t base, const std::size_t joiner) {
    assert(base != joiner);
    assert(nodes.size() > 1 && this->K > 0);
    const auto &base_node(this->nodes[base]), &joiner_node(this->nodes[joiner]);
    auto &nbd(this->nghs[base]);
    auto &chkd(this->checked[base]);
    assert(nbd.size() == chkd.size());
    const auto s = measure(base_node.data, joiner_node.data);
    const auto K2 = nghs[base].size();

    if (s < nbd.back().sim && K2 == this->K) {
      return NOT_INSERTED;
    }

    const ngh_t joiner_elem(joiner, s);
    assert(K2 > 0);
    assert(K2 <= K && K2 <= nodes.size() - 1);

    // find the position i such that nbd[i] is where joiner will be inserted

    // search ub
    const std::size_t ub =
        std::upper_bound(nbd.begin(), nbd.end(), joiner_elem, comp_dist) -
        nbd.begin();

    // to prevent perturbation of nbd, the probability of replacement
    // with most dissimilar element in nbd shoule be suppressed
    //      if(ub == K2 && (rand() % B) >= prB){
    //      const auto B = 1000000; // Big Integer (ad-hoc)
    const auto SHIFT = 20;
    const auto B = 1 << SHIFT; // Big Integer (ad-hoc)
    const auto prB = perturb_rate * B;
    if (ub == K2 && nbd.back().sim == s &&
        (rand() & (B - 1)) >= prB) { // ub == K2 && rand() <= perturb_rate
      return NOT_INSERTED;
    }

    // search lb
    const std::size_t lb = std::lower_bound(nbd.begin(), nbd.begin() + ub,
                                            joiner_elem, comp_dist) -
                           nbd.begin();

    if (K2 > 0 && nbd[lb].sim == s) {
      for (std::size_t i = lb; i < ub; ++i) {
        if (nbd[i].node_id == joiner) {
          return NOT_INSERTED;
        } // joiner still in nbd
      }
    }

    assert(lb <= ub);
    auto pos = (lb < ub) ? lb + rand() % (ub - lb) : lb;

    // insert
    if (K2 < K) {
      nbd.insert(nbd.begin() + pos, joiner_elem);
      chkd.insert(chkd.begin() + pos, false);
    } else {
      assert(K2 == K);
      ngh_t cur_elem(joiner_elem);
      bool cur_checked = false;
      for (std::size_t i = pos; i < K2; ++i) {
        std::swap(cur_elem, nbd[i]);
        std::swap(cur_checked, chkd[i]);
      }
    }

    return INSERTED;
  }

  void prep_ngh(const std::size_t i, nghs_t &new_nbd, nghs_t &old_nbd) {
    const std::size_t N = nodes.size();
    const nghs_t &nbd(this->nghs[i]);
    const std::size_t K2 = nbd.size();
    const std::size_t rhoK =
        std::min(static_cast<std::size_t>(std::ceil(rho * K)), N - 1);

    std::vector<std::size_t> sampled;
    for (std::size_t j = 0, n = 0; j < K2; ++j) {
      assert(nbd[j].node_id != i);
      if (checked[i][j]) {
        old_nbd.push_back(nbd[j]);
      } else {
        // choose rhoK unchecked element with reservoir sampling
        if (n < rhoK) {
          sampled.push_back(j);
        } else {
          std::size_t m = rand() % (n + 1);
          if (m < rhoK) {
            sampled[m] = j;
          }
        }
        ++n;
      }
    }

    for (const std::size_t j : sampled) {
      assert(i != nbd[j].node_id);
      checked[i][j] = true;
      new_nbd.push_back(nbd[j]);
    }
  }

  std::size_t local_join(const std::size_t i, nghs_t &new_nbd, nghs_t &old_nbd,
                         const nghs_t &new_rnbd, const nghs_t &old_rnbd) {

    std::size_t update_count = 0;
    const std::size_t N = nodes.size();
    const std::size_t rhoK =
        std::min(static_cast<std::size_t>(std::floor(rho * K)), N - 1);

    // old_nbd = old_nbd \cup sample(old_rnbd, rhoK)
    for (const auto &elem : old_rnbd) {
      if (rand() % old_rnbd.size() < rhoK) {
        old_nbd.emplace_back(elem);
      }
    }

    // new_nbd = new_nbd \cup sample(new_rnbd, rhoK)
    for (const auto &elem : new_rnbd) {
      if (rand() % new_rnbd.size() < rhoK) {
        new_nbd.emplace_back(elem);
      }
    }

    // join(new_nbd, this)
    for (const auto &elem : new_nbd) {
      assert(elem.node_id != i);
      update_count += join(elem.node_id, i);
    }

    // join(new_nbd, new_ndb)
    for (std::size_t j1 = 0, M = new_nbd.size(); j1 < M; ++j1) {
      const auto &elem1(new_nbd[j1]);
      for (std::size_t j2 = j1 + 1; j2 < M; ++j2) {
        const auto &elem2(new_nbd[j2]);
        if (elem1.node_id == elem2.node_id) {
          continue;
        }
        update_count += join(elem1.node_id, elem2.node_id);
        update_count += join(elem2.node_id, elem1.node_id);
      }
    }

    // join(new_nbd, old_ndb)
    for (const auto &elem1 : new_nbd) {
      for (const auto &elem2 : old_nbd) {
        if (elem1.node_id == elem2.node_id) {
          continue;
        }
        update_count += join(elem1.node_id, elem2.node_id);
        update_count += join(elem2.node_id, elem1.node_id);
      }
    }

    // random_join
    for (std::size_t j = 0; j < num_random_join; ++j) {
      std::size_t node_id = rand() % (nodes.size() - 1);
      if (node_id >= i) {
        ++node_id;
      }
      this->join(i, node_id);
    }

    return update_count;
  }

public:
  uint32_t K;
  uint32_t num_random_join; // random join size
  double rho;               // sample rate
  double perturb_rate;
  std::vector<node_t> nodes;
  std::vector<nghs_t> nghs;
  rand_t rand;
  measure_t measure;

  ~NNDescent() { drop_logger("nndescent"); }

private:
  std::vector<std::vector<bool>> checked;
};
} // namespace to_graph
} // namespace flow
} // namespace plugin
} // namespace amanogawa
#endif // AMANOGAWA_NNDESCENT_H
