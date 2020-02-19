#include <fstream>
#include "Exchange.h"

double Exchange::calculateAMI() {
    double AMI = 0;
    for (word_type clusterID1 = 0; clusterID1 < this->numClusters; ++clusterID1) {
        AMI += sumRowsEntropyOccurrences[clusterID1];
        AMI -= entropyLeft[clusterID1];
        AMI -= entropyRight[clusterID1];
    }
    return AMI;
}

vector<word_type>
Exchange::cluster(const word_type noClusters, const word_type noIterations, const double minAMIChange) {
//    assume the most popular noClusters - 1 words are their own clusters
//    all other words go into the last cluster
    vector<word_type> clusterAssignments = vector_word_type(corpus.vocabularySize, noClusters - 1);
    for (word_type i = 0; i < noClusters - 1; ++i) {
        clusterAssignments[i] = i;
    }
    return this->cluster(noClusters, noIterations, clusterAssignments);
}

vector<word_type> Exchange::cluster(const word_type noClusters, const word_type noIterations,
                                    const vector<word_type> clusterAssignments, const double minAMIChange) {
    initializeDataStructures(noClusters, clusterAssignments);
    return ExchangeAlgorithm::sortClusterAssignments(this->clusterInternal(noIterations, minAMIChange), numClusters);
}

vector<word_type> Exchange::clusterInternal(const word_type noIterations, const double minAMIChange) {
    if (noIterations > 0) {
        double AMI = calculateAMI();
        changesInPreviousIteration = 1;
        vector<double> amiChange;
#pragma omp parallel
        for (iteration = 0; iteration < noIterations; ++iteration) {
            if (changesInPreviousIteration == 0 && !AMIIncreasingOverThreshold) {
                break;
            }
#pragma omp barrier
#pragma omp single
            {
                changesInPreviousIteration = 0;
            }
            for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
                const word_type clusterToMoveFrom = wordsToClusters[wordID];
                if (clusterContent[clusterToMoveFrom].size() > 1) {
#pragma omp barrier
#pragma omp single
                    {
                        amiChange = vector<double>(numClusters, 0);
                    }
#pragma omp for
                    for (word_type clusterCandidate = 0; clusterCandidate < numClusters; ++clusterCandidate) {
                        if (clusterCandidate == clusterToMoveFrom) {
                            amiChange[clusterCandidate] = 0;
                        } else {
                            const double amiDiff = calculateAMIDiff(wordID, clusterCandidate);
                            amiChange[clusterCandidate] = amiDiff;
                        }
                    }
#pragma omp single
                    {
                        const auto clusterToMoveTo = (word_type) distance(amiChange.begin(),
                                                                          max_element(amiChange.begin(),
                                                                                      amiChange.end()));
                        if (clusterToMoveTo != wordsToClusters[wordID]) {
                            performMoveAndReturnAMIChange(wordID, clusterToMoveTo);
                            changesInPreviousIteration++;
                        }
                    }
                }
            }
#pragma omp single
            {
                const double newAMI = calculateAMI();
                const double AMIChangeInIteration = newAMI - AMI;
                AMI = newAMI;
                AMIIncreasingOverThreshold = AMIChangeInIteration > minAMIChange;
            }
        }
    }
    return wordsToClusters;
}

