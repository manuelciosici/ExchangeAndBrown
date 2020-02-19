#include <fstream>
#include "ReaderFrequency.h"
#include <unordered_map>
#include <algorithm>

using namespace std;

const Corpus ReaderFrequency::reorderCorpus(const Corpus corpus) {
    Corpus orderedCorpus;
    orderedCorpus.corpusLength = corpus.corpusLength;
    orderedCorpus.vocabularySize = corpus.vocabularySize;
    orderedCorpus.pl = vector<double>(corpus.vocabularySize);
    orderedCorpus.pr = vector<double>(corpus.vocabularySize);

    std::vector<pair<word_type, word_type>> wordsWithOccurrences;
    wordsWithOccurrences.reserve(corpus.vocabularySize);
    for (word_type i = 0; i < corpus.pl.size(); ++i) {
        wordsWithOccurrences.emplace_back(i, corpus.pl[i]);
    }
    std::sort(wordsWithOccurrences.begin(), wordsWithOccurrences.end(),
              [](pair<word_type, word_type> a, pair<word_type, word_type> b) { return a.second > b.second; });

    unordered_map<word_type, word_type> oldIdsToNewIds;
//    store the mapping of words to numbers
    for (word_type i = 0; i < corpus.vocabularySize; ++i) {
        const word_type oldWordIndex = wordsWithOccurrences[i].first;
        const auto wordOptional = corpus.getWord(oldWordIndex);
        if (wordOptional) {
            const string word = wordOptional.value();
            oldIdsToNewIds.insert({oldWordIndex, i});
            orderedCorpus.idsToWords.insert({i, word});
            orderedCorpus.wordCountAsNumbers.insert({i, wordsWithOccurrences[i].second});
            orderedCorpus.pl[i] = corpus.pl[oldWordIndex];
            orderedCorpus.pr[i] = corpus.pr[oldWordIndex];
        }
    }
    for (const auto &occurrence : corpus.occurrences) {
        const word_type value = occurrence.second;
        const word_type newFirst = oldIdsToNewIds.at(occurrence.first.first);
        const word_type newSecond = oldIdsToNewIds.at(occurrence.first.second);
        orderedCorpus.occurrences[{newFirst, newSecond}] = value;
    }
    return orderedCorpus;
}
