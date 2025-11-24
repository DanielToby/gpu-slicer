#include <catch2/catch_test_macros.hpp>
#include "catch2/generators/catch_generators.hpp"

#include "spatial_index/bvh.hpp"

using namespace slicer;

struct BinarySplitPointTestCase {
    std::size_t numItems;
    std::size_t minItemsPerSide;
    std::vector<std::size_t> expectedSplitPoints;
};

TEST_CASE("BVH: getAllBinarySplitPoints") {
    auto testCase = GENERATE(
        BinarySplitPointTestCase{0, 0, {}},
        BinarySplitPointTestCase{0, 1, {}},
        BinarySplitPointTestCase{1, 1, {}},
        BinarySplitPointTestCase{1, 0, {0}},

        BinarySplitPointTestCase{2, 1, {1}},
        BinarySplitPointTestCase{2, 0, {0, 1}},
        BinarySplitPointTestCase{2, 2, {}},

        BinarySplitPointTestCase{3, 0, {0, 1, 2}},
        BinarySplitPointTestCase{3, 1, {1, 2}},
        BinarySplitPointTestCase{3, 2, {}},

        BinarySplitPointTestCase{4, 0, {0, 1, 2, 3}},
        BinarySplitPointTestCase{4, 1, {1, 2, 3}},
        BinarySplitPointTestCase{4, 2, {2}},
        BinarySplitPointTestCase{4, 3, {}}
    );

    INFO(testCase.numItems);
    INFO(testCase.minItemsPerSide);

    auto result = detail::getBalancedBinarySplitPoints(testCase.numItems, testCase.minItemsPerSide);
    CHECK(result == testCase.expectedSplitPoints);
}