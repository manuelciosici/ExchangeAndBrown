#include "ReaderClusteringIntoCorpus.h"
#include "../CorpusUtils.h"
#include <fstream>
#include <algorithm>

using namespace std;

Corpus ReaderClusteringIntoCorpus::readFile(const std::string &fileNameCorpus, const std::string &fileNameClustering) {
    const Corpus corpus = Corpus::deserializeFromFile(fileNameCorpus);
    const vector<word_type> clusterAssignments = CorpusUtils::readClusterAssignmentsFromFile(fileNameClustering,
                                                                                             corpus);
    const word_type numberOfClusters = *std::max_element(clusterAssignments.begin(), clusterAssignments.end()) + 1;
    Corpus clusteredCorpus;
    clusteredCorpus.vocabularySize = numberOfClusters;
    clusteredCorpus.pl = vector<double>(numberOfClusters, 0);
    clusteredCorpus.pr = vector<double>(numberOfClusters, 0);

    for (const auto &elem : corpus.occurrences) {
                const word_type wordID1 = elem.first.first;
        const word_type wordID2 = elem.first.second;
        const word_type occ = elem.second;
        const word_type clusterID1 = clusterAssignments[wordID1];
        const word_type clusterID2 = clusterAssignments[wordID2];
        const pair<word_type, word_type> key = make_pair(clusterID1, clusterID2);
        auto it = clusteredCorpus.occurrences.find(key);
        if (it == clusteredCorpus.occurrences.end()) {
            clusteredCorpus.occurrences.insert({key, occ});
        } else {
            it->second += occ;
        }
    }
    return clusteredCorpus;
}

