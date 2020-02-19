#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <models/Corpus.h>
#include "ExchangeAlgorithm/Exchange/Exchange.h"
#include "ExchangeAlgorithm/StochasticExchange/StochasticExchange.h"
#include "ExchangeAlgorithm/ExchangeAlgorithm.h"
#include "readers/ReaderNoOrder.h"
#include "readers/ReaderFrequency.h"

Corpus createSimpleCorpus() {
    //    the corpus here is a a b c d c
    Corpus abcdCorpus;
    abcdCorpus.vocabularySize = 4;
    abcdCorpus.corpusLength = 6;
    abcdCorpus.pl = {2.0 / 5.0, 1.0 / 5.0, 1.0 / 5.0, 1.0 / 5.0};
    abcdCorpus.pr = {1.0 / 5.0, 1.0 / 5.0, 2.0 / 5.0, 1.0 / 5.0};
    abcdCorpus.idsToWords.insert({0, "a"});
    abcdCorpus.idsToWords.insert({1, "b"});
    abcdCorpus.idsToWords.insert({2, "c"});
    abcdCorpus.idsToWords.insert({3, "d"});
    abcdCorpus.occurrences.insert({{0, 0}, 1});
    abcdCorpus.occurrences.insert({{0, 1}, 1});
    abcdCorpus.occurrences.insert({{1, 2}, 1});
    abcdCorpus.occurrences.insert({{2, 3}, 1});
    abcdCorpus.occurrences.insert({{3, 2}, 1});
    abcdCorpus.wordCountAsNumbers.insert({0, 2});
    abcdCorpus.wordCountAsNumbers.insert({1, 1});
    abcdCorpus.wordCountAsNumbers.insert({2, 2});
    abcdCorpus.wordCountAsNumbers.insert({3, 1});

    return abcdCorpus;
}

TEST(ExchangeTest, testAMIExchangeAlgorithms) {
//    the corpus here is a b a c d
    const Corpus abcdCorpus = createSimpleCorpus();
    const vector<vector<word_type>> clusterAssignments = {
            {0, 1, 1, 2},
            {0, 1, 2, 2},
            {2, 1, 0, 2},
            {1, 1, 2, 0},
            {2, 0, 2, 1},
    };

    for (int index = 0; index < clusterAssignments.size(); ++index) {
        vector<double> amis;
        Exchange omp(abcdCorpus);
        StochasticExchange stochasticExchange(abcdCorpus);
        const vector<Exchange *> algorithms = {&omp, &stochasticExchange};
        for (Exchange *e : algorithms) {
            e->cluster(3, 0, clusterAssignments[index]);
            amis.push_back(e->calculateAMI());
        }
        for (int i = 0; i < amis.size(); ++i) {
            const string alg1 = algorithms[i]->getName();
            for (int j = i + 1; j < amis.size(); ++j) {
                const string alg2 = algorithms[j]->getName();
                EXPECT_NEAR(amis[i], amis[j], 0.001) << "Algorithms " << alg1 << "(" << amis[i]
                                                     << ") and " << alg2 << " do not agree when"
                                                     << " run for cluster assignment index " << index;
            }
        }
    }
}

TEST(ExchangeTest, testCalculateAMI) {
//    the corpus here is a b a c d
    const Corpus abcdCorpus = createSimpleCorpus();

    const vector_word_type initialClusterAssignments = {0, 1, 1, 2};

    Exchange omp(abcdCorpus);
    omp.prepareClustering(3, initialClusterAssignments);

    const double expectedAMI = 0.570950594454669;
    const double actualAMI = omp.calculateAMI();
    EXPECT_NEAR(expectedAMI, actualAMI, 0.001);
}