void Exchange::performMoveAndReturnAMIChange(const word_type wordID,
                                             const word_type clusterToMoveTo) {
    const word_type clusterToMoveFrom = this->wordsToClusters[wordID];

    plC[clusterToMoveFrom] -= corpus.pl[wordID];
    prC[clusterToMoveFrom] -= corpus.pr[wordID];
    entropyLeft[clusterToMoveFrom] = entropyTerm(plC[clusterToMoveFrom]);
    entropyRight[clusterToMoveFrom] = entropyTerm(prC[clusterToMoveFrom]);

    plC[clusterToMoveTo] += corpus.pl[wordID];
    prC[clusterToMoveTo] += corpus.pr[wordID];
    entropyLeft[clusterToMoveTo] = entropyTerm(plC[clusterToMoveTo]);
    entropyRight[clusterToMoveTo] = entropyTerm(prC[clusterToMoveTo]);

    clusterContent[clusterToMoveFrom].erase(wordID);
    clusterContent[clusterToMoveTo].insert(wordID);

    wordsToClusters[wordID] = clusterToMoveTo;

//            update occurrences
    for (word_type const &rightWord : toTheRightOf[wordID]) {
        if (rightWord != wordID) {
            const word_type noOfOccurrencesToTransfer = Utils::getOccurrence(corpus.occurrences, wordID, rightWord);
            const word_type rightCluster = wordsToClusters[rightWord];
            occurrencesClusters[clusterToMoveFrom][rightCluster] -= noOfOccurrencesToTransfer;
            occurrencesClusters[clusterToMoveTo][rightCluster] += noOfOccurrencesToTransfer;

            clusterToWord[clusterToMoveFrom][rightWord] -= noOfOccurrencesToTransfer;
            clusterToWord[clusterToMoveTo][rightWord] += noOfOccurrencesToTransfer;
        }
    }
    for (word_type const &leftWord : toTheLeftOf[wordID]) {
        if (leftWord != wordID) {
            const word_type noOfOccurrencesToTransfer = Utils::getOccurrence(corpus.occurrences, leftWord, wordID);
            const word_type leftCluster = wordsToClusters[leftWord];
            occurrencesClusters[leftCluster][clusterToMoveFrom] -= noOfOccurrencesToTransfer;
            occurrencesClusters[leftCluster][clusterToMoveTo] += noOfOccurrencesToTransfer;

            wordToCluster[leftWord][clusterToMoveFrom] -= noOfOccurrencesToTransfer;
            wordToCluster[leftWord][clusterToMoveTo] += noOfOccurrencesToTransfer;
        }
    }
    const word_type noOfOccurrencesToItself = Utils::getOccurrence(corpus.occurrences, wordID, wordID);
    if (noOfOccurrencesToItself > 0) {
        occurrencesClusters[clusterToMoveFrom][clusterToMoveFrom] -= noOfOccurrencesToItself;
        occurrencesClusters[clusterToMoveTo][clusterToMoveTo] += noOfOccurrencesToItself;

        clusterToWord[clusterToMoveFrom][wordID] -= noOfOccurrencesToItself;
        clusterToWord[clusterToMoveTo][wordID] += noOfOccurrencesToItself;

        wordToCluster[wordID][clusterToMoveFrom] -= noOfOccurrencesToItself;
        wordToCluster[wordID][clusterToMoveTo] += noOfOccurrencesToItself;
    }

    const vector<word_type> cl = {clusterToMoveFrom, clusterToMoveTo};
    for (word_type const &i : cl) {
        double entropyChange = 0;
#pragma omp parallel for reduction(+:entropyChange)
        for (word_type j = 0; j < numClusters; ++j) {
            double diff1 = 0;
            diff1 -= entropyOccurrences[i][j];
            entropyOccurrences[i][j] = entropyTerm(
                    (double) occurrencesClusters[i][j] / corpus.getNumberOfTransitions());
            diff1 += entropyOccurrences[i][j];

            entropyChange += diff1;
            this->sumColumnsEntropyOccurrences[j] += diff1;
        }
        this->sumRowsEntropyOccurrences[i] += entropyChange;
    }
    for (word_type const &i : cl) {
        double entropyChange = 0;
#pragma omp parallel for reduction(+:entropyChange)
        for (word_type j = 0; j < numClusters; ++j) {
            double diff2 = 0;
            diff2 -= entropyOccurrences[j][i];
            entropyOccurrences[j][i] = entropyTerm(
                    (double) occurrencesClusters[j][i] / corpus.getNumberOfTransitions());
            diff2 += entropyOccurrences[j][i];
            entropyChange += diff2;
            this->sumRowsEntropyOccurrences[j] += diff2;
        }
        this->sumColumnsEntropyOccurrences[i] += entropyChange;
    }
}

