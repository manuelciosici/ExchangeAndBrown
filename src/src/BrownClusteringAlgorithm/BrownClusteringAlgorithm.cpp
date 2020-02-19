#include "BrownClusteringAlgorithm.h"
#include <omp.h>
#include "../Utils.h"
#include "easylogging++/easylogging++.h"


BrownClusteringAlgorithm::BrownClusteringAlgorithm(const Corpus &corpus) : clustering(1, 2) {
    this->corpus = corpus;
    const pair<matrix_occurrences, matrix_occurrences> leftiesAndRighties = Corpus::computeLeftiesAndRighties(corpus);
    this->toTheLeftOf = leftiesAndRighties.first;
    this->toTheRightOf = leftiesAndRighties.second;
}

vector_word_type BrownClusteringAlgorithm::cluster(const word_type noClusters, const word_type windowSize) {
    if (noClusters > windowSize || windowSize == 0) {
        const string error_message = string("Window size (currently set to ") + to_string(windowSize) +
                                     ") should greater or equal to the number of clusters (currently set to " +
                                     to_string(noClusters) + ").";
        throw std::runtime_error(error_message);
    } else if (noClusters <= 1 || noClusters > corpus.vocabularySize) {
        const string error_message = string("The number of clusters number of clusters (currently set to ") +
                                     to_string(noClusters) +
                                     ") should be larger than 1 and smaller than the number of words in the vocabulary " +
                                     "(which is " + to_string(corpus.vocabularySize) + ").";
        throw std::runtime_error(error_message);
    }
    auto currentWindowSize = windowSize;

    clustering = Clustering(currentWindowSize, corpus.vocabularySize);
    for (word_type i = 0; i < this->corpus.vocabularySize; ++i) {
        // add all leaves past windowSize to queue
        if (i >= currentWindowSize) {
            this->leavesToProcess.push(std::move(make_unique<LeafNode>(i)));
        }

    }

    this->initializeDataStructures(currentWindowSize);

//    compute initial q
    this->oldI = BrownClusteringAlgorithm::computeQInitial(q, occurrencesC, currentWindowSize, plC, prC,
                                                           corpus.corpusLength - 1);

//    compute initial sk
    BrownClusteringAlgorithm::computeSk(sk, q, currentWindowSize);
    word_type mergeID = 0;
    while (!this->leavesToProcess.empty() || currentWindowSize > noClusters) {
//        consider combinations
        MergeData mergeData = findLowestAMILoss(currentWindowSize, currentWindowSize - 2);
        mergeData.mergeID = mergeID;

//        update AMI to reflect the drop from merging the two clusters
        this->oldI = mergeData.amiAfterLoss;


        const auto remaining = this->leavesToProcess.size() + currentWindowSize - 1;
        LOG(INFO) << remaining << " merges remaining";

//        merge clusters (create inner node)
        clustering.mergeClusters(mergeData);

//        update plC, prC
        this->plC[mergeData.to] += this->plC[mergeData.from];
        this->prC[mergeData.to] += this->prC[mergeData.from];

//        update occurrencesC
        const auto sizeOfOccurrencesC = occurrencesC.size();
        for (auto m = 0; m < sizeOfOccurrencesC; ++m) {
            if (m != mergeData.from) {
                occurrencesC[mergeData.to][m] += occurrencesC[mergeData.from][m];
                occurrencesC[m][mergeData.to] += occurrencesC[m][mergeData.from];
            } else {
                occurrencesC[mergeData.to][mergeData.to] += occurrencesC[mergeData.from][mergeData.from];
            }
        }

        unique_ptr<LeafNode> nextNode = getNextCluster();
        if (nextNode != nullptr) {
            const auto idOfNext = nextNode->getWord();
//            bring in plC, prC for new node
            this->plC[mergeData.from] = this->corpus.pl[idOfNext];
            this->prC[mergeData.from] = this->corpus.pr[idOfNext];

//            update clusterToTree
            clustering.assignLeafToCluster(std::move(nextNode), mergeData.from);

//            update clusterContent
            clustering.assignWordToEmptyCluster(idOfNext, mergeData.from);

//            update occurrencesC
            for (auto m = 0; m < sizeOfOccurrencesC; ++m) {
                occurrencesC[m][mergeData.from] = 0;
                occurrencesC[mergeData.from][m] = 0;
            }

            for (const auto i : this->toTheLeftOf[idOfNext]) {
                if (i <= idOfNext) {
                    const word_type clusterOfI = clustering.getWordsToClusters()[i];
                    occurrencesC[clusterOfI][mergeData.from] += Utils::getOccurrence(this->corpus.occurrences, i,
                                                                                     idOfNext);
                } else {
                    break;
                }
            }

            for (const auto i : this->toTheRightOf[idOfNext]) {
                if (i <= idOfNext) {
                    const word_type clusterOfI = clustering.getWordsToClusters()[i];
                    occurrencesC[mergeData.from][clusterOfI] += Utils::getOccurrence(this->corpus.occurrences, idOfNext,
                                                                                     i);
                } else { //outside window
                    break;
                }
            }
            this->updateQInclusion(mergeData.to, mergeData.from, currentWindowSize, corpus.corpusLength - 1);
        } else {
            clustering.moveLastCluster(mergeData.from);
            reduceOneCluster(mergeData, currentWindowSize, corpus.corpusLength - 1);
            currentWindowSize--;
        }
        mergeID++;
    }
    clustering.setAllCurrentNodesAsFinal();

//    We must pick up the flat clustering at this point as later one we reuse the flat clustering data structure during
// clustering from noClusters down to 2 clusters.
    const vector_word_type valueToReturn(clustering.getWordsToClusters());

    LOG(INFO) << "Reducing remaining clusters";
//    reduce the noClusters to one single cluster
    for (word_type remaining = noClusters; remaining > 2; --remaining) {
        LOG(INFO) << remaining - 1 << " merges remaining";
        MergeData mergeData = findLowestAMILoss(remaining, remaining - 2);
        mergeData.mergeID = mergeID;

//        update AMI to reflect the drop from merging the two clusters
        this->oldI = mergeData.amiAfterLoss;

//        merge clusters (create inner node)
        clustering.mergeClusters(mergeData);

//        update plC, prC
        this->plC[mergeData.to] += this->plC[mergeData.from];
        this->prC[mergeData.to] += this->prC[mergeData.from];

//        update occurrencesC
        for (word_type m = 0; m < remaining; ++m) {
            if (m != mergeData.from) {
                occurrencesC[mergeData.to][m] += occurrencesC[mergeData.from][m];
                occurrencesC[m][mergeData.to] += occurrencesC[m][mergeData.from];
            } else {
                occurrencesC[mergeData.to][mergeData.to] += occurrencesC[mergeData.from][mergeData.from];
            }
        }
        clustering.moveLastCluster(mergeData.from);
        reduceOneCluster(mergeData, remaining, corpus.corpusLength - 1);
        mergeID++;
    }
    LOG(INFO) << "Finalizing tree construction...";
    clustering.attachToRoot(mergeID);
    return valueToReturn;
}

