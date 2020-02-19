#ifndef BROWN_WORDMAPPINGS_H
#define BROWN_WORDMAPPINGS_H

#include "../Utils.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <locale>
#include <fstream>
#include <unordered_map>

using namespace std;

class WordMappings {
private:
    word_type nextID;
    unordered_map<string, word_type> wordsToIDs;
    unordered_map<word_type, string> IDsToWords;
public:
    WordMappings() {
        nextID = 0;
    }

    auto getID(const string &word) {
        auto const it = wordsToIDs.find(word);
        if (it == wordsToIDs.end()) {
            word_type const wordID = nextID;
            wordsToIDs.insert({word, wordID});
            IDsToWords.insert({wordID, word});
            ++nextID;
            return wordID;
        }
        return (*it).second;
    }

    auto getIDWithoutAllocation(const string &word) {
        const word_type wordIDToReturn = wordsToIDs.at(word);
        return wordIDToReturn;
    }

    auto getWordForID(const word_type &wordID) const {
        const string& word = IDsToWords.at(wordID);
        return word;
    }

    auto alreadyHasID(const string &word) const {
        return wordsToIDs.find(word) != wordsToIDs.end();
    }
};


#endif