TEST(ExchangeTest, test2ExchangeAlgorithms) {
//    the corpus here is a b a c d
    const Corpus abcdCorpus = createSimpleCorpus();

    const vector_word_type initialClusterAssignments = {0, 1, 1, 2};
    const vector_word_type expectedClusterAssignments = {0, 1, 2, 1};

    Exchange omp(abcdCorpus);

    const auto actualClusterAssignments = omp.cluster(3, 1, initialClusterAssignments);
    EXPECT_THAT(actualClusterAssignments, ::testing::ContainerEq(expectedClusterAssignments))
                        << "Failed on " << omp.getName();

    const auto actualClusterAssignments2 = omp.getClusterAssignments();
    EXPECT_THAT(actualClusterAssignments2, ::testing::ContainerEq(expectedClusterAssignments))
                        << "Failed on " << omp.getName();

    const double actualAMI = omp.calculateAMI();
    const double expectedAMI = 1.12192809488736;
    EXPECT_NEAR(expectedAMI, actualAMI, 0.001);
}

TEST(ExchangeTest, testSortClusterAssignments) {
    const word_type noClusters = 5;
    const vector_word_type input = {2, 1, 3, 0, 1, 4, 0, 3, 2, 4};
    const vector_word_type expected = {0, 1, 2, 3, 1, 4, 3, 2, 0, 4};

    const vector_word_type actual = ExchangeAlgorithm::sortClusterAssignments(input, noClusters);
    EXPECT_THAT(actual, ::testing::ContainerEq(expected));
}

TEST(ExchangeTest, testFileCorpora) {
    const string path = "tests/test_data/";
    const vector<pair<string, word_type>> filesToRunOn = {{"alice_long_tokenized.txt", 10},
                                                          {"debug_dog.txt",            3}};
    ReaderNoOrder readerNoOrder;
    ReaderFrequency readerFrequency;
    for (auto pair : filesToRunOn) {
        const string fileName = pair.first;
        const word_type noClusters = pair.second;
        const Corpus corpusNoOrder = readerNoOrder.readFile(path + fileName);
        const Corpus corpus = readerFrequency.reorderCorpus(corpusNoOrder);
        Exchange omp(corpus);
        const vector<Exchange *> algorithms = {&omp};


        vector<double> amisBeginning;
        for (Exchange *e : algorithms) {
            e->cluster(noClusters, 0);
            amisBeginning.push_back(e->calculateAMI());
        }
        for (int i = 0; i < amisBeginning.size(); ++i) {
            const string alg1 = algorithms[i]->getName();
            for (int j = i + 1; j < amisBeginning.size(); ++j) {
                const string alg2 = algorithms[j]->getName();
                EXPECT_NEAR(amisBeginning[i], amisBeginning[j], 0.001)
                                    << "Algorithms " << alg2 << "(" << amisBeginning[j]
                                    << ") and " << alg1 << " do not agree";
            }
        }

        vector<vector_word_type> results = vector<vector_word_type>(algorithms.size());
        for (word_type i = 0; i < algorithms.size(); ++i) {
            Exchange *e = algorithms[i];
            results[i] = e->cluster(noClusters, 1);
        }
        for (word_type i = 0; i < algorithms.size(); ++i) {
            const vector_word_type results_i = results[i];
            const string algorithm_i = algorithms[i]->getName();
            for (word_type j = i + 1; j < algorithms.size(); ++j) {
                const string algorithm_j = algorithms[j]->getName();
                const vector_word_type results_j = results[j];
                if (i != j) {
                    EXPECT_THAT(results_i, ::testing::ContainerEq(results_j))
                                        << "Failed on file " << fileName << " when comparing " << algorithm_j
                                        << " and " << algorithm_i;
                }
            }
        }
        vector<double> amis;
        for (ExchangeAlgorithm *e : algorithms) {
            amis.push_back(e->calculateAMI());
        }
        for (int i = 0; i < amis.size(); ++i) {
            const string alg1 = algorithms[i]->getName();
            for (int j = i + 1; j < amis.size(); ++j) {
                const string alg2 = algorithms[j]->getName();
                EXPECT_NEAR(amis[i], amis[j], 0.001) << "Algorithms " << alg1 << " and " << alg2 << " do not agree";
            }
        }
    }
}