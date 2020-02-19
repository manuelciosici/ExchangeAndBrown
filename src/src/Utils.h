#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <sstream>
#include <iterator>
#include <cmath>
#include <limits>
using namespace std;


typedef uint32_t word_type;
typedef vector<word_type> vector_word_type;
typedef vector<vector_word_type> matrix_occurrences;
typedef vector<vector<double>> matrix_double;
typedef pair<word_type, word_type> pair_of_word_type;
typedef map<pair_of_word_type, word_type> occurrence_type;
typedef map<pair_of_word_type, double> occurrence_probability_type;

class Utils {
public:
    Utils() {};

    virtual ~Utils() {};

    static double logNoInf(double argument);

    static inline bool logically_equal(double a, double b) {
        return a == b ||
               std::abs(a - b) < std::abs(std::min(a, b)) * std::numeric_limits<double>::epsilon();
    }

    static inline word_type
    getOccurrence(const occurrence_type &occ, const word_type i, const word_type j) {
        const occurrence_type::key_type pair = {i, j};
        occurrence_type::const_iterator it = occ.find(pair);
        if (it == occ.end()) {
            return 0;
        } else {
            return it->second;
        }
    }

    static inline double
    getOccurrence(const occurrence_probability_type &occ, const word_type i, const word_type j) {
        const occurrence_type::key_type pair = {i, j};
        if (occ.count(pair) == 0) {
            return 0;
        } else {
            return pair.second;
        }
    }

    static double computeMI(const double jointProb, const double pl, const double pr);

    template<typename K, typename V>
    static V getWithDefault(const std::map<K, V> &m, const K &key, const V &defval) {
        typename std::map<K, V>::const_iterator it = m.find(key);
        if (it == m.end()) {
            return defval;
        } else {
            return it->second;
        }
    }

    template<typename Ts>
    static void
    writeMultipleScoresToCSV(const string &outputFileName, const vector<string> columnNames,
                             const vector<vector<Ts>> scores,
                             std::function<string(Ts)> &extractor) {
        ofstream output(outputFileName);
        if (!output.is_open()) {
            throw runtime_error("Cannot open file " + outputFileName);
        }
        if (scores.size() == 0) {
            throw invalid_argument("Must provide at least one Silhouette Coefficient ordering");
        }
        if (scores.size() != columnNames.size()) {
            std::stringstream streamError("Column number (");
            streamError << columnNames.size();
            streamError << " does not match number of Silhouette Coefficient rankings (";
            streamError << scores.size();
            streamError << ")!";
            throw invalid_argument(streamError.str());
        }
        output << "cluster_ID";
        for (const string &columnName : columnNames) {
            output << "," << columnName;
        }
        output << endl;
        for (uint32_t clusterID = 0; clusterID < scores[0].size(); clusterID++) {
            output << clusterID;
            for (uint32_t measureID = 0; measureID < scores.size(); measureID++) {
                const Ts &item = scores[measureID][clusterID];
                output << ",";
                output << extractor(item);
            }
            output << endl;
        }
        output.close();
    }

};

template<typename T, typename U>
std::ostream &operator<<(std::ostream &out, const std::pair<T, U> &v) {
    out << "<" << v.first << ", " << v.second << ">" << endl;
    return out;
}


template<typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    if (!v.empty()) {
        out << '[';
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        out << "\b\b]";
    }
    return out;
}

template<typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<vector<T>> &v) {
    if (!v.empty()) {
        for (vector<T> row : v) {
            out << "[" << row << "]" << endl;
        }
    }
    return out;
}

template<typename T, typename U, typename V>
std::ostream &operator<<(std::ostream &out, const std::map<pair<T, U>, V> &v) {
    if (!v.empty()) {
        out << '[' << endl;
        for (auto i = v.begin(); i != v.end(); ++i) {
            out << "<" << i->first.first << ", " << i->first.second << "> => " << i->second << endl;
        }
        out << "]";
    }
    return out;
}

#endif /* UTILS_H_ */