void
BrownClusteringAlgorithm::updateSkAfterRemovalOf(const word_type intoID, const word_type fromID,
                                                 const word_type currentWindowSize) {
#pragma omp parallel for
    for (word_type m = 0; m < currentWindowSize; ++m) {
        sk[m] -= q[intoID][m];
        sk[m] -= q[m][intoID];
        sk[m] -= q[fromID][m];
        sk[m] -= q[m][fromID];
    }
}

void
BrownClusteringAlgorithm::computeSk(vector<double> &sk, const matrix_double &q, const word_type currentWindowSize) {
    for (word_type i = 0; i < currentWindowSize; ++i) {
        for (word_type j = 0; j < currentWindowSize; ++j) {
            sk[j] += q[i][j];
            if (i != j) {
                sk[i] += q[i][j];
            }
        }
    }
}

double BrownClusteringAlgorithm::computeQInitial(matrix_double &q, const matrix_occurrences &occurrencesC,
                                                 const word_type currentWindowSize, vector<double> &plC,
                                                 vector<double> &prC, const word_type corpusLength) {
    double I = 0;
#pragma omp parallel for reduction (+:I)
    for (word_type i = 0; i < currentWindowSize; ++i) {
        for (word_type j = 0; j < currentWindowSize; ++j) {
            q[i][j] = Utils::computeMI((double) occurrencesC[i][j] / corpusLength, plC[i], prC[j]);
            I += q[i][j];
        }
    }
    return I;
}

