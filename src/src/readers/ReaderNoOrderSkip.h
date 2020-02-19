#ifndef BROWN_READERNOORDERSKIP_H
#define BROWN_READERNOORDERSKIP_H

#include "../models/Corpus.h"
#include <string>

/**
 * Reads from a flat text file into a Corpus object using a skip-gram model.
 */
class ReaderNoOrderSkip {
public:
    /**
     * Construct a Corpus object from the given file name using the provided window.
     * @param fileName Path to input text corpus file.
     * @param maxSkipGramWidth Window size for skip-gram. Skip-grams are constructed from the current work,
     * with all previous words within the provided window.
     * For example, for the corpus A B C D and maxSkipGramWidh = 2, the following pairs are constructed:
     * AB, BA, AC, CA, BD, DB, CD, DC.
     * @return Corpus object representing the text file.
     *
     */
    Corpus readFile(const std::string fileName, const uint32_t maxSkipGramWidth);

    /**
     * Construct a Corpus object from the given file name using the provided window and Corpus object. This method can
     * be used when the Corpus object has been pre-filtered for frequency.
     * @param fileNameCorpus Path to Corpus object to use.
     * @param fileName Path to input text corpus file.
     * @param maxSkipGramWidth Window size for skip-gram. Skip-grams are constructed from the current work,
     * with all previous words within the provided window.
     * For example, for the corpus A B C D and maxSkipGramWidh = 2, the following pairs are constructed:
     * AB, BA, AC, CA, BD, DB, CD, DC.
     * @return Corpus object representing the text file.
     *
     */
    Corpus readFile(const std::string &fileNameCorpus, const std::string fileNameVocabulary,
                    const uint32_t maxSkipGramWidth);
};


#endif //BROWN_READERNOORDERSKIP_H
