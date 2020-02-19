#ifndef BROWN_READERFREQUENCY_H
#define BROWN_READERFREQUENCY_H

#include <string>
#include "AbstractReader.h"


class ReaderFrequency : public AbstractReader {
public:
    /**
     * Takes in a corpus model and reorders it so that all word are sorted descending based on their frequency.
     */
    const Corpus reorderCorpus(Corpus corpus) override;

};


#endif //BROWN_READERFREQUENCY_H
