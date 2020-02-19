#ifndef BROWN_READERNOORDER_H
#define BROWN_READERNOORDER_H

#include "../models/Corpus.h"
#include <string>

/**
 * Reads a text corpus into a Corpus object. Words are allocated IDs in the order they appear in text.
 */
class ReaderNoOrder {
public:
    Corpus readFile(std::string fileName);
};


#endif //BROWN_READERNOORDER_H
