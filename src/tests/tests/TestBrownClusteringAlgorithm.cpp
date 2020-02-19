#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "Utils.h"
#include "BrownClusteringAlgorithm/BrownClusteringAlgorithm.h"
#include <gmock/gmock.h>

TEST(BrownClusteringAlgorithmTest, testSwapForVector) {
    vector_word_type actual {1, 2, 3, 4, 5, 6, 7};
    const vector_word_type expected {1, 4, 3, 4, 5, 6, 7};
    const auto sizeToConsider = 4;
    BrownClusteringAlgorithm::swapForVector(actual, 1, sizeToConsider);
    EXPECT_EQ(expected, actual);
}

TEST(BrownClusteringAlgorithmTest, testSwapForMatrix) {
    matrix_occurrences actual {{1, 2, 3, 4, 5}, {6, 7, 8, 9, 10}, {11, 12, 13, 14, 15}, {16, 17, 18, 19, 20}, {21, 22, 23, 24, 25}};
    const matrix_occurrences expected {{1, 4, 3, 4, 5}, {16, 19, 18, 19, 20}, {11, 14, 13, 14, 15}, {16, 19, 18, 19, 20}, {21, 22, 23, 24, 25}};
    const auto position = 1;
    const auto sizeToConsider = 4;
    BrownClusteringAlgorithm::swapForMatrix(actual, position, sizeToConsider);
    EXPECT_EQ(expected, actual);
}

TEST(BrownClusteringAlgorithmTest, testComputeSk) {
    const matrix_double q = {
            {1, 2, 3, 4, 5},
            {6, 7, 8, 9, 10},
            {11, 12, 13, 14, 15},
            {16, 17 ,18, 19, 20},
            {21, 22, 23, 24, 25}
    };
    auto actualSk = vector<double>(5, 0);
    const vector<double> expectedSk = {69, 93, 117, 141, 165};
    BrownClusteringAlgorithm::computeSk(actualSk, q, 5);
    EXPECT_THAT(actualSk, ::testing::ContainerEq(expectedSk));
}