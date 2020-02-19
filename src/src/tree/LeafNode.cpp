#include "LeafNode.h"

LeafNode::LeafNode(const word_type w) {
    this->word = w;
}

vector<word_type> LeafNode::getWordMembers() {
    vector<word_type> valueToReturn;
    valueToReturn.push_back(this->word);
    return valueToReturn;
}

void LeafNode::addWordsToMap(vector<pair<string, word_type>> &map, const string& startingAddress,
                             const bool allowAddressExtension) const {
    map.push_back({startingAddress, this->word});
}
