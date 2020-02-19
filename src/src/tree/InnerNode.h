#ifndef INNERNODE_H_
#define INNERNODE_H_

#include "Node.h"
#include "../Utils.h"
#include <string>
#include <memory>
#include <iomanip>

class InnerNode : public Node {
protected:
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> rightChild;
    double amiLoss;
    double amiAfterLoss;
    word_type mergeID;
public:
    /**
     * Constructor. Takes ownership of the left and right child pointers.
     * @param leftChildVal left child
     * @param rightChildVal right child
     */
    InnerNode(std::unique_ptr<Node> leftChildVal, std::unique_ptr<Node> rightChildVal);

    virtual ~InnerNode() {};

    /**
     * Returns a pointer to the left child. This node still owns and manages the underlying pointer.
     * @return a pointer to the left child
     */
    Node *getLeftChild() const {
        return leftChild.get();
    }

    /**
     * Returns a pointer to the right child. This node still owns and manages the underlying pointer.
     * @return a pointer to the right child
     */
    Node *getRightChild() const {
        return rightChild.get();
    }

    vector<word_type> getWordMembers() override;

    void addWordsToMap(vector<pair<string, word_type>> &map, const string &startingAddress,
                       bool allowAddressExtension) const override;

    void setAmiLoss(double amiLoss);

    void setAmiAfterLoss(double amiAfterLoss);

    void setMergeID(word_type mergeID);

    word_type getWordLabel() override { return this->leftChild->getWordLabel(); };

    void printMerges(ofstream &stream, const Corpus &corpus) override {
        const word_type leftWordLabel = leftChild->getWordLabel();
        const word_type rightWordLabel = rightChild->getWordLabel();

        stream << std::setprecision(10) << corpus.idsToWords.at(leftWordLabel) << "\t"
               << corpus.idsToWords.at(rightWordLabel)
               << "\t" << this->mergeID << "\t" << this->amiLoss << "\t" << this->amiAfterLoss << endl;
        this->leftChild->printMerges(stream, corpus);
        this->rightChild->printMerges(stream, corpus);
    };
};

#endif /* INNERNODE_H_ */