double Exchange::calculateAMIDiff(const word_type wordID,
                                  const word_type clusterCandidate) {
    const word_type clusterToMoveFrom = wordsToClusters[wordID];
    double amiDiff = 0;
//    this is what we used to have
    amiDiff += entropyLeft[clusterToMoveFrom];
    amiDiff += entropyRight[clusterToMoveFrom];
    amiDiff += entropyLeft[clusterCandidate];
    amiDiff += entropyRight[clusterCandidate];
    amiDiff -= sumRowsEntropyOccurrences[clusterCandidate];
    amiDiff -= sumColumnsEntropyOccurrences[clusterCandidate];
    amiDiff += entropyTerm(
            (double) occurrencesClusters[clusterCandidate][clusterCandidate] / corpus.getNumberOfTransitions());
    amiDiff -= sumRowsEntropyOccurrences[clusterToMoveFrom];
    amiDiff -= sumColumnsEntropyOccurrences[clusterToMoveFrom];
    amiDiff += entropyTerm(
            (double) occurrencesClusters[clusterToMoveFrom][clusterToMoveFrom] / corpus.getNumberOfTransitions());
    amiDiff += entropyTerm(
            (double) occurrencesClusters[clusterToMoveFrom][clusterCandidate] / corpus.getNumberOfTransitions());
    amiDiff += entropyTerm(
            (double) occurrencesClusters[clusterCandidate][clusterToMoveFrom] / corpus.getNumberOfTransitions());

//    this is what we're getting
    for (word_type clusterID1 = 0; clusterID1 < numClusters; ++clusterID1) {
        if (clusterID1 != clusterCandidate && clusterID1 != clusterToMoveFrom) {
            const auto newEntropy = (double) (occurrencesClusters[clusterCandidate][clusterID1]
                                              + wordToCluster[wordID][clusterID1]) / corpus.getNumberOfTransitions();
            const auto contribution = entropyTerm(newEntropy);
            amiDiff += contribution;
        }
    }

    for (word_type clusterID1 = 0; clusterID1 < numClusters; ++clusterID1) {
        if (clusterID1 != clusterCandidate && clusterID1 != clusterToMoveFrom) {
            const auto newOcc = (double) (occurrencesClusters[clusterID1][clusterCandidate]
                                          + clusterToWord[clusterID1][wordID]) / corpus.getNumberOfTransitions();
            const auto contribution = entropyTerm(newOcc);
            amiDiff += contribution;
        }
    }

    for (word_type clusterID1 = 0; clusterID1 < numClusters; ++clusterID1) {
        if (clusterID1 != clusterCandidate && clusterID1 != clusterToMoveFrom) {
            const auto newOcc = (double) (occurrencesClusters[clusterToMoveFrom][clusterID1]
                                          - wordToCluster[wordID][clusterID1]) / corpus.getNumberOfTransitions();
            const auto contribution = entropyTerm(newOcc);
            amiDiff += contribution;
        }
    }

    for (word_type clusterID1 = 0; clusterID1 < numClusters; ++clusterID1) {
        if (clusterID1 != clusterCandidate && clusterID1 != clusterToMoveFrom) {
            const auto newOcc = (double) (occurrencesClusters[clusterID1][clusterToMoveFrom]
                                          - clusterToWord[clusterID1][wordID]) / corpus.getNumberOfTransitions();
            const auto contribution = entropyTerm(newOcc);
            amiDiff += contribution;
        }
    }

    const double newOccCandSource = (double) (occurrencesClusters[clusterCandidate][clusterToMoveFrom] -
                                              clusterToWord[clusterCandidate][wordID] +
                                              wordToCluster[wordID][clusterToMoveFrom] -
                                              Utils::getOccurrence(corpus.occurrences, wordID, wordID)) /
                                    corpus.getNumberOfTransitions();
    const double contributionCandSource = entropyTerm(newOccCandSource);
    amiDiff += contributionCandSource;

    const double newOccSourceCand = (double) (occurrencesClusters[clusterToMoveFrom][clusterCandidate] -
                                              wordToCluster[wordID][clusterCandidate] +
                                              clusterToWord[clusterToMoveFrom][wordID] -
                                              Utils::getOccurrence(corpus.occurrences, wordID, wordID)) /
                                    corpus.getNumberOfTransitions();
    const double contributionSourceCand = entropyTerm(newOccSourceCand);
    amiDiff += contributionSourceCand;

    const double newOccCandCand = (double) (occurrencesClusters[clusterCandidate][clusterCandidate] +
                                            clusterToWord[clusterCandidate][wordID] +
                                            wordToCluster[wordID][clusterCandidate] +
                                            Utils::getOccurrence(corpus.occurrences, wordID, wordID)) /
                                  corpus.getNumberOfTransitions();
    const double contributionCandCand = entropyTerm(newOccCandCand);
    amiDiff += contributionCandCand;

    const word_type lossSourceSource = clusterToWord[clusterToMoveFrom][wordID] +
                                       wordToCluster[wordID][clusterToMoveFrom] -
                                       Utils::getOccurrence(corpus.occurrences, wordID, wordID);
    const double newOccSourceSource =
            (double) (occurrencesClusters[clusterToMoveFrom][clusterToMoveFrom] - lossSourceSource) /
            corpus.getNumberOfTransitions();
    const double contributionSourceSource = entropyTerm(newOccSourceSource);
    amiDiff += contributionSourceSource;

    const double newPlCandidate = plC[clusterCandidate] + corpus.pl[wordID];
    amiDiff -= entropyTerm(newPlCandidate);

    const double newPrCandidate = prC[clusterCandidate] + corpus.pr[wordID];
    amiDiff -= entropyTerm(newPrCandidate);

    const double newPlSource = plC[clusterToMoveFrom] - corpus.pl[wordID];
    amiDiff -= entropyTerm(newPlSource);

    const double newPrSource = prC[clusterToMoveFrom] - corpus.pr[wordID];
    amiDiff -= entropyTerm(newPrSource);

    return amiDiff;
}

