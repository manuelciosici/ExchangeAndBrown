#ifndef BROWNCLUSTERING_H_
#define BROWNCLUSTERING_H_


#include <string>
#include "../Utils.h"
#include "../tree/RootNode.h"
#include "../models/Corpus.h"
#include <chrono>

class BrownClusteringAlgorithm {
public:
    BrownClusteringAlgorithm(const Corpus &corpus);

    virtual ~BrownClusteringAlgorithm() = default;

    /**
     * Returns a pointer to the clustering tree. This object maintains ownership of the unique_ptr.
     * @return a pointer to the clustering tree
     */
    RootNode *getClusterTree() const {
        return this->clustering.getClusterTree();
    };

    /**
    * Clusters words.
    * @param noCluster number of desired clusters
    * @param windowSize size od decision window
    */
    vector_word_type cluster(word_type noClusters, word_type windowSize);

/**
     * Moves the item at the end of the vector to position position and resizes the vector to be by 1 smaller.
     * This is the matrix version that will also resize the matrix so that it's one column smaller.
     * @tparam T
     * @param matrix
     * @param position to what position to move the last item
     * @param sizeToConsider number of items to consider present in the matrix
     */
    template<typename T>
    static void swapForMatrix(vector<vector<T>> &matrix, const word_type position, const word_type sizeToConsider) {
        const word_type lastPosition = sizeToConsider - 1;
        matrix[position] = matrix[lastPosition];
        for (word_type i = 0; i < sizeToConsider; ++i) {
            matrix[i][position] = matrix[i][lastPosition];
        }
    }

    /**
     * Moves the item at the end of the vector to position "position".
     * @tparam T
     * @param v
     * @param position
     * @param sizeToConsider
     */
    template<typename T>
    static void swapForVector(vector<T> &v, const word_type position, const word_type sizeToConsider) {
        const word_type lastPosition = sizeToConsider - 1;
        v[position] = v[lastPosition];
    }

    /**
     * Moves the item at the end of the vector to position "position" and resizes the vector to be by 1 smaller.
     * @tparam T
     * @param v
     * @param position
     * @param sizeToConsider
     */
    template<typename T>
    static void swapForVector(vector<unique_ptr<T>> &v, const word_type position, const word_type sizeToConsider) {
        const word_type lastPosition = sizeToConsider - 1;
        for (word_type i = position; i < lastPosition; ++i) {
            v[i] = std::move(v[i + 1]);
        }
    }

    /**
     * This is an implementation of equation 16 from Brown.
     * @param sk
     * @param q
     * @param currentWindowSize
     */
    static void computeSk(vector<double> &sk, const matrix_double &q, word_type currentWindowSize);

private:
    struct MergeData {
        word_type from;
        word_type to;
        double amiLoss;
        double amiAfterLoss;
        word_type mergeID;
    };

    /**
     * Models a flat and hierarchical clustering and is used by BrownClusteringAlgorithm internally.
     */
    class Clustering {
    private:
        matrix_occurrences clusterContent;
        vector_word_type wordsToClusters;
        word_type maxClusterID;
        word_type vocabularySize;
        unique_ptr<RootNode> root;
        vector<unique_ptr<Node>> clusterToTree;
    public:
        /**
         * Constructor. Initialized the internal data structures to default values
         * (i.e. first numberOfClusters elements are assigned to their own cluster).
         * @param numberOfClusters
         * @param vocabularySize
         */
        Clustering(word_type numberOfClusters, word_type vocabularySize);

        /**
         * Moves all elements from fromID to toID.
         * @param from
         * @param to
         */
        void mergeClusters(const BrownClusteringAlgorithm::MergeData &mergeData);

        /**
         * Moves the last cluster from its position to the given one.
         * @param destinationID
         */
        void moveLastCluster(word_type destinationID);

        /**
         * Assigns the given word to the given cluster. Requires that the destination cluster is empty.
         * @param wordID word to assign
         * @param destinationClusterID cluster to assign to
         */
        void assignWordToEmptyCluster(word_type wordID, word_type destinationClusterID);

        /**
         * Assigns the given leaf to a clusterID
         * @param leaf
         * @param destinationClusterID
         */
        void assignLeafToCluster(unique_ptr<LeafNode> leaf, word_type destinationClusterID);

