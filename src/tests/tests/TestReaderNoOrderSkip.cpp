#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <readers/ReaderNoOrderSkip.h>

TEST(ReaderNoOrderSkipTest, testReadSimpleFile) {
    ReaderNoOrderSkip reader;
    const Corpus c = reader.readFile("tests/test_data/skip_gram_1.txt", 2);

    EXPECT_EQ(c.vocabularySize, 4);
    EXPECT_EQ(c.corpusLength, 14);
    const word_type wordIDThe = 0;
    const word_type wordIDCat = 3;
    const word_type occ1 = c.occurrences.at({wordIDThe, wordIDCat});
    EXPECT_EQ(occ1, 1);
}

TEST(ReaderNoOrderSkipTest, testReadSimpleFileNoSkip) {
    ReaderNoOrderSkip reader;
    const Corpus c = reader.readFile("tests/test_data/skip_gram_1.txt", 1);

    EXPECT_EQ(c.vocabularySize, 4);
    EXPECT_EQ(c.corpusLength, 8);
    const word_type wordIDThe = 0;
    const word_type wordIDCat = 3;
    const word_type occ1 = c.occurrences.at({wordIDThe, wordIDCat});
    EXPECT_EQ(occ1, 1);
}

TEST(ReaderNoOrderSkipTest, testReadSimpleFileWithVocabulary) {
    ReaderNoOrderSkip reader;
    const Corpus c = reader.readFile("tests/test_data/skip_gram_1.txt",
                                     "tests/test_data/skip_gram_1.vocabulary",
                                     2);

    EXPECT_EQ(c.vocabularySize, 3);
    EXPECT_EQ(c.corpusLength, 10);
    const word_type wordIDThe = 0;
    const word_type wordIDdog = 1;
    const word_type occ1 = c.occurrences.at({wordIDThe, wordIDdog});
    EXPECT_EQ(occ1, 2);
}

TEST(ReaderNoOrderSkipTest, testReadSimpleFileWithVocabularyNoSkip) {
    ReaderNoOrderSkip reader;
    const Corpus c = reader.readFile("tests/test_data/skip_gram_1.txt",
                                     "tests/test_data/skip_gram_1.vocabulary",
                                     1);

    EXPECT_EQ(c.vocabularySize, 3);
    EXPECT_EQ(c.corpusLength, 6);
    const word_type wordIDThe = 0;
    const word_type wordIDdog = 1;
    const word_type occ1 = c.occurrences.at({wordIDThe, wordIDdog});
    EXPECT_EQ(occ1, 1);
}