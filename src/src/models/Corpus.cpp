#include "Corpus.h"
#include <fstream>
#include <algorithm>

void Corpus::serializeToFile(const Corpus c, const string fileName) {
    std::ofstream ofs(fileName, std::ios::binary);
    cereal::PortableBinaryOutputArchive oa(ofs);
    oa(c);
    ofs.close();
}

Corpus Corpus::deserializeFromFile(const string fileName) {
    Corpus newCorpus;
    std::ifstream ifs(fileName);
    if (!ifs.is_open()) {
        throw runtime_error("File " + fileName + " could not be opened");
    }
    cereal::PortableBinaryInputArchive ia(ifs);
    ia(newCorpus);
    return newCorpus;
}

pair<matrix_occurrences, matrix_occurrences>
Corpus::computeLeftiesAndRighties(const Corpus &corpus) {
    pair<matrix_occurrences, matrix_occurrences> result;

    result.first = matrix_occurrences(corpus.vocabularySize, vector_word_type());
    result.second = matrix_occurrences(corpus.vocabularySize, vector_word_type());

    for (auto occurrence : corpus.occurrences) {
        result.first[occurrence.first.second].push_back(occurrence.first.first);
        result.second[occurrence.first.first].push_back(occurrence.first.second);
    }
    return result;
};

void Corpus::writeClustersToFile(const string &outputFile,
                                 const vector_word_type &clusterAssignments,
                                 const word_type num_clusters) const {
    vector<vector<pair<word_type, word_type>>> clusterContent(num_clusters, vector<pair<word_type, word_type>>());
    vector<word_type> clusterFrequencies(num_clusters, 0);
    for (word_type i = 0; i < vocabularySize; ++i) {
        const word_type clusterID = clusterAssignments[i];
        clusterContent[clusterID].push_back({i, pl[i]});
        clusterFrequencies[clusterID] += pl[i];
    }
    vector<pair<word_type, word_type>> clusterContentFreqForSort;
    for (word_type i = 0; i < num_clusters; ++i) {
        float const frequency = clusterFrequencies[i];
        clusterContentFreqForSort.push_back({i, frequency});
    }
    std::sort(clusterContentFreqForSort.begin(), clusterContentFreqForSort.end(),
              [](pair<word_type, word_type> a, pair<word_type, word_type> b) { return a.second > b.second; });
    ofstream output(outputFile);
    for (auto cluster : clusterContentFreqForSort) {
        word_type const clusterID = cluster.first;
        auto content = clusterContent[clusterID];
        std::sort(content.begin(), content.end(),
                  [](pair<word_type, word_type> a, pair<word_type, word_type> b) { return a.second > b.second; });
        for (pair<word_type, word_type> pair : content) {
            const auto wordOptional = getWord(pair.first);
            if (wordOptional) {
                output << wordOptional.value() << ", ";
            }
        }
        output << endl;
    }
    output.close();
}