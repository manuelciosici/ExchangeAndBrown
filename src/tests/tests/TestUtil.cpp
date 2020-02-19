#include <gtest/gtest.h>
#include "Utils.h"

TEST(UtilsTest, testlogzero) {
    EXPECT_EQ(Utils::logNoInf(0), 0);
}

TEST(UtilsTest, testlogtwo) {
    EXPECT_EQ(Utils::logNoInf(2), 1);
}

TEST(UtilsTest, testlog1024) {
    EXPECT_EQ(Utils::logNoInf(1024), 10);
}

TEST(UtilsTest, testlogInf) {
    EXPECT_EQ(Utils::logNoInf(-1), 0);
}

TEST(UtilsTest, testMIzerojoint) {
    EXPECT_EQ(Utils::computeMI(0, 1, 1), 0);
}

TEST(UtilsTest, testMIzeroLR) {
    EXPECT_EQ(Utils::computeMI(1, 0, 1), 0);
    EXPECT_EQ(Utils::computeMI(1, 1, 0), 0);
}

TEST(UtilsTest, testMINormalValue) {
    EXPECT_EQ(Utils::computeMI(16, 2, 2), 32);
}