void Exchange::initializeDataStructures(const word_type numClusters, const vector<word_type> &clusterAssignments) {
    this->numClusters = numClusters;
    occurrencesClusters = matrix_occurrences(numClusters, vector_word_type(numClusters, 0));
    entropyOccurrences = matrix_double(numClusters, vector<double>(numClusters, 0));
    wordsToClusters = vector_word_type(clusterAssignments.begin(), clusterAssignments.end());
    clusterContent = vector<set<word_type>>(numClusters, set<word_type>());
    this->plC = vector<double>(numClusters, 0);
    this->prC = vector<double>(numClusters, 0);
    sumColumnsEntropyOccurrences = vector<double>(numClusters, 0);
    sumRowsEntropyOccurrences = vector<double>(numClusters, 0);
    this->entropyLeft = vector<double>(numClusters, 0);
    this->entropyRight = vector<double>(numClusters, 0);
    this->wordToCluster = vector<vector_word_type>(corpus.vocabularySize, vector_word_type(numClusters, 0));
    this->clusterToWord = vector<vector_word_type>(numClusters, vector_word_type(corpus.vocabularySize, 0));
    for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
        const word_type destinationCluster = wordsToClusters[wordID];
        clusterContent[destinationCluster].insert(wordID);
        plC[destinationCluster] += corpus.pl[wordID];
        prC[destinationCluster] += corpus.pr[wordID];
    }

    auto leftiesAndRighties = Corpus::computeLeftiesAndRighties(corpus);
    this->toTheLeftOf = leftiesAndRighties.first;
    this->toTheRightOf = leftiesAndRighties.second;

    for (word_type wordID = 0; wordID < corpus.vocabularySize; ++wordID) {
        const word_type destinationCluster = wordsToClusters[wordID];
        for (word_type const &rightWord : toTheRightOf[wordID]) {
            const word_type rightCluster = wordsToClusters[rightWord];
            const word_type occurrence = Utils::getOccurrence(corpus.occurrences, wordID, rightWord);
            occurrencesClusters[destinationCluster][rightCluster] += occurrence;
            wordToCluster[wordID][rightCluster] += occurrence;
            clusterToWord[destinationCluster][rightWord] += occurrence;
        }
    }

#pragma omp parallel for
    for (word_type clusterID = 0; clusterID < numClusters; ++clusterID) {
        entropyLeft[clusterID] = entropyTerm(plC[clusterID]);
        entropyRight[clusterID] = entropyTerm(prC[clusterID]);
    }

    for (word_type clusterID1 = 0; clusterID1 < numClusters; ++clusterID1) {
        for (word_type clusterID2 = 0; clusterID2 < numClusters; ++clusterID2) {
            double clusterProbability =
                    (double) occurrencesClusters[clusterID1][clusterID2] / corpus.getNumberOfTransitions();
            entropyOccurrences[clusterID1][clusterID2] = entropyTerm(clusterProbability);
            sumRowsEntropyOccurrences[clusterID1] += entropyOccurrences[clusterID1][clusterID2];
            sumColumnsEntropyOccurrences[clusterID2] += entropyOccurrences[clusterID1][clusterID2];
        }
    }
    this->initialized = true;
}

double inline Exchange::entropyTerm(const double argument) {
    if (argument == 0 || argument == 1) {
        return 0;
    } else {
        return argument * std::log2(argument);
    }
}

void Exchange::prepareClustering(const word_type numClusters, const vector<word_type> &clusterAssignments) {
    initializeDataStructures(numClusters, clusterAssignments);
}

void Exchange::prepareClustering(const word_type numClusters) {
//    assume the most popular noClusters - 1 words are their own clusters
//    all other words go into the last cluster
    vector<word_type> clusterAssignments = vector_word_type(corpus.vocabularySize, numClusters - 1);
    for (word_type i = 0; i < numClusters - 1; ++i) {
        clusterAssignments[i] = i;
    }
    prepareClustering(numClusters, clusterAssignments);
}

bool Exchange::clusterOneIteration(const double minAMIChange) {
    this->clusterInternal(1, minAMIChange);
    return changesInPreviousIteration > 0 && AMIIncreasingOverThreshold;
}

vector_word_type Exchange::getClusterAssignments() const {
    return ExchangeAlgorithm::sortClusterAssignments(this->wordsToClusters, this->numClusters);
}

uint32_t Exchange::getChangesInPreviousIteration() const {
    return changesInPreviousIteration;
}
