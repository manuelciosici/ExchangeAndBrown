#ifndef BROWN_CORPUS_H
#define BROWN_CORPUS_H

#include "../Utils.h"
#include <map>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/string.hpp>
#include <cassert>

class Corpus {
public:
    Corpus(){
        this->vocabularySize = 0;
        this->corpusLength = 0;
    }

    bool operator == (const Corpus &Ref) const
    {
        bool equal = true;
        equal &= this->vocabularySize == Ref.vocabularySize;
        equal &= this->corpusLength == Ref.corpusLength;
        equal &= this->pl == Ref.pl;
        equal &= this->pr == Ref.pr;
        equal &= this->idsToWords == Ref.idsToWords;
        equal &= this->occurrences == Ref.occurrences;
        equal &= this->wordCountAsNumbers == Ref.wordCountAsNumbers;
        return equal;
    }

    /**
     * Size of vocabulary.
     */
    word_type vocabularySize;
    /**
     * Length of corpus.
     */
    word_type corpusLength;
    /**
     * Probabilities left.
     */
    vector<double> pl;
    /**
     * Probabilities right.
     */
    vector<double> pr;
    /**
     * Mapping between IDs and words.
     */
    unordered_map<word_type, string> idsToWords;
    /**
     * Map of occurrences between words.
     */
    occurrence_type occurrences;
    /**
     * Map of word counts.
     */
    unordered_map<word_type, word_type> wordCountAsNumbers;
    /**
     * Returns the number of transitions in the corpus.
     */
    word_type getNumberOfTransitions() const {
        return this->corpusLength - 1;
    }
    /**
     * Returns the string representation of a word ID
     * @param wordID
     * @return
     */
    optional<string> getWord(const word_type wordID) const {
        optional<string> valueToReturn = std::nullopt;
        if (wordID >= 0 && wordID < this->vocabularySize) {
            valueToReturn = this->idsToWords.at(wordID);
        }
        return valueToReturn;
    }

    /**
     * Writes clusters to file.
     * The file format is one cluster per line with words separated by comma. Words are ordered in decreasing order
     * of their probability in the cluster.
     */
    void writeClustersToFile(const string &outputFile,
                                    const vector_word_type &clusterAssignments,
                                    word_type num_clusters) const;
    /**
     * Method for serializing to a file.
     */
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar(vocabularySize);
        ar(corpusLength);
        ar(pl);
        ar(pr);
        ar(occurrences);
        ar(wordCountAsNumbers);
        ar(idsToWords);
    }
    /**
     * Method for deserializing from a file.
     */
    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar(vocabularySize);
        ar(corpusLength);
        ar(pl);
        ar(pr);
        ar(occurrences);
        ar(wordCountAsNumbers);
        ar(idsToWords);
    }
    /**
     * Serializes a corpus to a file.
     * @param c corpus to write
     * @param fileName path to file
     */
    static void serializeToFile(Corpus c, string fileName);
    /**
     * Deserializes a corpus from a file.
     * @param fileName path to corpus file
     * @return instance of contained corpus
     */
    static Corpus deserializeFromFile(string fileName);
    /**
     * Computes for every word in a corpus a list of words following the word and a list of words preceeding the word.
     * @return a pair consisting of the words to the left of a given word (preceeding) and the words to the right of
     * a given word (following).
     */
    static pair<matrix_occurrences, matrix_occurrences> computeLeftiesAndRighties(const Corpus &corpus);

};

#endif //BROWN_CORPUS_H
