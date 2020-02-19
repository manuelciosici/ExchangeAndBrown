#include <iostream>
#include "BrownClusteringAlgorithm/BrownClusteringAlgorithm.h"
#include "models/Corpus.h"
#include "readers/ReaderNoOrder.h"
#include <fstream>
#include <ExchangeAlgorithm/Exchange/Exchange.h>
#include <CorpusUtils.h>
#include <omp.h>
#include <readers/ReaderThreshold.h>
#include "readers/ReaderNoOrder.h"
#include "readers/ReaderNoOrderSkip.h"
#include "readers/ReaderFrequency.h"
#include "ExchangeAlgorithm/ExchangeAlgorithm.h"
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
    CLI::App app{"Main binary. Can turn text into Corpus objects, filter them and run the Brown algorithm"};
    app.set_failure_message(CLI::FailureMessage::help);
//    we need exactly one sub-command like read, filter, etc
    app.require_subcommand(1);

    auto sub_read = app.add_subcommand("read", "Read a text file into a corpus object");
    sub_read->add_option("--input", inputFile, "Path to input text file")->required()->check(CLI::ExistingFile);
    sub_read->add_option("--output", outputFile,
                         "Path to file to write the resulting corpus to")->required();

    auto sub_read_skip = app.add_subcommand("read-skip", "Read a text file into a corpus object using skip-grams");
    sub_read_skip->add_option("--input", inputFile, "Path to input text file")->required()->check(CLI::ExistingFile);
    sub_read_skip->add_option("--output", outputFile,
                              "Path to file to write the resulting corpus to")->required();
    sub_read_skip->add_option("--vocabulary", inputFileVocabulary, "Use vocabulary from this file")->check(
            CLI::ExistingFile);

    auto sub_reorder_freq = app.add_subcommand("reorder",
                                               "Reorder the given corpus for that high frequency words have lower IDs.");
    sub_reorder_freq->add_option("--input", inputFile,
                                 "Path to input file containing a corpus object")->required()->check(CLI::ExistingFile);
    sub_reorder_freq->add_option("--output", outputFile,
                                 "Path to file to write the resulting corpus object to")->required();

    auto sub_filter_freq = app.add_subcommand("filter",
                                              "Filter the given corpus for that words with frequency below the threshold are removed.");
    sub_filter_freq->add_option("--input", inputFile,
                                "Path to input file containing a corpus object")->required()->check(CLI::ExistingFile);
    sub_filter_freq->add_option("--output", outputFile,
                                "Path to file to write the resulting corpus object to")->required();
    sub_filter_freq->add_option("--threshold", threshold,
                                "Minimum frequency. Words with frequency below this will be removed.");
    sub_filter_freq->add_flag("--strict", filterStrict,
                              "Whether to use strict filtering. See Corpus ReaderThreshold::reorderCorpus for details.");
    sub_filter_freq->add_option("--output-vocabulary", outputFileVocabulary,
                                "Path to file to write the resulting vocabulary to");

    auto sub_learn_brown = app.add_subcommand("induce_brown",
                                              "Induce a clustering using the Brown algorithm");
    sub_learn_brown->add_option("--input", inputFile,
                                "Path to input file containing a corpus object")->required()->check(CLI::ExistingFile);
    sub_learn_brown->add_option("--output", outputFile,
                                "Path to file to write the resulting clusters to")->required();
    sub_learn_brown->add_option("--window", windowSize,
                                "Size of the window to use when learning. Must be at least one larger than the number of clusters")->required();
    sub_learn_brown->add_option("--numClusters", numClusters, "Number of clusters to generate.")->set_default_val(
            "500");
    string helpMsgThreads = "The number of threads to use for clustering. If not provided, the automatically ";
    helpMsgThreads += "determined value will be used. For your current setup that is ";
    helpMsgThreads += std::to_string(numThreadsToUse);
    helpMsgThreads += ".";
    sub_learn_brown->add_option("--threads", numThreadsToUse, helpMsgThreads)->set_default_val(to_string(numThreadsToUse));

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

    if (app.got_subcommand(sub_read_skip)) {
        LOG(INFO) << "Size of input vocabulary " << inputFileVocabulary.size();
        ReaderNoOrderSkip reader;
        Corpus corpus;
        LOG(INFO) << "Reading SKIP-GRAM corpus from file " << inputFile;
        if (!inputFileVocabulary.empty()) {
            LOG(INFO) << "Reading using vocabulary from file " << inputFileVocabulary;
            corpus = reader.readFile(inputFile, inputFileVocabulary, 5);
        } else {
            corpus = reader.readFile(inputFile, 5);
        }
        Corpus::serializeToFile(corpus, outputFile);
        LOG(INFO) << "Corpus size: " << corpus.corpusLength << " vocabulary size: " << corpus.vocabularySize;
    } else if (app.got_subcommand(sub_read)) {
        ReaderNoOrder reader;
        LOG(INFO) << "Reading corpus from text file " << inputFile;
        const Corpus corpus = reader.readFile(inputFile);
        LOG(INFO) << "Corpus size: " << corpus.corpusLength
                  << " vocabulary size: "
                  << corpus.vocabularySize;
        LOG(INFO) << "Writing corpus object to file " << outputFile;
        Corpus::serializeToFile(corpus, outputFile);
    } else if (app.got_subcommand(sub_reorder_freq)) {
        LOG(INFO) << "Will reorder corpus from file " << inputFile;
        LOG(INFO) << "\t using order FREQUENCY";
        ReaderFrequency readerOrder;
        const Corpus corpus = Corpus::deserializeFromFile(inputFile);
        const Corpus reorderedCorpus = readerOrder.reorderCorpus(corpus);
        Corpus::serializeToFile(reorderedCorpus, outputFile);
    } else if (app.got_subcommand(sub_filter_freq)) {
        LOG(INFO) << "Will filter corpus from file " << inputFile << " using threshold " << threshold;
        const Corpus corpus = Corpus::deserializeFromFile(inputFile);
        ReaderThreshold reader;
        const Corpus filteredCorpus = reader.reorderCorpus(corpus, threshold, filterStrict);
        LOG(INFO) << "Corpus size: " << filteredCorpus.corpusLength
                  << " vocabulary size: " << filteredCorpus.vocabularySize;
        LOG (INFO) << "Writing corpus to file " << outputFile;
        Corpus::serializeToFile(filteredCorpus, outputFile);
        LOG (INFO) << "Writing vocabulary to file " << outputFileVocabulary;
        CorpusUtils::writeCorpusVocabularyToFile(filteredCorpus, outputFileVocabulary);
    } else if (app.got_subcommand(sub_learn_brown)) {
        LOG(INFO) << "Inducing Brown clustering";
        LOG(INFO) << "Reading corpus from file " << inputFile;
        const Corpus corpus = Corpus::deserializeFromFile(inputFile);
        vector_word_type clusterAssignments;
        LOG(INFO) << "Initializing and running BROWN for " << numClusters
                  << " clusters with a window of " << windowSize;
        unique_ptr<BrownClusteringAlgorithm> brown = make_unique<BrownClusteringAlgorithm>(corpus);
        try {
            clusterAssignments = brown->cluster(numClusters, windowSize);
        } catch( const std::exception & ex ) {
            cerr << ex.what() << endl;
        }
        LOG(INFO) << "Clustering finished. Writing clusters to file " << outputFile;
        corpus.writeClustersToFile(outputFile, clusterAssignments, numClusters);
        const string outputFileTree = outputFile + ".tree";
        LOG(INFO) << "Writing tree to file " << outputFileTree;
        CorpusUtils::writeTreeToLiangFile(outputFileTree,
                                          brown->getClusterTree()->getTreeAsBitAddressFormat(),
                                          corpus);
        const string outputFileMerges = outputFile + ".merges";
        LOG(INFO) << "Writing merge log to file " << outputFileMerges;
        ofstream output(outputFileMerges);
        if (!output.is_open()) {
            throw runtime_error("File " + outputFileMerges + " could not be opened!");
        }
        brown->getClusterTree()->printMerges(output, corpus);
    }
    LOG(INFO) << "DONE";
    return 0;
}
