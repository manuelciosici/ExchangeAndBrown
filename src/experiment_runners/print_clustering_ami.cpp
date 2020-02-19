#include <iostream>
#include <Utils.h>
#include <models/Corpus.h>
#include <ExchangeAlgorithm/Exchange/Exchange.h>
#include "easylogging++/easylogging++.h"
#include <CLI11.hpp>
#include <CorpusUtils.h>

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int ac, char *av[]) {

    string inputClusters;
    string inputCorpus;
    string outputFile;
    CLI::App app{"Prints the AMI of a given flat clustering"};
    app.set_failure_message(CLI::FailureMessage::help);
    app.add_option("--corpus", inputCorpus, "Path to input file containing a corpus object")->required()->check(
            CLI::ExistingFile);
    app.add_option("--input_clusters", inputClusters, "Path to file containing flat clusters")->required()->check(
            CLI::ExistingFile);
    try {
        app.parse(ac, av);
        LOG(INFO) << "Will read clusters from " << inputClusters;
        LOG(INFO) << "Will read corpus from " << inputCorpus;
        const Corpus corpus = Corpus::deserializeFromFile(inputCorpus);
        LOG(INFO) << "Loaded corpus with " << corpus.vocabularySize << " types in vocabulary";
        Exchange exchange(corpus);
        const auto clustering = CorpusUtils::readClusterAssignmentsFromFile(inputClusters, corpus);
        const word_type numClusters = *std::max_element(clustering.begin(), clustering.end()) + 1;
        LOG(INFO) << "Loaded cluster assignments for " << clustering.size() << " types into " << numClusters
                  << " clusters";
        exchange.initializeDataStructures(numClusters, clustering);
        const double ami = exchange.calculateAMI();
        LOG(INFO) << "AMI of clustering is " << ami;
        cout.precision(std::numeric_limits<double>::max_digits10);
        cout << "AMI of clustering is " << ami << endl;
    } catch (CLI::CallForHelp &e) {
        (app).exit(e);
        return 1;
    } catch (CLI::ParseError &e) {
        (app).exit(e);
        return 1;
    }
    LOG(INFO) << "DONE";
    return 0;
}