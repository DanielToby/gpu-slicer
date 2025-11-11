#include <catch2/catch_test_macros.hpp>

#include <ranges>

#include "bvh.hpp"

TEST_CASE("Views") {
    auto vec = std::vector{1, 2, 3, 4, 5};
    auto evens_view = vec | std::views::filter([](auto x) { return x % 2 == 0; });
    auto evens = std::vector(evens_view.begin(), evens_view.end());

    CHECK(evens == std::vector{2, 4});
}