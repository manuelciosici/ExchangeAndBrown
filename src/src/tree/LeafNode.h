#ifndef LEAF_H_
#define LEAF_H_

#include "../Utils.h"
#include "Node.h"

class LeafNode : public Node {
private:
    word_type word;
public:
    LeafNode(word_type w);

    virtual ~LeafNode() = default;

    vector<word_type> getWordMembers() final;

    word_type getWord() const {
        return word;
    }

    void addWordsToMap(vector<pair<string, word_type>> &map, const string &startingAddress,
                       bool allowAddressExtension) const override;

    word_type getWordLabel() override { return this->getWord(); };

    void printMerges(ofstream &stream, const Corpus &corpus) override {};
};

#endif /* LEAF_H_ */
