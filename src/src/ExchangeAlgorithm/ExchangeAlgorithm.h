/**
 * A virtual class describing the behaviour of any ExchangeAlgorithm implementation 
 * and implementing some utilities common to any ExchangeAlgorithm implementation.
 */

#ifndef EXCHANGE_ALGORITHM_H
#define EXCHANGE_ALGORITHM_H


#include "../models/Corpus.h"

class ExchangeAlgorithm {
public:
    /**
     * Default minimum AMI change that must be observed between two iterations, otherwise the
     * clustering is considered converged.
     */
    constexpr static double DEFAULT_MIN_AMI_CHANGE = 0.0001;

    /**
     * Constructs a new instance of an ExchangeAlgorithm with a given corpus.
     */
    ExchangeAlgorithm(const Corpus &corpus) : corpus(corpus) {};

    /**
     * Destructs the ExchangeAlgorithm solver.
     */
    virtual ~ExchangeAlgorithm() {};

    /**
     * Runs the ExchangeAlgorithm for up to noIterations iterations to produce
     * noClusters clusters.
     * @param noClusters the number of desired clusters
     * @param noIterations the maximum number of iterations to execute
     * @param minAMIChange minimum AMI stopping threshold
     */
    virtual vector_word_type
    cluster(word_type noClusters, word_type noIterations, double minAMIChange) = 0;

    /**
     * Runs the ExchangeAlgorithm for up to noIterations iterations to produce
     * noClusters clusters. This version of the method takes the initial assignment of word to clusters as a parameter.
     * @parameter clusterAssignments assignment of words to clusters clusterAssignments[wordID] = clusterID
     * @param noClusters the number of desired clusters
     * @param noIterations the maximum number of iterations to execute
     * @param minAMIChange minimum AMI stopping threshold
     */
    virtual vector<word_type>
    cluster(word_type numClusters, word_type noIterations, vector<word_type> clusterAssignments,
            double minAMIChange) = 0;

    /**
     * Calculates the AMI of a set of clusters.
     * @return The AMI of the current clustering.
     */
    virtual double calculateAMI() = 0;

    /**
     * Takes a vector of cluster assignments and changes cluster IDs so that cluster IDs are in ascending order of word
     * IDs. For example, if in the input consists only of word 4 is assigned to cluster 2 and word 5 is assigned to
     * cluster 1, in the output word 4 will be assigned to cluster 1 and word 5 to cluster 2.
     */
    static vector_word_type sortClusterAssignments(vector_word_type inputClusterAssignments, word_type noClusters);

    /**
     * Returns the implementation name to be used for distinguishing between different children of ExchangeAlgorithm.
     * @return name of the implementation
     */
    virtual string getName() = 0;

    /**
     * Returns minimum AMI change that must be observed between two iterations, otherwise the
     * clustering is considered converged.
     * @return the default minimum AMI change
     */
    auto getMinAMIChange() {
        return ExchangeAlgorithm::DEFAULT_MIN_AMI_CHANGE;
    }

protected:
    /**
     * The corpus over which the clustering is to be conducted
     */
    const Corpus corpus;
    /**
     * The number of clusters in the clustering
     */
    word_type numClusters;
    /**
     * Mapping between word ids and cluster ids
     */
    vector_word_type wordsToClusters;




};

#endif //EXCHANGE_ALGORITHM_H
