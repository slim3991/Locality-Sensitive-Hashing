#include <catch2/catch_test_macros.hpp>
#include "lsh.hpp"

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
