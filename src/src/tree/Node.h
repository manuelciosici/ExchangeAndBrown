#ifndef NODE_H_
#define NODE_H_

#include "../Utils.h"
#include "../models/Corpus.h"

class Node {
protected:
    static const string LEFT_BIT;
    static const string RIGHT_BIT;
    bool isFinalCluster = false;
public:
    Node() { };

    virtual ~Node() { };

    virtual vector<word_type> getWordMembers() { return {}; };

    /**
     * Returns the word label of this node. Always the leftmost child leaf.
     * @return
     */
    virtual word_type getWordLabel() { return {};};

    virtual void printMerges(ofstream &stream, const Corpus &corpus) {};

    void setIsFinalCluster(bool value) {
        this->isFinalCluster = value;
    }

    auto getIsFinalCluster() const {
        return this->isFinalCluster;
    }

    /**
     * Should only be implemented by InnerNode and LeafNode. Recursively adds all children nodes to the given map.
     * @param map
     * @param startingAddress
     * @param allowAddressExtension
     */
    virtual void addWordsToMap(vector<pair<string, word_type>> &map, const string& startingAddress,
                               const bool allowAddressExtension) const { };
};

#endif /* NODE_H_ */
