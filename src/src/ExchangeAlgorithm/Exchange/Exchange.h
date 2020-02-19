#ifndef EXCHANGE_H
#define EXCHANGE_H


#include "../../models/Corpus.h"
#include "../ExchangeAlgorithm.h"
#include <set>
#include <algorithm>

/**
 * Multi-threaded implementation of ExchangeAlgorithm.
 */
class Exchange : public ExchangeAlgorithm {
protected:
    bool initialized = false;
    uint32_t changesInPreviousIteration = 0;
    bool AMIIncreasingOverThreshold = true;
    word_type iteration;
    matrix_occurrences occurrencesClusters;
    vector<double> sumColumnsEntropyOccurrences;
    vector<double> sumRowsEntropyOccurrences;
    vector<double> entropyLeft;
    vector<double> entropyRight;
    matrix_double entropyOccurrences;
    vector<double> plC;
    vector<double> prC;
    matrix_occurrences toTheLeftOf;
    matrix_occurrences toTheRightOf;
    vector<set<word_type>> clusterContent;
    vector<vector_word_type> clusterToWord;
    vector<vector_word_type> wordToCluster;

    double calculateAMIDiff(word_type wordID, word_type clusterCandidate);

    void performMoveAndReturnAMIChange(word_type wordID, word_type clusterToMoveTo);

    /**
     * Calculates entropy for the given input. Handles input = 0 internally.
     * In other words, in calculates: input * log2(input).
     * @param input
     * @return
     */
    double inline entropyTerm(double input);

public:
    uint32_t getChangesInPreviousIteration() const;

    Exchange(const Corpus &corpus) : ExchangeAlgorithm(corpus) {
        this->initialized = false;
    };

    ~Exchange() override = default;

    vector<word_type>
    cluster(word_type numClusters, word_type noIterations, double minAMIChange = DEFAULT_MIN_AMI_CHANGE) override;

    vector<word_type>
    cluster(word_type noClusters, word_type noIterations, vector<word_type> clusterAssignments,
            double minAMIChange = DEFAULT_MIN_AMI_CHANGE) override;

    /**
     * Initializes the data structures and sets the initial cluster assignments match the provided assignments.
     * @param numClusters number of desired clusters
     * @param clusterAssignments initial cluster assignments
     */
    void prepareClustering(word_type numClusters, const vector<word_type> &clusterAssignments);

    /**
     * Initializes the data structures and sets the initial cluster assignments to the default assignments.
     * @param numClusters number of desired clusters
     */
    void prepareClustering(word_type numClusters);

    /**
     * Runs EXCHANGE for one iteration.
     * @param minAMIChange minimum AMI threshold.
     * @return whether this clustering should be considered converged (either no swaps occurred, or
     * the minimum AMI change was below the provided threshold)
     */
    bool clusterOneIteration(double minAMIChange = DEFAULT_MIN_AMI_CHANGE);

    vector_word_type getClusterAssignments() const;

    /**
     * Runs initialisation steps to prepare data structures for clustering.
     */
    void initializeDataStructures(word_type numClusters, const vector<word_type> &clusterAssignments);


    double calculateAMI() override;

    string getName() override { return "Exchange"; };

    auto getIterations() { return this->iteration; }

protected:
    vector<word_type> clusterInternal(word_type noIterations, double minAMIChange);
};


#endif //EXCHANGE_H
