#include "Utils.h"

double Utils::logNoInf(const double argument) {
    if (argument == 0) {
        return 0;
    } else {
        double value = std::log2(argument);
        if (std::isinf(value) || std::isnan(value)) {
            value = 0;
        }
        return value;
    }

}

double Utils::computeMI(const double jointProb, const double pl, const double pr) {
    if (jointProb == 0 || pl == 0 || pr == 0) {
        return 0;
    } else {
        return jointProb * Utils::logNoInf(
                jointProb / (pl * pr));
    }
}