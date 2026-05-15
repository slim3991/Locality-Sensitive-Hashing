#pragma once
#include "lsh.hpp"
#include <cstddef>
#include <functional>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// TODO:: Add multiple tables
// TODO:: Fix the types, make it so that any iterable with a type that can be
// cast to double is accepted

namespace find_better_name {

class VectorDB {

  struct MyHash {
    std::size_t operator()(const HashResult &s) const noexcept {
      std::size_t seed = 0;
      for (int i : s) {
        // Gold ratio constant helps distribute bits
        seed ^= std::hash<int>{}(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

  // TODO: bdData could be any iterable with length "data_length"
  // containing anything that could be cast to a double.
  using dbData = std::vector<double>;
  using id_t = int;

  size_t n_buckets;   // number of bucket the LHS makes
  size_t data_length; // length of the vectors
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

  std::vector<id_t> query(const dbData &data) const {
    HashResult hash_res = hasher.compute_hash(data);
    auto it = table.find(hash_res);
    if (it != table.end())
      return it->second;
    else
      return {};
  };
};
} // namespace find_better_name
