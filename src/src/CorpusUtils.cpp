#include "CorpusUtils.h"
#include <fstream>
#include "models/Corpus.h"
#include <json/json.hpp>

using json = nlohmann::json;

vector_word_type CorpusUtils::readClusterAssignmentsFromFile(const string &fileName, const Corpus &corpus) {
    unordered_map<string, word_type> wordsToIds;
    for (word_type i = 0; i < corpus.vocabularySize; i++) {
        const auto wordOptional = corpus.getWord(i);
        if (wordOptional) {
            const string word = wordOptional.value();
            wordsToIds.insert({word, i});
        }
    }
    vector_word_type valueToReturn(corpus.vocabularySize, 0);
    ifstream file(fileName);
    if (!file.is_open()) {
        throw runtime_error("File " + fileName + " could not be opened!");
    }
    string line;
    word_type lineNumber = 0;
    const string delimiter = ", ";
    while (std::getline(file, line)) {
        auto start = 0U;
        auto end = line.find(delimiter);
        while (end != std::string::npos) {
            const string word = line.substr(start, end - start);
            const auto it = wordsToIds.find(word);
            if (it != wordsToIds.end()) {
                const word_type wordID = (*it).second;
                valueToReturn[wordID] = lineNumber;
            } else {
                cerr << "Could not find word \"" << word << "\" in the corpus " << endl;
            }
            start = end + delimiter.length();
            end = line.find(delimiter, start);
        }
        ++lineNumber;
    }
    return valueToReturn;
}

vector_word_type CorpusUtils::calculateClusterFrequency(const vector_word_type &clusterAssignments,
                                                        const Corpus &corpus) {
    const word_type numClusters = *std::max_element(clusterAssignments.begin(), clusterAssignments.end()) + 1;
    vector_word_type valueToReturn(numClusters, 0);
    for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
        const word_type wordFrequency = corpus.wordCountAsNumbers.at(wordID);
        const word_type clusterID = clusterAssignments[wordID];
        valueToReturn[clusterID] += wordFrequency;
    }
    return valueToReturn;
}

void CorpusUtils::writeTreeToLiangFile(const string &outputFileName,
                                       const vector<pair<string, word_type>> &treeContents,
                                       const Corpus &corpus) {
    ofstream output(outputFileName);
    if (!output.is_open()) {
        throw runtime_error("File " + outputFileName + " could not be opened!");
    }
    for (auto const &item : treeContents) {
        const string &address = item.first;
        const word_type &wordID = item.second;
        const auto wordOptional = corpus.getWord(wordID);
        if (wordOptional) {
            const string &wordString = wordOptional.value();
            const word_type &wordCount = corpus.wordCountAsNumbers.at(wordID);
            output << address << "\t" << wordString << "\t" << wordCount << endl;
        }
    }
    output.close();
}

void CorpusUtils::writeClusterInteractionToJSON(const Corpus &corpus,
                                              const vector_word_type clusterAssignments, const string outputFile){
    const word_type numClusters = *std::max_element(clusterAssignments.begin(), clusterAssignments.end()) + 1;
    matrix_occurrences cluster2cluster(numClusters, vector_word_type(numClusters, 0));
    for (auto bigram : corpus.occurrences) {
        const word_type w1 = bigram.first.first;
        const word_type w2 = bigram.first.second;
        const word_type occ = bigram.second;
        const word_type cluster1 = clusterAssignments[w1];
        const word_type cluster2 = clusterAssignments[w2];
        cluster2cluster[cluster1][cluster2] += occ;
    }
    json json_object;
    json_object["num_clusters"] = numClusters;
    json_object["corpus_length"] = corpus.corpusLength;
    json_object["vocabulary_size"] = corpus.vocabularySize;
    json_object["cluster2cluster"] = cluster2cluster;
    cout<<"Writing clusters to file: "<<outputFile<<endl;
    ofstream out(outputFile);
    out<<std::setw(4)<<json_object;
    out.close();
}

void
CorpusUtils::writeTreeToLiangFile(const string &outputFileName, const vector<pair<string, word_type>> &treeContents,
                                  const Corpus &corpus, const vector<vector<word_type>> &clusterContent) {
    ofstream output(outputFileName);
    if (!output.is_open()) {
        throw runtime_error("File " + outputFileName + " could not be opened!");
    }
    for (auto const &item : treeContents) {
        const string &address = item.first;
        const word_type &clusterID = item.second;
        word_type clusterCount = 0;
        stringstream ss;
        for (const word_type &wordID : clusterContent[clusterID]) {
            const auto wordOptional = corpus.getWord(wordID);
            if (wordOptional) {
                const string &wordString = wordOptional.value();
                ss << wordString << " ";
                clusterCount += corpus.wordCountAsNumbers.at(wordID);
            }
        }
        output << address << "\t" << ss.str() << "\t" << clusterCount << endl;
    }
    output.close();
}

void CorpusUtils::writeCorpusVocabularyToFile(const Corpus &corpus, const string &fileNameOutput) {
    ofstream output(fileNameOutput);
    if (!output.is_open()) {
        throw runtime_error("File " + fileNameOutput + " could not be opened!");
    }
    for (word_type wordID = 0; wordID < corpus.vocabularySize; wordID++) {
        const auto wordOptional = corpus.getWord(wordID);
        if (wordOptional) {
            const string &wordString = wordOptional.value();
            const auto frequency = corpus.pl[wordID];
            output << wordString << " " << frequency << endl;
        }
    }
}

unordered_map<string, word_type>
CorpusUtils::readVocabularyFromFile(const string &fileNameVocabulary) {
    ifstream file;
    file.open(fileNameVocabulary.c_str());
    if (!file.is_open()) {
        throw runtime_error("File " + fileNameVocabulary + " could not be opened!");
    }
    string word;
    double frequency;
    unordered_map<string, word_type> vocabulary;
    while (file >> word && file >> frequency) {
        vocabulary.insert({word, frequency});
    }
    return vocabulary;
}
