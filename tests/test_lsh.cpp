#include "lsh.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("LSH functionality", "[lsh]") {
  // This setup runs for EVERY section below
  double W = 4.0;
  size_t hashes = 2;
  LSH lsh(W, hashes, 3);

  SECTION("Hash result size") {
    auto result = lsh.compute_hash({1.0, 2.0, 3.0});
    REQUIRE(result.size() == hashes);
  }

  SECTION("Comparing identical hashes") {
    auto res1 = lsh.compute_hash({1.0, 1.0, 1.0});
    auto res2 = res1;
    // We expect a perfect match (1.0 similarity)
    REQUIRE(lsh.compare(res1, res2) == 1.0);
  }
}

TEST_CASE("LSH detailed mechanics", "[lsh]") {
  double W = 4.0;
  size_t hashes = 10;
  size_t dim = 3;
  LSH lsh(W, hashes, dim);

  SECTION("Deterministic output for same input") {
    std::vector<double> data = {1.0, -1.0, 0.5};
    auto res1 = lsh.compute_hash(data);
    auto res2 = lsh.compute_hash(data);

    REQUIRE(res1 == res2);
  }

  SECTION("Dimension mismatch in compare throws exception") {
    std::vector<int> res1 = {1, 2};
    std::vector<int> res2 = {1, 2, 3};

    // This triggers the size mismatch check in compare()
    REQUIRE_THROWS_AS(lsh.compare(res1, res2), std::invalid_argument);
  }

  SECTION("Similarity score degrades with distance") {
    size_t many_hashes = 50;
    LSH lsh_precise(2.0, many_hashes, 2);

    std::vector<double> target = {0.0, 0.0};
    std::vector<double> very_close = {0.1, 0.1};
    std::vector<double> medium_far = {2.0, 2.0};
    std::vector<double> very_far = {100.0, 100.0};

    auto h_target = lsh_precise.compute_hash(target);
    auto h_close = lsh_precise.compute_hash(very_close);
    auto h_medium = lsh_precise.compute_hash(medium_far);
    auto h_far = lsh_precise.compute_hash(very_far);

    double score_close = lsh_precise.compare(h_target, h_close);
    double score_medium = lsh_precise.compare(h_target, h_medium);
    double score_far = lsh_precise.compare(h_target, h_far);

    // On average, similarity should decrease as distance increases
    CHECK(score_close >= score_medium);
    CHECK(score_medium >= score_far);
  }
  SECTION("Locality sensitivity property") {
    // High dimensional LSH for better statistical signal
    size_t many_hashes = 100;
    LSH lsh_stat(W, many_hashes, 2);

    std::vector<double> base = {0.0, 0.0};
    std::vector<double> near = {0.1, 0.1};
    std::vector<double> far = {100.0, 100.0};

    auto h_base = lsh_stat.compute_hash(base);
    auto h_near = lsh_stat.compute_hash(near);
    auto h_far = lsh_stat.compute_hash(far);

    double sim_near = lsh_stat.compare(h_base, h_near);
    double sim_far = lsh_stat.compare(h_base, h_far);

    // Close points should generally have higher similarity than far points
    CHECK(sim_near >= sim_far);
  }

  SECTION("Handling of zero vectors") {
    std::vector<double> zero_vec(dim, 0.0);
    auto res = lsh.compute_hash(zero_vec);

    REQUIRE(res.size() == hashes);
    // Result should be floor(b / W). Since 0 <= b < W, res should be 0.
    for (double val : res) {
      REQUIRE(val == 0.0);
    }
  }
}

TEST_CASE("LSH Parameter Edge Cases", "[lsh]") {
  SECTION("Small W results in higher sensitivity") {
    // With a very small W, even slight movements likely change the bucket
    LSH lsh_small_w(0.001, 10, 2);
    auto h1 = lsh_small_w.compute_hash({1.0, 1.0});
    auto h2 = lsh_small_w.compute_hash({1.001, 1.001});

    // It is highly probable that at least one hash differs
    CHECK(lsh_small_w.compare(h1, h2) < 1.0);
  }

  SECTION("Large W results in lower sensitivity") {
    // With a very large W, most points fall into the same bucket
    LSH lsh_large_w(10000.0, 10, 2);
    auto h1 = lsh_large_w.compute_hash({1.0, 1.0});
    auto h2 = lsh_large_w.compute_hash({2.0, 2.0});

    // It is highly probable that they share buckets
    CHECK(lsh_large_w.compare(h1, h2) == 1.0);
  }
}
