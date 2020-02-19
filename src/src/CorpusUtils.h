#ifndef BROWN_CORPUSUTILS_H
#define BROWN_CORPUSUTILS_H


#include "Utils.h"
#include "models/Corpus.h"

class CorpusUtils {
public:
    /**
     * Reads cluster assignments from a text file. The text file is assumed to have numClusters rows, each row
     * containing the contained words separated by space.
     *
     * @return a vector where position i contains the cluster id for word with id i
     */
    static vector_word_type readClusterAssignmentsFromFile(const string &fileName, const Corpus &corpus);

    /**
     * Calculates how ofter each cluster appears (sum of appearances of all contained words).
     * @return vector containing frequency of cluster i on position i.
     */
    static vector_word_type calculateClusterFrequency(const vector_word_type &clusterAssignments, const Corpus &corpus);

    /**
     * Writes the contents of a tree to a text file in the format defined by Percy.
     * Read more about the format at https://github.com/percyliang/brown-cluster/tree/master/cluster-viewer
     */
    static void writeTreeToLiangFile(const string &outputFileName,
                                     const vector<pair<string, word_type>> &treeContents,
                                     const Corpus &corpus);
    /**
     * Writes the contents of a tree to a text file in the format defined by Percy.
     * Read more about the format at https://github.com/percyliang/brown-cluster/tree/master/cluster-viewer
     */
    static void writeTreeToLiangFile(const string &outputFileName,
                                     const vector<pair<string, word_type>> &treeContents,
                                     const Corpus &corpus,
                                     const vector<vector<word_type>> &clusterContent);

    static void writeClusterInteractionToJSON(const Corpus &corpus,
                                 const vector_word_type clusterAssignments, const string outputFile);

    /**
     * Dumps the vocabulary of a corpus into a file where each line has the following format:
     * word <space> word_frequency
     * @param corpus the corpus to dump from
     * @param outputFile name of file to write into
     */
    static void writeCorpusVocabularyToFile(const Corpus &corpus, const string &fileNameOutput);

    /**
     * Reads a vocabulary from the given file.
     * @param fileNameVocabulary file name
     * @return an unordered set containing word strings and their frequency in the corpus
     */
    static unordered_map<string, word_type> readVocabularyFromFile(const string &fileNameVocabulary);
};


#endif //BROWN_CORPUSUTILS_H
