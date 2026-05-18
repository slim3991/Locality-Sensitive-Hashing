#include "hash_table.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace LSHIndex;

TEST_CASE("LSH core functionality", "[lsh]") {
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
        REQUIRE(lsh.compare(res1, res2) == 1.0);
    }

    SECTION("Deterministic output for same input") {
        std::vector<double> data = {1.0, -1.0, 0.5};
        auto res1 = lsh.compute_hash(data);
        auto res2 = lsh.compute_hash(data);
        REQUIRE(res1 == res2);
    }

    SECTION("Dimension mismatch throws exception") {
        REQUIRE_THROWS_AS(lsh.compute_hash({1.0}), std::invalid_argument);
    }
}

TEST_CASE("VectorHashIndex operations", "[index]") {
    size_t n_buckets = 5;
    size_t dim = 3;
    double W = 4.0;
    size_t n_tables = 3;

    VectorHashIndex index(n_buckets, dim, W, n_tables);

    SECTION("Add and Query") {
        std::vector<double> vec1 = {1.0, 1.0, 1.0};
        index.add(vec1, 100);

        auto results = index.query(vec1);
        
        // The point should be found in its own bucket
        REQUIRE(results.size() == 1);
        CHECK(results[0] == 100);
    }

    SECTION("Remove operation") {
        std::vector<double> vec1 = {1.0, 1.0, 1.0};
        index.add(vec1, 100);
        index.remove(100);

        auto results = index.query(vec1);
        CHECK(results.empty());
    }

    SECTION("Multi-table collisions increase recall") {
        // Use very few buckets to force collisions
        VectorHashIndex small_index(1, dim, 100.0, 2);
        
        std::vector<double> p1 = {1.0, 1.0, 1.0};
        std::vector<double> p2 = {1.1, 1.1, 1.1};
        
        small_index.add(p1, 1);
        small_index.add(p2, 2);

        auto results = small_index.query(p1);
        
        // Both should be in the same bucket due to large W and low hash count
        CHECK(std::find(results.begin(), results.end(), 1) != results.end());
        CHECK(std::find(results.begin(), results.end(), 2) != results.end());
    }

    SECTION("Deduplication in query") {
        std::vector<double> vec = {1.0, 1.0, 1.0};
        index.add(vec, 500);

        auto results = index.query(vec);
        
        // Even if found in multiple tables, ID 500 should appear only once
        size_t count = std::count(results.begin(), results.end(), 500);
        CHECK(count == 1);
    }
}

TEST_CASE("LSH Statistical Properties", "[lsh][statistical]") {
    SECTION("Similarity score degrades with distance") {
        size_t many_hashes = 50;
        LSH lsh_stat(2.0, many_hashes, 2);

        std::vector<double> target = {0.0, 0.0};
        std::vector<double> near = {0.1, 0.1};
        std::vector<double> far = {100.0, 100.0};

        auto h_target = lsh_stat.compute_hash(target);
        auto h_near = lsh_stat.compute_hash(near);
        auto h_far = lsh_stat.compute_hash(far);

        double sim_near = lsh_stat.compare(h_target, h_near);
        double sim_far = lsh_stat.compare(h_target, h_far);

        CHECK(sim_near >= sim_far);
    }

    SECTION("Handling of zero vectors") {
        LSH lsh(4.0, 5, 3);
        std::vector<double> zero_vec(3, 0.0);
        auto res = lsh.compute_hash(zero_vec);

        // dot product is 0, so result is floor(b / W).
        // Since 0 <= b < W, floor(b/W) is always 0.
        for (int val : res) {
            REQUIRE(val == 0);
        }
    }
}
