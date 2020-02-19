#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <models/WordMappings.h>

TEST(WordMappingsTest, testmapping) {
    WordMappings mapping;
    vector<string> expectedMappings = {"a", "b", "c", "a", "b"};
    vector<word_type> expectedIDs = {0, 1, 2, 0, 1};
    vector<word_type> actualIDs;
    for (const string& item : expectedMappings) {
        actualIDs.push_back(mapping.getID(item));
    }
    EXPECT_THAT(actualIDs, ::testing::ContainerEq(expectedIDs));
    vector<word_type> actualIDs2;
    for (const string& item : expectedMappings) {
        actualIDs2.push_back(mapping.getIDWithoutAllocation(item));
    }
    EXPECT_THAT(actualIDs2, ::testing::ContainerEq(expectedIDs));
}