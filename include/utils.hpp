#ifndef UTILS_H
#define UTILS_H

#include <complex>
#include <random>
#include <set>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;

double get_abs_tol(const int &precision);
double get_rel_tol(const int &precision);
bool is_close(const double &a, const double &b, const int &precision);
bool is_close(const complex<double> &a, const complex<double> &b, const int &precision);
template <typename T>
T weighted_choice(const std::vector<T> &elements, const std::vector<double> &weights) {
    if (elements.size() != weights.size() || elements.empty()) {
        throw std::invalid_argument("Elements and weights must have the same non-zero size.");
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());

    discrete_distribution<> dist(weights.begin(), weights.end());
    return elements[dist(gen)];
}
double round_to(double value, int decimals);
string to_binary(boost::multiprecision::cpp_int basis);
bool are_matrices_equal(const vector<vector<complex<double>>> &arr1, const vector<vector<complex<double>>> &arr2, int precision);
bool is_matrix_in_list(const vector<vector<complex<double>>> & matrix, const vector<vector<vector<complex<double>>>> &matrix_list, int precision);

inline double round_to(double value, int decimals) {
    double power = pow(10.0, decimals);
    return std::round(value * power) / power;
}

vector<vector<complex<double>>> multiply_matrices(const vector<vector<complex<double>>> &left, const vector<vector<complex<double>>> &right);

pair<double, pair<complex<double>, complex<double>>> get_kraus_matrix_probability(const vector<vector<complex<double>>> &matrix, const complex<double> &a, const complex<double> &b);
set<int> get_intersection(const set<int>& set1, set<int> set2);
void split_str(string const &str, const char &delim, vector<string> &out);
#endif
