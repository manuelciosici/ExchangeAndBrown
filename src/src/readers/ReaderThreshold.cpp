#include "ReaderThreshold.h"
#include <unordered_map>

using namespace std;

const Corpus ReaderThreshold::reorderCorpus(const Corpus corpus) {
    return reorderCorpus(corpus, 10, true);
}

const Corpus ReaderThreshold::reorderCorpus(const Corpus &corpus, const word_type thresholdVal, const bool strict) {
    Corpus orderedCorpus;
    unordered_map<word_type, word_type> idMappings;
    word_type nextID = 0;
    for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
        if (corpus.pl[wordID] >= thresholdVal) {
            const word_type idForCurrentWord = nextID;
            nextID++;
            idMappings.insert({wordID, idForCurrentWord});
            if (strict) {
                orderedCorpus.pl.push_back(corpus.pl[wordID]);
                orderedCorpus.pr.push_back(corpus.pr[wordID]);
            }
            const auto wordOptional = corpus.getWord(wordID);
            if (wordOptional) {
                const string& word = wordOptional.value();
                orderedCorpus.idsToWords.insert({idForCurrentWord, word});
            }
        }
    }
    if (!strict) {
        orderedCorpus.pl = vector<double>(idMappings.size(), 0);
        orderedCorpus.pr = vector<double>(idMappings.size(), 0);
        orderedCorpus.corpusLength = 0;
    } else {
        orderedCorpus.corpusLength = corpus.corpusLength;
    }

    orderedCorpus.vocabularySize = idMappings.size();

    for (const auto elem : corpus.occurrences) {
        const word_type &id1 = elem.first.first;
        const word_type &id2 = elem.first.second;
        const word_type &value = elem.second;
        if (idMappings.find(id1) != idMappings.end() && idMappings.find(id2) != idMappings.end()) {
            const word_type id1Mapped = idMappings[id1];
            const word_type id2Mapped = idMappings[id2];
            orderedCorpus.occurrences.insert({{id1Mapped, id2Mapped}, value});
            if (!strict) {
                orderedCorpus.pl[id1Mapped] += value;
                orderedCorpus.pr[id2Mapped] += value;
                orderedCorpus.corpusLength += value;
            }
        }
    }
    return orderedCorpus;
}
