#ifndef STOCHASTICEXCHANGE_H
#define STOCHASTICEXCHANGE_H


#include "../../models/Corpus.h"
#include "../Exchange/Exchange.h"
#include <set>
#include <algorithm>

/**
 * ExchangeAlgorithm implementation that allows for some randomness.
 */
class StochasticExchange : public Exchange {
private:
    double randomnessLevel = 0.0;

public:
    /**
     * Sets how often a swap should be made at random.
     * @param randomnessLevel percentage of swaps to be made at random.
     */
    void setRandomness(double percentage) { randomnessLevel = percentage; };

    StochasticExchange(const Corpus &corpus) : Exchange(corpus) { this->initialized = false; };

    ~StochasticExchange() override = default;

    string getName() override { return "StochasticExchange"; };
    /**
     * Runs EXCHANGE for one iteration.
     * @param minAMIChange minimum AMI threshold.
     * @return whether this clustering should be considered converged (either no swaps occurred, or
     * the minimum AMI change was below the provided threshold)
     */
    bool clusterOneIteration(double minAMIChange = DEFAULT_MIN_AMI_CHANGE);
protected:
    vector<word_type> clusterInternal(word_type noIterations, double minAMIChange);
};


#endif //STOCHASTICEXCHANGE_H
