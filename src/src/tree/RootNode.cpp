#include "RootNode.h"
#include <map>


vector<pair<string, word_type>> RootNode::getTreeAsBitAddressFormat() {
    vector<pair<string, word_type>> wordsWithAddresses;
    const string leftStartingAddress = LEFT_BIT;
    const string rightStartingAddress = RIGHT_BIT;
    this->leftChild->addWordsToMap(wordsWithAddresses, leftStartingAddress, true);
    this->rightChild->addWordsToMap(wordsWithAddresses, rightStartingAddress, true);

    return wordsWithAddresses;
}