void BrownClusteringAlgorithm::updateQMergedCluster(const word_type intoID, const word_type currentWindowSize,
                                                    const word_type corpusLength) {
    sk[intoID] = 0;
    double sk_i = 0;
#pragma omp parallel for reduction (+:sk_i)
    for (word_type m = 0; m < currentWindowSize; ++m) {
        const double joint1 = (double) this->occurrencesC[m][intoID] / corpusLength;
        const double joint2 = (double) this->occurrencesC[intoID][m] / corpusLength;
        this->q[m][intoID] = Utils::computeMI(joint1, plC[m], prC[intoID]);
        this->q[intoID][m] = Utils::computeMI(joint2, plC[intoID], prC[m]);
        sk_i += this->q[m][intoID];
        if (m != intoID) {
            sk_i += this->q[intoID][m];
            sk[m] += this->q[m][intoID];
            sk[m] += this->q[intoID][m];
        }
    }
    sk[intoID] = sk_i;
}

void BrownClusteringAlgorithm::initializeDataStructures(const word_type windowSize) {
    //    initialize sk
    this->sk = vector<double>(windowSize, 0);

//    copy over the initial plC and prC
    this->plC = vector<double>(corpus.pl.begin(), corpus.pl.end());
    this->prC = vector<double>(corpus.pr.begin(), corpus.pr.end());

//  initialize q and occurrencesC
    this->q = matrix_double(windowSize, vector<double>(windowSize));
    this->occurrencesC = matrix_occurrences(windowSize, vector_word_type(windowSize, 0));

    //    copy over the initial occurrences
#pragma omp parallel for schedule(dynamic)
    for (word_type i = 0; i < windowSize; ++i) {
        for (word_type j : toTheRightOf[i]) {
            if (j < windowSize) {
                occurrencesC[i][j] = Utils::getOccurrence(this->corpus.occurrences, i, j);
            }
        }
    }
}

double
BrownClusteringAlgorithm::computeQMergeContribution(const word_type i, const word_type j,
                                                    const word_type currentWindowSize,
                                                    const word_type corpusLength) const {
    const double plMergeI = this->plC[i] + this->plC[j];
    const double prMergeI = this->prC[i] + this->prC[j];
    double valueToReturn = 0;

    for (word_type m = 0; m < currentWindowSize; ++m) {
        if (m != i && m != j) {
            const double pkMerge_i_m = (double) (this->occurrencesC[i][m] +
                                                 this->occurrencesC[j][m]) / corpusLength;
            const double pkMerge_m_i = (double) (this->occurrencesC[m][i] +
                                                 this->occurrencesC[m][j]) / corpusLength;

            valueToReturn += Utils::computeMI(pkMerge_i_m, plMergeI, prC[m]);
            valueToReturn += Utils::computeMI(pkMerge_m_i, plC[m], prMergeI);
        } else if (m == i) {
            double pkMerge_ij_ij = (double) (this->occurrencesC[i][i] +
                                             this->occurrencesC[i][j] +
                                             this->occurrencesC[j][j] +
                                             this->occurrencesC[j][i]) / corpusLength;
            valueToReturn += Utils::computeMI(pkMerge_ij_ij, plMergeI, prMergeI);
        }
    }
    return valueToReturn;
}

BrownClusteringAlgorithm::MergeData
BrownClusteringAlgorithm::findLowestAMILoss(const word_type currentWindowSize, const word_type limitI) const {
    const auto numThreads = (unsigned long) omp_get_max_threads();
    auto candidateAMI = vector<double>(numThreads, 0);
    auto candidateIs = vector<word_type>(numThreads, 0);
    auto candidateJs = vector<word_type>(numThreads, 1);
    auto candidateValue = vector<double>(numThreads, std::numeric_limits<double>::max());

#pragma omp parallel for schedule(dynamic)
    for (word_type i = 0; i <= limitI; ++i) {
        const auto threadID = omp_get_thread_num();
        for (word_type j = i + 1; j < currentWindowSize; ++j) {
//         we can't merge a cluster into itself
//                also, we must respect order

//                  compute I for merge
            const double qMergeContribution = this->computeQMergeContribution(i, j, currentWindowSize,
                                                                              corpus.corpusLength - 1);

//            This is equation 14 in Brown.
            const double newI = oldI - sk[i] - sk[j] + q[i][j] + q[j][i] + qMergeContribution;

//                compute loss
            const double loss = oldI - newI;
//            Without this clause, clustering with multiple threads will not be identical between runs as for
//            the same loss, threads with different i and j can arrive and occupy the candidate data structures.
//            This clause make sure that the candidates are updated with a bias towards lower i and j, in other words,
//            it prioritizes making merges between clusters with lower IDs. This behavior is specific to this
//            implementation since it is not defined by Brown.
            if (loss <= candidateValue[threadID] && i <= candidateIs[threadID] && j <= candidateJs[threadID]) {
                candidateValue[threadID] = loss;
                candidateAMI[threadID] = newI;
                candidateIs[threadID] = i;
                candidateJs[threadID] = j;
            }
        }
    }
    const auto best_index = std::distance(candidateValue.begin(),
                                          std::min_element(candidateValue.begin(), candidateValue.end()));

    MergeData valueToReturn = {};
    valueToReturn.from = candidateJs[best_index];
    valueToReturn.to = candidateIs[best_index];
    valueToReturn.amiLoss = this->oldI - candidateAMI[best_index];
    valueToReturn.amiAfterLoss = candidateAMI[best_index];
    return valueToReturn;
}

