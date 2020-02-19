#ifndef BROWN_READERCLUSTERINGINTOCORPUS_H
#define BROWN_READERCLUSTERINGINTOCORPUS_H

#include "../models/Corpus.h"
#include <string>

class ReaderClusteringIntoCorpus {
public:
    Corpus readFile(const std::string &fileNameCorpus, const std::string &fileNameClustering);
};


#endif //BROWN_READERCLUSTERINGINTOCORPUS_H
