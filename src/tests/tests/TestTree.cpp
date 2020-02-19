#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tree/LeafNode.h>
#include <tree/InnerNode.h>
#include <tree/RootNode.h>

TEST(TreeTest, testGetTreeAsBitAddressFormat) {
    unique_ptr<Node> node0 = std::make_unique<LeafNode>(0);
    node0->setIsFinalCluster(true);
    unique_ptr<Node> node1 = std::make_unique<LeafNode>(1);
    node1->setIsFinalCluster(true);
    unique_ptr<Node> node2 = std::make_unique<LeafNode>(2);
    unique_ptr<Node> node3 = std::make_unique<LeafNode>(3);
    unique_ptr<Node> node4 = std::make_unique<LeafNode>(4);
    unique_ptr<Node> node5 = std::make_unique<LeafNode>(5);

    unique_ptr<Node> innerNode0 = std::make_unique<InnerNode>(std::move(node3), std::move(node4));
    unique_ptr<Node> innerNode1 = std::make_unique<InnerNode>(std::move(node5), std::move(innerNode0));
    innerNode1->setIsFinalCluster(true);
    unique_ptr<Node> innerNode2 = std::make_unique<InnerNode>(std::move(node2), std::move(innerNode1));
    unique_ptr<Node> innerNode3 = std::make_unique<InnerNode>(std::move(node1), std::move(innerNode2));

    unique_ptr<RootNode> rootNode = make_unique<RootNode>(std::move(node0), std::move(innerNode3));

    const vector<pair<string, word_type>> expectedValue = {
            {"0", 0},
            {"10", 1},
            {"110", 2},
            {"111", 5},
            {"111", 3},
            {"111", 4},
    };
    const vector<pair<string, word_type>> &actualValues = rootNode->getTreeAsBitAddressFormat();
    EXPECT_THAT(actualValues, ::testing::ContainerEq(expectedValue));
}