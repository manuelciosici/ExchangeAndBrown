#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <models/Corpus.h>
#include <Utils.h>
#include <gmock/gmock.h>

TEST(CorpusTest, testrightiesandlefties) {
    Corpus testCorpus;
    const word_type vocabularySize = 3;
    testCorpus.vocabularySize = vocabularySize;
//    testCorpus.occurrences = matrix_occurrences(5);
    testCorpus.occurrences[{0, 0}] = 1;
    testCorpus.occurrences[{0, 1}] = 1;
    testCorpus.occurrences[{0, 2}] = 1;

    testCorpus.occurrences[{2, 0}] = 1;
    testCorpus.occurrences[{2, 2}] = 1;

    matrix_occurrences expectedToTheLeftOfWord = matrix_occurrences(3);
    expectedToTheLeftOfWord[0] = {0, 2};
    expectedToTheLeftOfWord[1] = {0};
    expectedToTheLeftOfWord[2] = {0, 2};

    matrix_occurrences expectedToTheRightOfWord = matrix_occurrences(3);
    expectedToTheRightOfWord[0] = {0, 1, 2};
    expectedToTheRightOfWord[1] = {};
    expectedToTheRightOfWord[2] = {0, 2};

    matrix_occurrences actualToTheLeftOfWord;
    matrix_occurrences actualToTheRightOfWord;
    std::tie(actualToTheLeftOfWord, actualToTheRightOfWord) = Corpus::computeLeftiesAndRighties(testCorpus);

    EXPECT_EQ(expectedToTheLeftOfWord.size(), actualToTheLeftOfWord.size());
    EXPECT_EQ(expectedToTheRightOfWord.size(), actualToTheRightOfWord.size());
    for (word_type i = 0; i < vocabularySize; ++i) {
        EXPECT_THAT(actualToTheLeftOfWord[i], ::testing::ContainerEq(expectedToTheLeftOfWord[i]));
        EXPECT_THAT(expectedToTheRightOfWord[i], ::testing::ContainerEq(expectedToTheRightOfWord[i]));
    }
}

Corpus createSimpleCorpus2() {
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

TEST(CorpusTest, testWriteAndLoadCorpus) {
    Corpus testCorpus = createSimpleCorpus2();

    const string pathForCorpus = "/tmp/corpus.test";

    Corpus::serializeToFile(testCorpus, pathForCorpus);
    Corpus newCorpus = Corpus::deserializeFromFile(pathForCorpus);
    EXPECT_TRUE(testCorpus == newCorpus);
}

TEST(CorpusTest, testGetWordOutOfRange) {
    Corpus testCorpus = createSimpleCorpus2();

    const auto word = testCorpus.getWord(666);
    EXPECT_TRUE(word == std::nullopt);
}

TEST(CorpusTest, testCannotReadFromFileWhenNoFile) {

    EXPECT_THROW(Corpus newCorpus = Corpus::deserializeFromFile("/tmp/non/nonexistent_file"),
                 runtime_error);
}