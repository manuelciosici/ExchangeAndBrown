#include <iostream>
#include "InnerNode.h"

InnerNode::InnerNode(std::unique_ptr<Node> leftChildVal, std::unique_ptr<Node> rightChildVal) {
    bool leftNull = leftChildVal == nullptr;
    bool rightNull = rightChildVal == nullptr;
    if (leftNull || rightNull) {
        string errorMsg("One of the given children is a null reference left is null: " + to_string(leftNull) + ", right is null: " + to_string(rightNull));
        cerr<<errorMsg << endl;
        throw runtime_error(errorMsg);
    }
    this->leftChild = std::move(leftChildVal);
    this->rightChild = std::move(rightChildVal);
}

vector<word_type> InnerNode::getWordMembers() {
    vector<word_type> left = this->leftChild->getWordMembers();
    vector<word_type> right = this->rightChild->getWordMembers();
    std::copy(right.begin(), right.end(), std::back_inserter(left));
    return left;
}

void InnerNode::addWordsToMap(vector<pair<string, word_type>> &map, const string& startingAddress,
                              const bool allowAddressExtension) const {
    string leftStartingAddress = startingAddress;
    string rightStartingAddress = startingAddress;
    bool shouldAllowAddressExtension = false;
    if (!this->getIsFinalCluster() && allowAddressExtension) {
        leftStartingAddress += LEFT_BIT;
        rightStartingAddress += RIGHT_BIT;
        shouldAllowAddressExtension = true;
    }
    this->leftChild->addWordsToMap(map, leftStartingAddress, shouldAllowAddressExtension);
    this->rightChild->addWordsToMap(map, rightStartingAddress, shouldAllowAddressExtension);
}

void InnerNode::setAmiLoss(double amiLoss) {
    InnerNode::amiLoss = amiLoss;
}

void InnerNode::setAmiAfterLoss(double amiAfterLoss) {
    InnerNode::amiAfterLoss = amiAfterLoss;
}

void InnerNode::setMergeID(word_type mergeID) {
    InnerNode::mergeID = mergeID;
};