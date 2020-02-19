#include "ReaderNoOrder.h"
#include <fstream>
#include <unordered_map>

using namespace std;

Corpus ReaderNoOrder::readFile(const std::string fileName) {
    std::cout << "Reading file " << fileName << std::endl;
    ifstream file;
    file.open(fileName.c_str());
    if (!file.is_open()) {
        throw runtime_error("File " + fileName + " could not be opened!");
    }
    Corpus corpus;
    string word;
    bool previousIdWasSetUp = false;
    word_type previousId = 0;

    unordered_map<string, word_type> wordsToIds;
//    count frequency of all words and measure vocabulary and corpus size
    while (file >> word) {
        word_type currentId = 0;
        const auto iteratorID = wordsToIds.find(word);
        if (iteratorID == wordsToIds.end()) {
            currentId = corpus.vocabularySize;
            wordsToIds.insert({word, currentId});
            corpus.idsToWords.insert({currentId, word});
            ++corpus.vocabularySize;
            corpus.pl.push_back(0);
            corpus.pr.push_back(0);
            corpus.wordCountAsNumbers.insert({currentId, 0});
        } else {
            currentId = iteratorID->second;
        }
        ++corpus.corpusLength;
        if (previousIdWasSetUp) {
            ++corpus.pl[previousId];
            ++corpus.pr[currentId];
            const occurrence_type::key_type key = {previousId, currentId};
            if (corpus.occurrences.find(key) != corpus.occurrences.end()) {
                ++corpus.occurrences[key];
            } else {
                corpus.occurrences.emplace(key, 1);
            }
        } else {
            previousIdWasSetUp = true;
        }
        previousId = currentId;
        corpus.wordCountAsNumbers[currentId] += 1;
    }
    #pragma omp parallel for simd
    for (auto i = 0; i < corpus.vocabularySize; ++i) {
        corpus.pl[i] = corpus.pl[i] / ( corpus.corpusLength - 1 );
        corpus.pr[i] = corpus.pr[i] / ( corpus.corpusLength - 1 );
    }
    file.close();
    return corpus;
}

