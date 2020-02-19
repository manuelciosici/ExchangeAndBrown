#include <fstream>
#include <unordered_map>
#include "../models/Corpus.h"
#include "ReaderNoOrderSkip.h"
#include "../CorpusUtils.h"

using namespace std;

Corpus ReaderNoOrderSkip::readFile(const std::string fileName, const uint32_t maxSkipGramWidth) {
    ifstream file;
    file.open(fileName.c_str());
    if (!file.is_open()) {
        throw runtime_error("File " + fileName + " could not be opened!");
    }
    Corpus corpus;
    string word;
    vector<bool> previousIdWasSetUp(maxSkipGramWidth, false);
    vector<word_type> previousId(maxSkipGramWidth, 0);

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
        } else {
            currentId = iteratorID->second;
        }
        for (int i = maxSkipGramWidth - 1; i >= 0; --i) {
            if (previousIdWasSetUp[i] == true) {
                ++corpus.corpusLength;
                ++corpus.pl[previousId[i]];
                ++corpus.pr[currentId];
                const occurrence_type::key_type key = {previousId[i], currentId};
                if (corpus.occurrences.find(key) != corpus.occurrences.end()) {
                    ++corpus.occurrences[key];
                } else {
                    corpus.occurrences.emplace(key, 1);
                }
                ++corpus.corpusLength;
                ++corpus.pl[currentId];
                ++corpus.pr[previousId[i]];
                const occurrence_type::key_type key2 = {currentId, previousId[i]};
                if (corpus.occurrences.find(key2) != corpus.occurrences.end()) {
                    ++corpus.occurrences[key2];
                } else {
                    corpus.occurrences.emplace(key2, 1);
                }
            } else {
                break;
            }
        }

        for (uint32_t i = 0; i < maxSkipGramWidth - 1; ++i) {
            previousIdWasSetUp[i] = previousIdWasSetUp[i + 1];
            previousId[i] = previousId[i + 1];
        }
        previousId[maxSkipGramWidth - 1] = currentId;
        previousIdWasSetUp[maxSkipGramWidth - 1] = true;

    }
    file.close();
    return corpus;
}

Corpus ReaderNoOrderSkip::readFile(const std::string &fileNameCorpus,
                                   const std::string fileNameVocabulary,
                                   const uint32_t maxSkipGramWidth) {
    ifstream file;
    file.open(fileNameCorpus.c_str());
    if (!file.is_open()) {
        throw runtime_error("File " + fileNameCorpus + " could not be opened!");
    }
    Corpus corpus;
    string word;
    const unordered_map<string, word_type> vocabulary = CorpusUtils::readVocabularyFromFile(
            fileNameVocabulary);
//    const size_t sizeOfPreviousWords = 5;
    vector<bool> previousIdWasSetUp(maxSkipGramWidth, false);
    vector<word_type> previousId(maxSkipGramWidth, 0);
    vector<bool> previousIdIsUsable(maxSkipGramWidth, 0);

    unordered_map<string, word_type> wordsToIds;
//    count frequency of all words and measure vocabulary and corpus size
    while (file >> word) {
        word_type currentId = 0;
        bool currentWordIsInVocabulary = vocabulary.find(word) != vocabulary.end();
        const auto iteratorID = wordsToIds.find(word);
        if (iteratorID == wordsToIds.end()) {
            if (currentWordIsInVocabulary) {
                currentId = corpus.vocabularySize;
                wordsToIds.insert({word, currentId});
                corpus.idsToWords.insert({currentId, word});
                ++corpus.vocabularySize;
                corpus.pl.push_back(0);
                corpus.pr.push_back(0);
            } else {
//                at this point currentID will stay 0
            }
        } else {
            currentId = iteratorID->second;
        }
        if (currentWordIsInVocabulary) {
//        add skip-grams
            for (int i = maxSkipGramWidth - 1; i >= 0; --i) {
                if (previousIdWasSetUp[i] == true) {
                    if (previousIdIsUsable[i]) {
//                    incoming skip-grams (from a word that has already appeared within the window)
                        ++corpus.corpusLength;
                        ++corpus.pl[previousId[i]];
                        ++corpus.pr[currentId];
                        const occurrence_type::key_type key = {previousId[i], currentId};
                        if (corpus.occurrences.find(key) != corpus.occurrences.end()) {
                            ++corpus.occurrences[key];
                        } else {
                            corpus.occurrences.emplace(key, 1);
                        }
                        ++corpus.corpusLength;
                        ++corpus.pl[currentId];
                        ++corpus.pr[previousId[i]];
                        const occurrence_type::key_type key2 = {currentId, previousId[i]};
                        if (corpus.occurrences.find(key2) != corpus.occurrences.end()) {
                            ++corpus.occurrences[key2];
                        } else {
                            corpus.occurrences.emplace(key2, 1);
                        }
                    }
                } else {
                    break;
                }
            }
        }
        for (uint32_t i = 0; i < maxSkipGramWidth - 1; ++i) {
            previousIdWasSetUp[i] = previousIdWasSetUp[i + 1];
            previousId[i] = previousId[i + 1];
            previousIdIsUsable[i] = previousIdIsUsable[i + 1];
        }
        previousId[maxSkipGramWidth - 1] = currentId;
        previousIdWasSetUp[maxSkipGramWidth - 1] = true;
        previousIdIsUsable[maxSkipGramWidth - 1] = currentWordIsInVocabulary;

    }
    file.close();
    return corpus;
}

