#include <gtest/gtest.h>
#include <models/Corpus.h>
#include <gmock/gmock.h>
#include <fstream>
#include "CorpusUtils.h"

TEST(CorpusUtilsTest, testCalculateClusterFrequency) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.wordCountAsNumbers.insert({0, 1});
    corpus.wordCountAsNumbers.insert({1, 2});
    corpus.wordCountAsNumbers.insert({2, 3});
    corpus.wordCountAsNumbers.insert({3, 4});
    corpus.wordCountAsNumbers.insert({4, 5});

    const vector_word_type clusterAssignments = {0, 1, 2, 0, 1};
    const vector_word_type expectedFrequencies = {5, 7, 3};
    const vector_word_type actualFrequencies = CorpusUtils::calculateClusterFrequency(clusterAssignments, corpus);
    EXPECT_THAT(actualFrequencies, ::testing::ContainerEq(expectedFrequencies));
}

TEST(CorpusUtilsTest, testReadClusterAssignmentsFromFile) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.idsToWords = {{0, "a"}, {1, "b"}, {2, "c"}, {3, "d"}, {4, "e"}};
    const vector_word_type expectedClusterAssignments = {0, 1, 2, 0, 1};
    const vector_word_type actualClusterAssignments =
            CorpusUtils::readClusterAssignmentsFromFile("tests/test_data/cluster_assignments.test", corpus);
    EXPECT_THAT(actualClusterAssignments, ::testing::ContainerEq(expectedClusterAssignments));

}

TEST(CorpusUtilsTest, testReadClusterAssignmentsFromFileWhenNoFile) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.idsToWords.insert({0, "a"});
    corpus.idsToWords.insert({1, "b"});
    corpus.idsToWords.insert({2, "c"});
    corpus.idsToWords.insert({3, "d"});
    corpus.idsToWords.insert({4, "e"});
    EXPECT_THROW(CorpusUtils::readClusterAssignmentsFromFile("/tmp/non/nonexistent_file", corpus),
                 runtime_error);
}

TEST(CorpusUtilsTest, testReadVocabularyFromFileWhenNoFile) {
    EXPECT_THROW(CorpusUtils::readVocabularyFromFile("/tmp/non/nonexistent_file"), runtime_error);
}

TEST(CorpusUtilsTest, testWriteClustersToFile) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.idsToWords.insert({0, "a"});
    corpus.idsToWords.insert({1, "b"});
    corpus.idsToWords.insert({2, "c"});
    corpus.idsToWords.insert({3, "d"});
    corpus.idsToWords.insert({4, "e"});

    corpus.wordCountAsNumbers.insert({0, 1});
    corpus.wordCountAsNumbers.insert({1, 2});
    corpus.wordCountAsNumbers.insert({2, 3});
    corpus.wordCountAsNumbers.insert({3, 4});
    corpus.wordCountAsNumbers.insert({4, 5});

    corpus.pl.push_back(1);
    corpus.pl.push_back(2);
    corpus.pl.push_back(3);
    corpus.pl.push_back(4);
    corpus.pl.push_back(5);
    corpus.pl.push_back(6);

    const vector_word_type clusterAssignments = {0, 1, 2, 0, 1};
    std::string tmpFileName = "/tmp/unit_test_corpus_util_write_clusters_to_file";
    corpus.writeClustersToFile(tmpFileName, clusterAssignments, 3);
    const string expectedContents = "e, b, \nd, a, \nc, \n";
    std::ifstream t(tmpFileName);
    const std::string actualContent((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    EXPECT_TRUE(expectedContents.compare(actualContent) == 0)
        << "File content not as expected. Expected:\n" << expectedContents << "\n========== But Found: \n" <<
                actualContent << endl << "==========";
}

TEST(CorpusUtilsTest, testWriteTreeToLiangFile) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.idsToWords.insert({0, "a"});
    corpus.idsToWords.insert({1, "b"});
    corpus.idsToWords.insert({2, "c"});
    corpus.idsToWords.insert({3, "d"});
    corpus.idsToWords.insert({4, "e"});

    corpus.wordCountAsNumbers.insert({0, 1});
    corpus.wordCountAsNumbers.insert({1, 2});
    corpus.wordCountAsNumbers.insert({2, 3});
    corpus.wordCountAsNumbers.insert({3, 4});
    corpus.wordCountAsNumbers.insert({4, 5});

    const vector<pair<string, word_type>> treeContents = {
        {"0", 0},
        {"100", 1},
        {"1010", 2},
        {"10110", 3},
        {"10111", 4},
    };
    const std::string tmpFileName = "/tmp/unit_test_corpus_util_write_tree_to_liang_file";
    CorpusUtils::writeTreeToLiangFile(tmpFileName, treeContents, corpus);
    const string expectedContents =
            string("0\ta\t1\n") +
            "100\tb\t2\n" +
            "1010\tc\t3\n" +
            "10110\td\t4\n" +
            "10111\te\t5\n";
    std::ifstream t(tmpFileName);
    const std::string actualContent((std::istreambuf_iterator<char>(t)),
                                    std::istreambuf_iterator<char>());
    EXPECT_TRUE(expectedContents.compare(actualContent) == 0)
    << "File content not as expected. Expected:\n" << expectedContents << "\n========== But Found: \n" <<
    actualContent << endl << "==========";
}

TEST(CorpusUtilsTest, testCannotWriteTreeToFileWhenNoFile) {
    Corpus corpus;
    corpus.vocabularySize = 5;
    corpus.idsToWords.insert({0, "a"});
    corpus.idsToWords.insert({1, "b"});
    corpus.idsToWords.insert({2, "c"});
    corpus.idsToWords.insert({3, "d"});
    corpus.idsToWords.insert({4, "e"});

    corpus.wordCountAsNumbers.insert({0, 1});
    corpus.wordCountAsNumbers.insert({1, 2});
    corpus.wordCountAsNumbers.insert({2, 3});
    corpus.wordCountAsNumbers.insert({3, 4});
    corpus.wordCountAsNumbers.insert({4, 5});

    const vector<pair<string, word_type>> treeContents = {
            {"0", 0},
            {"100", 1},
            {"1010", 2},
            {"10110", 3},
            {"10111", 4},
    };

    EXPECT_THROW(CorpusUtils::writeTreeToLiangFile("/tmp/non/nonexistent_file", treeContents, corpus),
                 runtime_error);
}