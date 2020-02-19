#ifndef BROWN_ABSTRACTREADER_H
#define BROWN_ABSTRACTREADER_H

#include "../models/Corpus.h"

class AbstractReader {
public:
    /**
     * Reads the corpus and provides a model of the corpus.
     */
    virtual const Corpus reorderCorpus(Corpus corpus) = 0;
};

#endif //BROWN_ABSTRACTREADER_H
