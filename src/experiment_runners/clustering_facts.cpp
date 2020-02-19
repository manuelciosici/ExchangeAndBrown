#include "easylogging++/easylogging++.h"
#include <iostream>
#include <Utils.h>
#include <models/Corpus.h>
#include <fstream>
#include <json/json.hpp>
#include <CorpusUtils.h>
#include <CLI11.hpp>

INITIALIZE_EASYLOGGINGPP

using namespace std;
using json = nlohmann::json;

int main(int ac, char *av[]) {
    string inputFileCorpus;
    string inputFileClustering;
    string outputFile;

    CLI::App app{"Outputs some basic information about a given flat clustering"};
    app.set_failure_message(CLI::FailureMessage::help);
    app.add_option("--input_clustering", inputFileClustering,
                   "Path to file containing a flat clustering")->required()->check(CLI::ExistingFile);;
    app.add_option("--input_corpus", inputFileCorpus,
                   "Path to input file containing a corpus object for the provided clustering")->required()->check(CLI::ExistingFile);;
    app.add_option("--output", outputFile,
                   "file to write output to")->required();
    try {
        app.parse(ac, av);
    } catch (CLI::CallForHelp &e) {
        (app).exit(e);
        return 1;
    } catch (CLI::ParseError &e) {
        (app).exit(e);
        return 1;
    }

    json experiment_data;

    experiment_data["file"]["corpus"] = inputFileCorpus;
    experiment_data["file"]["clustering"] = inputFileClustering;
    LOG(INFO) << "Reading corpus from " << inputFileCorpus;
    const Corpus corpus = Corpus::deserializeFromFile(inputFileCorpus);
    LOG(INFO) << "Corpus vocabulary size " << corpus.vocabularySize
              << " and length "
              << corpus.corpusLength;

    experiment_data["corpus"]["vocabulary_size"] = corpus.vocabularySize;
    experiment_data["corpus"]["corpus_length"] = corpus.corpusLength;
    const vector_word_type clusterAssignments = CorpusUtils::readClusterAssignmentsFromFile(
            inputFileClustering, corpus);

    const word_type numClusters =
            *std::max_element(clusterAssignments.begin(), clusterAssignments.end()) + 1;
    experiment_data["clustering"]["number_clusters"] = numClusters;
    vector<word_type> clusterSizes(numClusters, 0);
    for (uint32_t wordID = 0; wordID < corpus.vocabularySize; wordID++) {
        const auto clusterID = clusterAssignments[wordID];
        clusterSizes[clusterID] += corpus.wordCountAsNumbers.at(wordID);
    }
    experiment_data["clustering"]["cluster_frequencies"] = clusterSizes;
    LOG(INFO) << "Writing output to file " << outputFile;
    ofstream out(outputFile);
    out << std::setw(4) << experiment_data;
    out.close();
    LOG(INFO) << "DONE";
    return 0;
}