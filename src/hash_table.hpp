#pragma once
#include "lsh.hpp"
#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace find_better_name {

class VectorDB {

  // TODO: This could be any iterable with length "data_length"
  // containing anything that could be cast to a double.
  using dbData = std::vector<double>;
  using id_t = int;

  size_t n_buckets;   // number of bucket the LHS makes
  size_t data_length; // lenght of the vectors
  size_t W;
  LSH hasher;

  std::unordered_map<HashResult, std::vector<id_t>, MyHash> table;

public:
  VectorDB(size_t n_buckets, size_t data_length, double W)
      : n_buckets{n_buckets}, data_length{data_length},
        hasher(W, n_buckets, data_length) {}

  void add(const dbData &data, id_t ID) {
    HashResult res = hasher.compute_hash(data);
    table[res].push_back(ID);
  }

  std::vector<id_t> querry(const dbData &data) const {
    HashResult hash_res = hasher.compute_hash(data);
    try {
      return table.at(hash_res);

    } catch (std::out_of_range) {
      return {};
    };
  };

private:
  static struct MyHash {
    std::size_t operator()(const HashResult &s) const noexcept {
      std::hash<int> hasher;
      int hash;
      size_t n = s.size();
      for (const auto &i : s) {
        hash += hasher(i) / n;
      }
      return hash;
    };
  };
};
} // namespace find_better_name
