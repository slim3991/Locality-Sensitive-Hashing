#pragma once

#include <cmath>
#include <csignal>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

class LSH {
  using HashResult = std::vector<double>;
  double W;
  size_t n_hash;
  size_t data_size;

  struct Projections {
    std::vector<double> a;
    double b;
  };
  std::vector<Projections> projections;

public:
  LSH(double W, size_t n_hash, size_t dim)
      : W{W}, n_hash{n_hash}, data_size(dim) {

    std::uniform_real_distribution<double> dist_u(0, W);
    std::normal_distribution<double> dist_n(0, 1);

   projections.resize(n_hash);
    for (auto &proj : projections) {
      std::vector<double> a(data_size);
      for (double &element : a)
        element = dist_n(random_gen());
      proj = {a, dist_u(random_gen())};
    }
  };

  HashResult compute_hash(const std::vector<double> &data) {
    HashResult results(n_hash);
    for (size_t i{0}; i < n_hash; i++) {
      double dot = std::inner_product(projections[i].a.begin(),
                                      projections[i].a.end(), data.begin(), 0);
      results[i] = std::floor((dot + projections[i].b) / W);
    };
    return results;
  };

  double compare(const HashResult &res1, const HashResult &res2) {
    if (res1.size() != res2.size()) {
      throw std::invalid_argument("Size missmatch");
    }

    int matches{0};
    for (size_t i{0}; i < n_hash; i++) {
      if (res1[i] == res2[i])
        matches++;
    }
    return static_cast<double>(matches) / n_hash;
  }

private:
  std::mt19937 &random_gen() {
    static std::random_device rd;
    static std::mt19937 gen{rd()};
    return gen;
  }
};