void
BrownClusteringAlgorithm::updateQNewCluster(const word_type fromID, const word_type intoID,
                                            const word_type currentWindowSize, const word_type corpusLength) {
    sk[fromID] = 0;
    double sk_i = 0;
    double parallelOldI = 0;
#pragma omp parallel for reduction (+:sk_i, parallelOldI)
    for (word_type m = 0; m < currentWindowSize; ++m) {
        const double joint1 = (double) occurrencesC[m][fromID] / corpusLength;
        const double joint2 = (double) occurrencesC[fromID][m] / corpusLength;
        this->q[m][fromID] = Utils::computeMI(joint1, plC[m], prC[fromID]);
        this->q[fromID][m] = Utils::computeMI(joint2, plC[fromID], prC[m]);
        sk_i += this->q[m][fromID];
        parallelOldI += this->q[m][fromID];
        parallelOldI += this->q[fromID][m];
        if (m != fromID) {
            sk_i += this->q[fromID][m];
            if (m != intoID) {
                sk[m] += this->q[m][fromID];
                sk[m] += this->q[fromID][m];
            }
        }
    }
    sk[fromID] = sk_i;
    oldI += parallelOldI;
}

void
BrownClusteringAlgorithm::reduceOneCluster(const MergeData &merge, const word_type currentWindowSize,
                                           const word_type corpusLength) {
    swapForVector(plC, merge.from, currentWindowSize);
    swapForVector(prC, merge.from, currentWindowSize);

//            swap occurrencesC
    swapForMatrix(occurrencesC, merge.from, currentWindowSize);

    this->updateQReduction(merge.to, merge.from, currentWindowSize, corpusLength);
}

void
BrownClusteringAlgorithm::updateQReduction(const word_type intoID, const word_type fromID,
                                           const word_type currentWindowSize, const word_type corpusLength) {
    updateSkAfterRemovalOf(intoID, fromID, currentWindowSize);
    swapForVector(sk, fromID, currentWindowSize);
    swapForMatrix(q, fromID, currentWindowSize);
    this->updateQMergedCluster(intoID, currentWindowSize, corpusLength);
}

void
BrownClusteringAlgorithm::updateQInclusion(const word_type intoID, const word_type fromID,
                                           const word_type currentWindowSize,
                                           const word_type corpusLength) {
    updateSkAfterRemovalOf(intoID, fromID, currentWindowSize);
    this->updateQMergedCluster(intoID, currentWindowSize, corpusLength);
    this->updateQNewCluster(fromID, intoID, currentWindowSize, corpusLength);
}

BrownClusteringAlgorithm::Clustering::Clustering(const word_type numberOfClusters, const word_type vocabularySize) {
    this->clusterToTree = vector<unique_ptr<Node>>(numberOfClusters);
    this->vocabularySize = vocabularySize;
    this->clusterContent = vector<vector_word_type>(numberOfClusters, vector_word_type());
    this->wordsToClusters = vector_word_type(vocabularySize);
    maxClusterID = numberOfClusters - 1;
    for (word_type i = 0; i < numberOfClusters; i++) {
        assignWordToEmptyCluster(i, i);
        this->clusterToTree[i] = make_unique<LeafNode>(i);
    }
}

