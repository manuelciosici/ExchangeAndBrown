#ifndef BROWN_READERTHRESHOLD_H
#define BROWN_READERTHRESHOLD_H

#include <string>
#include "AbstractReader.h"


class ReaderThreshold : public AbstractReader {
public:
    /**
     * Takes in a corpus model and removes all words that appear less than 10 times.
     */
    const Corpus reorderCorpus(const Corpus corpus);

    /**
     * Takes in a corpus model and removes all words that appear less than the threshold.
     * @param corpus corpus to filter
     * @param threshold minimum frequency value (integer)
     * @param strict whether frequencies for words in the new corpus should be the original ones
     *      (including counts from bi-grams with words that have been removed). If this is selected,
     *      the new corpus length will be the same as the old corpus length. If not selected, the
     *      new corpus length will be the sum of all bi-grams that meet the threshold.
     */
    const Corpus
    reorderCorpus(const Corpus &corpus, const word_type thresholdVal, const bool strict);
};


#endif //BROWN_READERTHRESHOLD_H
