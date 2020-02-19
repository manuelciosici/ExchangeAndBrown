#include <fstream>
#include "StochasticExchange.h"
#include <random>

vector<word_type> StochasticExchange::clusterInternal(const word_type noIterations, const double minAMIChange) {
    if (noIterations > 0) {
//        set up the random device
        std::random_device rd;
        std::mt19937 e2(rd());
        std::uniform_real_distribution<> dist(0, 1);
        std::uniform_int_distribution<> cluster_dist(0, numClusters - 1);

        changesInPreviousIteration = 1;
        vector<double> amiChange;
        int destinationCluster;
#pragma omp parallel
        for (iteration = 0; iteration < noIterations; ++iteration) {
            if (changesInPreviousIteration == 0 && !AMIIncreasingOverThreshold) {
                break;
            }
#pragma omp barrier
#pragma omp single
            {
                changesInPreviousIteration = 0;
            }
            for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
                const word_type clusterToMoveFrom = wordsToClusters[wordID];
                if (clusterContent[clusterToMoveFrom].size() > 1) {
#pragma omp barrier
#pragma omp single
                    {
                        destinationCluster = -1;
                        amiChange = vector<double>(numClusters, 0);
                        double coinFlip = dist(e2) * 100;
                        if (coinFlip < randomnessLevel) {
//                            random swap
                            destinationCluster = cluster_dist(e2);
                        }
                    }
                    if (destinationCluster == -1) {
#pragma omp for
                        for (word_type clusterCandidate = 0; clusterCandidate < numClusters; ++clusterCandidate) {
                            if (clusterCandidate == clusterToMoveFrom) {
                                amiChange[clusterCandidate] = 0;
                            } else {
                                const double amiDiff = calculateAMIDiff(wordID, clusterCandidate);
                                amiChange[clusterCandidate] = amiDiff;
                            }
                        }
#pragma omp single
                        {
                            const auto clusterToMoveTo = (word_type) distance(amiChange.begin(),
                                                                                   max_element(amiChange.begin(),
                                                                                               amiChange.end()));
                            if (clusterToMoveTo != wordsToClusters[wordID]) {
                                performMoveAndReturnAMIChange(wordID, clusterToMoveTo);
                                changesInPreviousIteration++;
                            }
                        }
                    } else {
#pragma omp single
                        {
                            performMoveAndReturnAMIChange(wordID, destinationCluster);
                            changesInPreviousIteration++;
                        }
                    }
                }
            }
#pragma omp single
            {
                AMIIncreasingOverThreshold = true;
            }
        }
    }
    return wordsToClusters;
}

bool StochasticExchange::clusterOneIteration(const double minAMIChange) {
    this->clusterInternal(1, minAMIChange);
    return AMIIncreasingOverThreshold;
}