void BrownClusteringAlgorithm::Clustering::mergeClusters(const BrownClusteringAlgorithm::MergeData &mergeData) {
    if (mergeData.from > maxClusterID || mergeData.to > maxClusterID) {
        throw runtime_error(
                string("Merge ID (from, to) = " + to_string(mergeData.from) + "," + to_string(mergeData.to) +
                       "is outside maxID = " +
                       to_string(maxClusterID)));
    }
    for (auto elementOfSecondCluster : clusterContent[mergeData.from]) {
        wordsToClusters[elementOfSecondCluster] = mergeData.to;
    }
    clusterContent[mergeData.to].insert(clusterContent[mergeData.to].end(),
                                        clusterContent[mergeData.from].begin(),
                                        clusterContent[mergeData.from].end());
    clusterContent[mergeData.from].clear();
    unique_ptr<InnerNode> newNode = make_unique<InnerNode>(std::move(clusterToTree[mergeData.from]),
                                                           std::move(clusterToTree[mergeData.to]));
    newNode->setAmiLoss(mergeData.amiLoss);
    newNode->setAmiAfterLoss(mergeData.amiAfterLoss);
    newNode->setMergeID(mergeData.mergeID);
    clusterToTree[mergeData.to] = std::move(newNode);
}

void BrownClusteringAlgorithm::Clustering::moveLastCluster(word_type destinationID) {
    if (destinationID > maxClusterID) {
        throw runtime_error(
                string("Destination ID = " + to_string(destinationID) + "is outside maxID = " +
                       to_string(maxClusterID)));
    }
    if (!clusterContent[destinationID].empty()) {
        throw runtime_error(
                string("Destination ID = " + to_string(destinationID) + "is not empty. Current size is " +
                       to_string(clusterContent[destinationID].size())));
    }
    if (destinationID != maxClusterID) {
        clusterContent[destinationID].clear();
        clusterContent[destinationID].insert(clusterContent[destinationID].end(),
                                             clusterContent[maxClusterID].begin(),
                                             clusterContent[maxClusterID].end());
        for (auto elementOfSecondCluster : clusterContent[maxClusterID]) {
            wordsToClusters[elementOfSecondCluster] = destinationID;
        }
        clusterContent[maxClusterID].clear();
        if (clusterToTree[maxClusterID] == nullptr) {
            throw runtime_error(
                    string("Reference at maxClusterID = " + to_string(maxClusterID) + " is already null"));
        }
        clusterToTree[destinationID] = std::move(clusterToTree[maxClusterID]);
    }
    maxClusterID--;
}

void BrownClusteringAlgorithm::Clustering::assignWordToEmptyCluster(word_type wordID, word_type destinationClusterID) {
    if (destinationClusterID > maxClusterID) {
        throw runtime_error(
                string("Destination ID = " + to_string(destinationClusterID) + "is outside maxID = " +
                       to_string(maxClusterID)));
    }
    if (wordID >= vocabularySize) {
        throw runtime_error(
                string("Word ID = " + to_string(wordID) + "is greater that vocabulary size = " +
                       to_string(wordsToClusters.size())));
    }
    if (!clusterContent[destinationClusterID].empty()) {
        throw runtime_error(
                string("Destination ID = " + to_string(destinationClusterID) + "is not empty. Current size is " +
                       to_string(clusterContent[destinationClusterID].size())));
    }
    wordsToClusters[wordID] = destinationClusterID;
    clusterContent[destinationClusterID].push_back(wordID);
}

void BrownClusteringAlgorithm::Clustering::assignLeafToCluster(unique_ptr<LeafNode> leaf,
                                                               const word_type destinationClusterID) {
    if (clusterToTree[destinationClusterID] != nullptr) {
        throw runtime_error(
                string("Destination destinationClusterID = " + to_string(destinationClusterID) + " is not null"));
    } else if (leaf == nullptr) {
        throw runtime_error(
                string("Leaf cannot be null pointer"));
    } else { clusterToTree[destinationClusterID] = std::move(leaf); }
}

void BrownClusteringAlgorithm::Clustering::setAllCurrentNodesAsFinal() {
    for (word_type i = 0; i <= maxClusterID; ++i) {
        clusterToTree[i]->setIsFinalCluster(true);
    }
}

void BrownClusteringAlgorithm::Clustering::attachToRoot(const word_type mergeID) {
    bool leftNull = clusterToTree[0] == nullptr;
    bool rightNull = clusterToTree[1] == nullptr;
    if (leftNull || rightNull) {
        string errorMsg("One of root's children is a null reference left is null: " + to_string(leftNull) +
                        ", right is null: " + to_string(rightNull));
        cerr << errorMsg << endl;
        throw runtime_error(errorMsg);
    }
    this->root = make_unique<RootNode>(std::move(clusterToTree[0]), std::move(clusterToTree[1]));
    root->setMergeID(mergeID);
}
