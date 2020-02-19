#ifndef ROOTNODE_H_
#define ROOTNODE_H_

#include "InnerNode.h"
#include "LeafNode.h"
#include <queue>

using namespace std;

class RootNode : public InnerNode {
public:
    RootNode(std::unique_ptr<Node> leftChildVal, std::unique_ptr<Node> rightChildVal) : InnerNode(
            std::move(leftChildVal),
            std::move(rightChildVal)) {};

    ~RootNode() override = default;

    vector<pair<string, word_type>> getTreeAsBitAddressFormat();

    void addWordsToMap(vector<pair<string, word_type>> &map, const string &startingAddress,
                       const bool allowAddressExtension) const override {};
};

#endif /* ROOTNODE_H_ */
