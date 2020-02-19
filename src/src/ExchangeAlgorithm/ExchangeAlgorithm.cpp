/**
 * Implements some methods that are common to the various ExchangeAlgorithm 
 * implementations.
 */

#include <fstream>
#include "ExchangeAlgorithm.h"
#include <algorithm>

vector_word_type ExchangeAlgorithm::sortClusterAssignments(const vector_word_type inputClusterAssignments,
                                                           const word_type noClusters) {

    const word_type defaultValue = inputClusterAssignments.size();
    vector_word_type lowestWordIDs(noClusters, defaultValue);
    for (word_type i = 0; i < inputClusterAssignments.size(); ++i) {
        const word_type currentWordID = i;
        const word_type currentCluster = inputClusterAssignments[i];
        if (currentWordID < lowestWordIDs[currentCluster]) {
            lowestWordIDs[currentCluster] = currentWordID;
        }
    }

//    this stores clusterIDs together with the lowest
    vector<pair_of_word_type> clusterIDs = vector<pair_of_word_type>(noClusters);
    for (word_type i = 0; i < noClusters; ++i) {
        clusterIDs[i] = {i, lowestWordIDs[i]};
    }
    std::sort(clusterIDs.begin(), clusterIDs.end(),
              [](pair_of_word_type a, pair_of_word_type b) { return a.second < b.second; });

    vector_word_type clusterIDMappings = vector_word_type(noClusters, 0);
    for (word_type i = 0; i < noClusters; ++i) {
        clusterIDMappings[clusterIDs[i].first] = i;
    }

    vector_word_type vectorToReturn = vector_word_type(inputClusterAssignments.size());
    for (word_type i = 0; i < inputClusterAssignments.size(); ++i) {
        vectorToReturn[i] = clusterIDMappings[inputClusterAssignments[i]];
    }

    return vectorToReturn;
}
