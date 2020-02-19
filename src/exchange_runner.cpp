#include <iostream>
#include <Utils.h>
#include <models/Corpus.h>
#include <ExchangeAlgorithm/Exchange/Exchange.h>
#include <ExchangeAlgorithm/StochasticExchange/StochasticExchange.h>
#include <json/json.hpp>
#include <chrono>
#include <omp.h>
#include "easylogging++/easylogging++.h"
#include <CLI11.hpp>

INITIALIZE_EASYLOGGINGPP

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

const string ALG_EXCHANGE = "EXCHANGE";
const string ALG_EXCHANGE_STEPS = "EXCHANGE_STEPS";
const string ALG_EXCHANGE_STOCHASTIC = "STOCHASTIC_EXCHANGE";

int main(int ac, char *av[]) {

    string inputFile;
    string outputFile;
    word_type numClusters = 0;
    word_type noIterations = 0;
    string algorithm;
    double minAMIThreshold = 0.0;
    double percentageRandom = 0.0;
    auto numThreadsToUse = omp_get_max_threads();
    CLI::App app{"Runs the Exchange algorithm and writes out the clusters and AMI values at every iteration"};
    app.set_failure_message(CLI::FailureMessage::help);
    app.add_option("--clusters", numClusters,
                   "The number of desired clusters")->set_default_val("500");
    app.add_option("--algorithm", algorithm,
                   "Which algorithm to run: EXCHANGE, EXCHANGE_STEPS, or STOCHASTIC_EXCHANGE")->set_default_val(
            ALG_EXCHANGE);
    app.add_option("--iterations", noIterations, "Number of iterations")->set_default_val("10");
    app.add_option("--minAMI", minAMIThreshold, "Minimum AMI increase per iteration")->set_default_val(
            to_string(ExchangeAlgorithm::DEFAULT_MIN_AMI_CHANGE));
    app.add_option("--randomness", percentageRandom,
                   "Percentage of times swaps should be random (only applies if algorithm is set to STOCHASTIC_EXCHANGE). Should be floating point numbers in range [0,100].")->set_default_val(
            to_string(0));
    app.add_option("--input", inputFile, "Path to input file containing a corpus object")->required()->check(
            CLI::ExistingFile);
    app.add_option("--output", outputFile,
                   string("Path for output file. Files with .txt and .json extension will be created to store the") +
                   " flat clustering and AMI progression during clustering.")->required();
    string helpMsgThreads = "The number of threads to use for clustering. If not provided, the automatically ";
    helpMsgThreads += "determined value will be used. For your current setup that is ";
    helpMsgThreads += std::to_string(numThreadsToUse);
    helpMsgThreads += ".";
    app.add_option("--threads", numThreadsToUse, helpMsgThreads)->set_default_val(to_string(numThreadsToUse));
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
    LOG(INFO) << "Will run with at most " << numThreadsToUse << " thread(s)";
    omp_set_num_threads(numThreadsToUse);
    LOG(INFO) << "Reading corpus from " << inputFile;
    const Corpus fullCorpus = Corpus::deserializeFromFile(inputFile);
    LOG(INFO) << "Corpus vocabulary size " << fullCorpus.vocabularySize
              << " and length "
              << fullCorpus.corpusLength;

    experiment_data["num_clusters"] = numClusters;
    experiment_data["num_iterations"] = noIterations;
    experiment_data["total_words"] = fullCorpus.vocabularySize;
    experiment_data["algorithm"] = algorithm;
    experiment_data["omp_num_threads"] = numThreadsToUse;

    high_resolution_clock::time_point startTime, endTime;
    vector_word_type clusterAssignments;

    const string outputFileClusters = outputFile + ".txt";
    const string outputFileData = outputFile + ".json";

    if (ALG_EXCHANGE == algorithm) {
        LOG(INFO) << "Starting Exchange for single go...";
        Exchange ea(fullCorpus);
        startTime = high_resolution_clock::now();
        clusterAssignments = ea.cluster(numClusters, noIterations, minAMIThreshold);
        endTime = high_resolution_clock::now();
        auto elapsedTimeExchange = duration_cast<milliseconds>(endTime - startTime).count();
        double amiExchange = ea.calculateAMI();
        experiment_data["ami_exchange"] = amiExchange;
        experiment_data["duration_exchange"] = elapsedTimeExchange;
        experiment_data["iterations_exchange"] = ea.getIterations();
        LOG(INFO) << "AMI for Exchange: " << amiExchange;
    } else if (ALG_EXCHANGE_STEPS == algorithm) {
        LOG(INFO) << "Starting Exchange for single steps...";
        Exchange ea(fullCorpus);
        ea.prepareClustering(numClusters);
        experiment_data["ami_progression"] = vector_word_type();
        experiment_data["ami_progression"].push_back(ea.calculateAMI());
        experiment_data["durations"] = vector_word_type();
        experiment_data["swaps"] = vector<uint32_t>();
        clusterAssignments = ea.getClusterAssignments();
        const string outputFileClustersFirstIteration = outputFile + "_0.txt";
        fullCorpus.writeClustersToFile(outputFileClustersFirstIteration, clusterAssignments, numClusters);
        LOG(INFO) << "Beginning clustering for " << numClusters << " clusters";
        for (word_type i = 1; i <= noIterations; ++i) {
            startTime = high_resolution_clock::now();
            const bool itemsMoved = ea.clusterOneIteration(minAMIThreshold);
            endTime = high_resolution_clock::now();
            const auto elapsedTime = duration_cast<milliseconds>(endTime - startTime).count();
            const double amiExchange = ea.calculateAMI();
            experiment_data["ami_progression"].push_back(amiExchange);
            experiment_data["durations"].push_back(elapsedTime);
            experiment_data["swaps"].push_back(ea.getChangesInPreviousIteration());
            experiment_data["iterations_exchange"] = i;
            LOG(INFO) << "AMI for Exchange";
            LOG(INFO) << " (iterations " << i << ", duration "
                      << elapsedTime / 1000 << " sec. ): " << amiExchange;
            if (!itemsMoved) {
                break;
            } else {
                ofstream out(outputFileData);
                out << std::setw(4) << experiment_data;
                out.close();
            }
            clusterAssignments = ea.getClusterAssignments();
            const string outputFileClustersIteration = outputFile + "_" + std::to_string(i) + ".txt";
            fullCorpus.writeClustersToFile(outputFileClustersIteration, clusterAssignments, numClusters);
        }
    } else if (ALG_EXCHANGE_STOCHASTIC == algorithm) {
        LOG(INFO) << "Starting StochasticExchange...";
        StochasticExchange ea(fullCorpus);
        LOG(INFO) << "Setting randomness to " << percentageRandom;
        ea.setRandomness(percentageRandom);
        ea.prepareClustering(numClusters);
        experiment_data["ami_progression"] = vector_word_type();
        experiment_data["ami_progression"].push_back(ea.calculateAMI());
        experiment_data["durations"] = vector_word_type();
        experiment_data["swaps"] = vector<uint32_t>();
        clusterAssignments = ea.getClusterAssignments();
        const string outputFileClustersFirstIteration = outputFile + "_0.txt";
        fullCorpus.writeClustersToFile(outputFileClustersFirstIteration, clusterAssignments, numClusters);
        LOG(INFO) << "Beginning clustering for " << numClusters << " clusters";
        for (word_type i = 1; i <= noIterations; ++i) {
            startTime = high_resolution_clock::now();
            const bool itemsMoved = ea.clusterOneIteration(minAMIThreshold);
            endTime = high_resolution_clock::now();
            const auto elapsedTime = duration_cast<milliseconds>(endTime - startTime).count();
            const double amiExchange = ea.calculateAMI();
            experiment_data["ami_progression"].push_back(amiExchange);
            experiment_data["durations"].push_back(elapsedTime);
            experiment_data["swaps"].push_back(ea.getChangesInPreviousIteration());
            experiment_data["iterations_exchange"] = i;
            experiment_data["percentage_random_swaps"] = percentageRandom;
            LOG(INFO) << "AMI for StochasticExchange";
            LOG(INFO) << " (iterations " << i << ", duration "
                      << elapsedTime / 1000 << " sec. ): " << amiExchange;
            if (!itemsMoved) {
                break;
            } else {
                ofstream out(outputFileData);
                out << std::setw(4) << experiment_data;
                out.close();
            }
            clusterAssignments = ea.getClusterAssignments();
            const string outputFileClustersIteration = outputFile + "_" + std::to_string(i) + ".txt";
            fullCorpus.writeClustersToFile(outputFileClustersIteration, clusterAssignments, numClusters);
        }
    } else {
        cerr << "No recognised algorithm selected!" << endl;
        throw runtime_error("No recognised algorithm selected!");
    }

    LOG(INFO) << "Writing clusters to file: " << outputFileClusters;
    fullCorpus.writeClustersToFile(outputFileClusters, clusterAssignments, numClusters);
    ofstream out(outputFileData);
    out << std::setw(4) << experiment_data;
    out.close();
    LOG(INFO) << "DONE";
    return 0;
}