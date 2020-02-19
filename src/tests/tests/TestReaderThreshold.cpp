#include <models/Corpus.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <readers/ReaderThreshold.h>

Corpus createSimpleCorpusThreshold(){
    //    the corpus here is a a b c d c a a
    Corpus abcdCorpus;
    abcdCorpus.vocabularySize = 4;
    abcdCorpus.corpusLength = 8;
    abcdCorpus.pl = {3, 1, 2, 1};
    const word_type no_transitions = abcdCorpus.getNumberOfTransitions();
    abcdCorpus.pr = {3, 1,
                     2, 1};
    abcdCorpus.idsToWords.insert({0, "a"});
    abcdCorpus.idsToWords.insert({1, "b"});
    abcdCorpus.idsToWords.insert({2, "c"});
    abcdCorpus.idsToWords.insert({3, "d"});
    abcdCorpus.occurrences.insert({{0,0}, 2});
    abcdCorpus.occurrences.insert({{0,1}, 1});
    abcdCorpus.occurrences.insert({{1,2}, 1});
    abcdCorpus.occurrences.insert({{2,0}, 1});
    abcdCorpus.occurrences.insert({{2,3}, 1});
    abcdCorpus.occurrences.insert({{3,2}, 1});
    abcdCorpus.wordCountAsNumbers.insert({0,4});
    abcdCorpus.wordCountAsNumbers.insert({1,1});
    abcdCorpus.wordCountAsNumbers.insert({2,2});
    abcdCorpus.wordCountAsNumbers.insert({3,1});

    return abcdCorpus;
}

TEST(ReaderThresholdTest, testReaderThresholdSimpleCorpusNotStrict) {
    const Corpus abcdCorpus = createSimpleCorpusThreshold();
    ReaderThreshold reader;
    const Corpus filteredCorpus = reader.reorderCorpus(abcdCorpus, 2, true);
    EXPECT_EQ(filteredCorpus.vocabularySize, 2);
    EXPECT_EQ(filteredCorpus.corpusLength, 8);
    const vector<double> expectedPl = {3, 2};
    const vector<double> expectedPr = {3, 2};
    EXPECT_THAT(filteredCorpus.pl, ::testing::ContainerEq(expectedPl));
    EXPECT_THAT(filteredCorpus.pr, ::testing::ContainerEq(expectedPr));

    occurrence_type expectedOccurrences = {{{0,0}, 2},
                                           {{1,0}, 1}
    };
    EXPECT_THAT(filteredCorpus.occurrences, ::testing::ContainerEq(expectedOccurrences));
}