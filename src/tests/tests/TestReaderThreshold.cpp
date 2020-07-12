#include <models/Corpus.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <readers/ReaderThreshold.h>
#include <readers/ReaderNoOrder.h>

TEST(ReaderThresholdTest, testReaderThresholdSimpleCorpusStrict) {
    ReaderNoOrder reader;
    const Corpus c = reader.readFile("tests/test_data/abcd.txt");

    EXPECT_EQ(c.vocabularySize, 4);
    EXPECT_EQ(c.corpusLength, 8);

    ReaderThreshold readerThreshold;
    const Corpus filteredCorpus = readerThreshold.reorderCorpus(c, 2, true);

    EXPECT_EQ(filteredCorpus.vocabularySize, 2);
    EXPECT_EQ(filteredCorpus.corpusLength, 8);

    const vector<double> expectedPl = {0.42857, 0.2857};
    const vector<double> expectedPr = {0.42857, 0.2857};

    for (int i = 0; i < expectedPl.size(); ++i) {
        EXPECT_NEAR(filteredCorpus.pl[i], expectedPl[i],
                    0.001) << "Pl differs at position " << i << ". Expected " << expectedPl[i] << " actual "
                           << filteredCorpus.pl[i];
    }
    for (int i = 0; i < expectedPr.size(); ++i) {
        EXPECT_NEAR(filteredCorpus.pr[i], expectedPr[i], 0.001)
                            << "Pr differs at position " << i << ". Expected " << expectedPl[i] << " actual "
                            << filteredCorpus.pl[i];
    }

    occurrence_type expectedOccurrences = {{{0, 0}, 2},
                                           {{1, 0}, 1}
    };
    EXPECT_THAT(filteredCorpus.occurrences, ::testing::ContainerEq(expectedOccurrences));
}

TEST(ReaderThresholdTest, testReaderThresholdSimpleCorpusNotStrict) {
    ReaderNoOrder reader;
    const Corpus c = reader.readFile("tests/test_data/abcd.txt");

    EXPECT_EQ(c.vocabularySize, 4);
    EXPECT_EQ(c.corpusLength, 8);

    ReaderThreshold readerThreshold;
    const Corpus filteredCorpus = readerThreshold.reorderCorpus(c, 2, false);

    EXPECT_EQ(filteredCorpus.vocabularySize, 2);
    EXPECT_EQ(filteredCorpus.corpusLength, 3);

    const vector<double> expectedPl = {0.6666, 0.3333};
    const vector<double> expectedPr = {1, 0};

    for (int i = 0; i < expectedPl.size(); ++i) {
        EXPECT_NEAR(filteredCorpus.pl[i], expectedPl[i],
                    0.001) << "Pl differs at position " << i << ". Expected " << expectedPl[i] << " actual "
                           << filteredCorpus.pl[i];
    }
    for (int i = 0; i < expectedPr.size(); ++i) {
        EXPECT_NEAR(filteredCorpus.pr[i], expectedPr[i], 0.001)
                            << "Pr differs at position " << i << ". Expected " << expectedPl[i] << " actual "
                            << filteredCorpus.pl[i];
    }

    occurrence_type expectedOccurrences = {{{0, 0}, 2},
                                           {{1, 0}, 1}
    };
    EXPECT_THAT(filteredCorpus.occurrences, ::testing::ContainerEq(expectedOccurrences));
}