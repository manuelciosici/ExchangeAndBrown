#include <iostream>
#include "BrownClusteringAlgorithm/BrownClusteringAlgorithm.h"
#include "models/Corpus.h"
#include "readers/ReaderNoOrder.h"
#include <fstream>
#include <CorpusUtils.h>
#include <omp.h>
#include <readers/ReaderThreshold.h>
#include "readers/ReaderNoOrder.h"
#include "readers/ReaderNoOrderSkip.h"
#include "readers/ReaderFrequency.h"
#include "easylogging++/easylogging++.h"
#include <CLI11.hpp>

INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int ac, char *av[]) {
    string operation;
    string inputFile;
    string inputFileVocabulary;
    string outputFile;
    string outputFileVocabulary;
    word_type threshold;
    word_type numClusters;
    word_type windowSize;
    bool filterStrict;
    int numThreadsToUse = omp_get_max_threads();
    CLI::App app{
            "A simple way to run Brown clustering that does corpusNoOrder reading using bi-grams, sorting by frequency, and clustering in one go"};
    app.set_failure_message(CLI::FailureMessage::help);

    app.add_option("--input", inputFile, "Path to input text file")->required()->check(CLI::ExistingFile);

    app.add_option("--output", outputFile,
                   "Path to file to write the resulting clusters to")->required();
    app.add_option("--window", windowSize,
                   "Size of the window to use when learning. Must be at least one larger than the number of clusters")->required();
    app.add_option("--numClusters", numClusters, "Number of clusters to generate.")->set_default_val(
            "500");

    app.add_option("--threshold", threshold,
                   "Minimum frequency. Words with frequency below this will be removed.")->set_default_val("1");
    app.add_flag("--strict", filterStrict,
                 "Whether to use strict filtering. See Corpus ReaderThreshold::reorderCorpus for details.")->set_default_val(
            "false");
    string helpMsgThreads = "The number of threads to use for clustering. If not provided, the automatically ";
    helpMsgThreads += "determined value will be used. For your current setup that is ";
    helpMsgThreads += std::to_string(numThreadsToUse);
    helpMsgThreads += ".";
    app.add_option("--threads", numThreadsToUse, helpMsgThreads)->set_default_val(to_string(numThreadsToUse));

    try {
        app.parse(ac, av);
    } catch (CLI::CallForHelp e) {
        (app).exit(e);
        return 1;
    } catch (CLI::ParseError e) {
        (app).exit(e);
        return 1;
    }
    LOG(INFO) << "Will run with at most " << numThreadsToUse << " thread(s)";
    omp_set_num_threads(numThreadsToUse);

    Corpus finalCorpus;
    ReaderNoOrder readerNoOrder;
    LOG(INFO) << "Reading corpusNoOrder from text file " << inputFile;
    const Corpus corpusNoOrder = readerNoOrder.readFile(inputFile);
    LOG(INFO) << "Corpus size: " << corpusNoOrder.corpusLength
              << " vocabulary size: "
              << corpusNoOrder.vocabularySize;

    LOG(INFO) << "Will reorder corpus from using order FREQUENCY";
    ReaderFrequency readerOrder;
    const Corpus corpusFrequency = readerOrder.reorderCorpus(corpusNoOrder);

    LOG(INFO) << "Will filter corpus from file " << inputFile << " using threshold " << threshold;

    if (threshold > 1) {
        ReaderThreshold readerThreshold;
        const Corpus corpusReorderedFiltered = readerThreshold.reorderCorpus(corpusFrequency, threshold, filterStrict);
        LOG(INFO) << "Corpus size: " << corpusReorderedFiltered.corpusLength
                  << " vocabulary size: " << corpusReorderedFiltered.vocabularySize;
        finalCorpus = corpusReorderedFiltered;
    } else {
        finalCorpus = corpusFrequency;
    }
    LOG(INFO) << "Inducing Brown clustering";
    LOG(INFO) << "Reading corpus from file " << inputFile;

    vector_word_type clusterAssignments;
    LOG(INFO) << "Initializing and running BROWN for " << numClusters
              << " clusters with a window of " << windowSize;
    unique_ptr<BrownClusteringAlgorithm> brown(new BrownClusteringAlgorithm(finalCorpus));
    clusterAssignments = brown->cluster(numClusters, windowSize);
    LOG(INFO) << "Clustering finished. Writing clusters to file " << outputFile;
    finalCorpus.writeClustersToFile(outputFile, clusterAssignments, numClusters);
    LOG(INFO) << "writing tree to file " << outputFile;
    const string outputFileTree = outputFile + ".tree";
    CorpusUtils::writeTreeToLiangFile(outputFileTree,
                                      brown->getClusterTree()->getTreeAsBitAddressFormat(),
                                      finalCorpus);
    const string outputFileMerges = outputFile + ".merges";
    LOG(INFO) << "Writing merge log to file " << outputFileMerges;
    ofstream output(outputFileMerges);
    if (!output.is_open()) {
        throw runtime_error("File " + outputFileMerges + " could not be opened!");
    }
    brown->getClusterTree()->printMerges(output, finalCorpus);
    LOG(INFO) << "DONE";
    return 0;
}
