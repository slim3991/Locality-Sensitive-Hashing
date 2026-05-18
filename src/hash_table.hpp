#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <random>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

// TODO:: Fix the types, make it so that any iterable with a type that can be
// cast to double is accepted
// TODO:: Fix all the names

namespace LSHIndex {

using HashResult = std::vector<int>;

class LSH {
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

    std::uniform_real_distribution<> dist_u(0, W);
    std::normal_distribution<double> dist_n(0, 1);

    projections.resize(n_hash);
    for (auto &proj : projections) {
      std::vector<double> a(data_size);
      for (double &element : a)
        element = dist_n(random_gen());
      proj = {a, dist_u(random_gen())};
    }
  };

  HashResult compute_hash(const std::vector<double> &data) const {
    if (data.size() != data_size) {
      throw std::invalid_argument("Size missmatch");
    }

    HashResult results(n_hash);
    for (size_t i{0}; i < n_hash; i++) {
      double dot = std::inner_product(
          projections[i].a.begin(), projections[i].a.end(), data.begin(), 0.0);
      results[i] = std::floor((dot + projections[i].b) / W);
    };
    return results;
  };

  bool match(const HashResult &res1, const HashResult &res2) const {
    if (res1.size() != res2.size()) {
      throw std::invalid_argument("Size missmatch");
    }
    int matches{0};
    for (size_t i{0}; i < n_hash; i++) {
      if (res1[i] == res2[i])
        matches++;
    }
    return matches == n_hash;
  }

  double compare(const HashResult &res1, const HashResult &res2) const {
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
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen{rd()};
    return gen;
  }
};
class VectorHashIndex {

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

  using dbData = std::vector<double>;
  using id_t = int;
  using vectorHashTable =
      std::unordered_map<HashResult, std::vector<id_t>, MyHash>;

  struct HashTable {
    LSH hasher;
    vectorHashTable table;
    std::unordered_map<id_t, HashResult> hr_to_id;
  };

  size_t n_buckets;   // number of bucket the LHS makes
  size_t data_length; // length of the vectors
  double W;           // Bucket width
  std::vector<HashTable> hash_tables;

public:
  VectorHashIndex(size_t n_buckets, size_t data_length, double W,
                  size_t n_tables)
      : n_buckets{n_buckets}, data_length{data_length} {

    hash_tables.reserve(n_tables);
    for (size_t i{0}; i < n_tables; i++) {
      hash_tables.push_back({LSH{W, n_buckets, data_length}, vectorHashTable{},
                             std::unordered_map<id_t, HashResult>{}});
    }
  }

  void add(const dbData &data, id_t ID) {
    for (HashTable &table : hash_tables) {
      HashResult res = table.hasher.compute_hash(data);
      table.table[res].push_back(ID);
      table.hr_to_id[ID] = res;
    };
  }

  void remove(id_t ID) {
    for (HashTable &table : hash_tables) {
      auto it = table.hr_to_id.find(ID);
      if (it != table.hr_to_id.end()) {
        HashResult hash = it->second;
        auto &bucket = table.table[hash];
        bucket.erase(std::remove(bucket.begin(), bucket.end(), ID),
                     bucket.end());
        ;
        table.hr_to_id.erase(ID);
        if (bucket.empty())
          table.table.erase(hash);
      }
    };
  }

  std::vector<id_t> query(const dbData &data) const {
    std::vector<id_t> matches;
    for (const HashTable &table : hash_tables) {
      HashResult res = table.hasher.compute_hash(data);
      auto it = table.table.find(res);
      if (it != table.table.end()) {
        matches.insert(matches.end(), it->second.begin(), it->second.end());
      };
    }
    // deduplicate
    std::sort(matches.begin(), matches.end());
    auto last = std::unique(matches.begin(), matches.end());
    matches.erase(last, matches.end());
    return matches;
  };
};

} // namespace LSHIndex