        /**
         * Marks all current nodes as final clusters.
         */
        void setAllCurrentNodesAsFinal();

        const auto getWordsToClusters() const {
            return this->wordsToClusters;
        }

        /**
         * Attaches the first two node pointers to the root.
         */
        void attachToRoot(word_type mergeID);

        /**
         * Returns a pointer to the clustering tree. This object maintains ownership of the unique_ptr.
         * @return a pointer to the clustering tree
         */
        RootNode *getClusterTree() const {
            if (this->root == nullptr) {
                string errorMsg("Cannot return tree as root is not set. Is clustering finished?");
                cerr << errorMsg << endl;
                throw runtime_error(errorMsg);
            } else {
                return this->root.get();
            }
        };
    };

    Clustering clustering;
    /**
     * Ordered list of one-word clusters (also known as leaves) to process.
     */
    std::queue<std::unique_ptr<LeafNode>> leavesToProcess;

    /**
     * Returns the next leave that requires processing.
     * @return the next leave that requires processing or nullptr if no more leaves require processing
     */
    unique_ptr<LeafNode> getNextCluster() {
        if (!this->leavesToProcess.empty()) {
            auto n = std::move(this->leavesToProcess.front());
            this->leavesToProcess.pop();
            return n;
        } else {
            return nullptr;
        }
    }

    /**
     * Performs most of the necessary updates to s_k after a merge.
     * This method corresponds to the first two lines in equation 17 in Brown, minus the last two terms in each
     * equation, which are handled by BrownClusteringAlgorithm::updateQMergedCluster
     * @param intoID
     * @param fromID
     * @param currentWindowSize
     */
    void updateSkAfterRemovalOf(word_type intoID, word_type fromID, word_type currentWindowSize);


    /**
     * Calculates q as a matrix containing every possible variation of parameters.
     * In Brown this is equation 12.
     */
    static double computeQInitial(matrix_double &q, const matrix_occurrences &occurrencesC,
                                  word_type currentWindowSize, vector<double> &plC, vector<double> &prC,
                                  word_type corpusLength);

    Corpus corpus;
    double oldI = 0;
    matrix_occurrences toTheLeftOf;
    matrix_occurrences toTheRightOf;
    vector<double> plC;
    vector<double> prC;
    vector<double> sk;
    matrix_occurrences occurrencesC;
    matrix_double q;

    /**
     * Initializes the internal data structures.
     * @param windowSize
     */
    void initializeDataStructures(word_type windowSize);

    /**
     * Updates Q for the given row and column. This should be used after a merge, after all
     * other data structures have been updated.
     * It implements:
     * - The last two terms of the first two lines of Equation 17 in Brown.
     * - Equation 16 in Brown.
     * - Updates to elements of the matrix containing Equation 14 in Brown that were involved in a merge.
     * @param intoID
     * @param currentWindowSize
     * @param corpusLength
     */
    void updateQMergedCluster(word_type intoID, word_type currentWindowSize, word_type corpusLength);

    double
    computeQMergeContribution(word_type i, word_type j, word_type currentWindowSize, word_type corpusLength) const;

    /**
     * Finds the pair of clusters to merge so as to lose the least AMI.
     * @param currentWindowSize size of current window
     * @param limitI the highest ID of a cluster to consider merging into
     * @return a pair of clusters to merge. The first element is always the cluster with the smaller ID.
     */
    MergeData findLowestAMILoss(word_type currentWindowSize, word_type limitI) const;

    /**
     * Updates Q for the given row and column. This should be used after a merge, after all
     * other data structures have been updated.
     * It implements:
     * - Parts of Equation 17 in Brown.
     * - AMI update after inclusion of a new cluster into the active set (i.e. window).
     * - Q matrix (Equation 14) for the newly created cluster.
     * @param fromID
     * @param intoID
     * @param currentWindowSize
     * @param corpusLength
     */
    void updateQNewCluster(word_type fromID, word_type intoID, word_type currentWindowSize, word_type corpusLength);

    void reduceOneCluster(const MergeData &merge, word_type currentWindowSize, word_type corpusLength);

    void updateQReduction(word_type intoID, word_type fromID, word_type currentWindowSize, word_type corpusLength);

    void updateQInclusion(word_type intoID, word_type fromID, word_type currentWindowSize, word_type corpusLength);
};

#endif /* BROWNCLUSTERING_H_ */
