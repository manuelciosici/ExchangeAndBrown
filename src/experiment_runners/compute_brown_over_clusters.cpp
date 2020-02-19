#include "easylogging++/easylogging++.h"
#include <models/Corpus.h>
#include <CorpusUtils.h>
#include <fstream>
#include <readers/ReaderClusteringIntoCorpus.h>
#include <BrownClusteringAlgorithm/BrownClusteringAlgorithm.h>
#include <CLI11.hpp>
#include <omp.h>

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int ac, char *av[]) {
    string fileNameCorpus;
    string fileNameClustering;
    string fileNameOutput;
    int numThreadsToUse = omp_get_max_threads();
    CLI::App app{"Runs Brown clustering on top of a flat clustering (that can, for example, come from Exchange"};
    app.set_failure_message(CLI::FailureMessage::help);
    app.add_option("--clustering", fileNameClustering,
                   "Path to file containing a flat clustering")->required()->check(CLI::ExistingFile);;
    app.add_option("--corpus", fileNameCorpus,
                   "Path to input file containing a corpus object for the provided clustering")->required()->check(
            CLI::ExistingFile);;
    app.add_option("--output", fileNameOutput,
                   "Path to file to write the resulting clusters to")->required();
    string helpMsgThreads = "The number of threads to use for clustering. If not provided, the automatically ";
    helpMsgThreads += "determined value will be used. For your current setup that is ";
    helpMsgThreads += std::to_string(numThreadsToUse);
    helpMsgThreads += ".";
    app.add_option("--threads", numThreadsToUse,  helpMsgThreads)->set_default_val(to_string(numThreadsToUse));
    try {
        app.parse(ac, av);
    } catch (CLI::CallForHelp &e) {
        (app).exit(e);
        return 1;
    } catch (CLI::ParseError &e) {
        (app).exit(e);
        return 1;
    }

    LOG(INFO) << "Will run with at most " << numThreadsToUse << " thread(s)";
    omp_set_num_threads(numThreadsToUse);

    ReaderClusteringIntoCorpus reader;
    LOG(INFO) << "Reading original corpus";
    const Corpus corpus = Corpus::deserializeFromFile(fileNameCorpus);
    LOG(INFO) << "Constructing clustered corpus";
    const Corpus clusteredCorpus = reader.readFile(fileNameCorpus, fileNameClustering);
    LOG(INFO) << "Reading cluster assignments";
    const vector<word_type> clusterAssignments = CorpusUtils::readClusterAssignmentsFromFile(fileNameClustering,
                                                                                             corpus);

    const word_type numberOfClusters = *std::max_element(clusterAssignments.begin(), clusterAssignments.end()) + 1;

    vector<vector<word_type>> clusterContent(numberOfClusters, vector<word_type>());
    for (word_type i = 0; i < clusterAssignments.size(); ++i) {
        const auto clusterID = clusterAssignments[i];
        clusterContent[clusterID].push_back(i);
    }

    BrownClusteringAlgorithm brown(clusteredCorpus);
    LOG(INFO) << "Clustering...";
    brown.cluster(numberOfClusters, numberOfClusters);
    LOG(INFO) << "\twriting tree to file " << fileNameOutput;
    const string outputFileTree = fileNameOutput + ".tree";
    CorpusUtils::writeTreeToLiangFile(outputFileTree, brown.getClusterTree()->getTreeAsBitAddressFormat(), corpus,
                                      clusterContent);
    const string outputFileMerges = fileNameOutput + ".merges";
    LOG(INFO) << "Writing merge log to file " << outputFileMerges;
    ofstream output(outputFileMerges);
    if (!output.is_open()) {
        throw runtime_error("File " + outputFileMerges + " could not be opened!");
    }
    brown.getClusterTree()->printMerges(output, clusteredCorpus);
    LOG(INFO) << "DONE";
    return 0;
}
