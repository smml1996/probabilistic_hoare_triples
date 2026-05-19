#ifndef UTILS_H
#define UTILS_H

#include <complex>
#include <random>
#include <set>


using namespace std;

enum POMDPFormat {
    F1, // https://www.pomdp.org/code/pomdp-file-spec.html
    ABHSVI // https://arxiv.org/abs/2510.23744
};

class MyFloat {
public:
    double value;
    static int precision;
    MyFloat(double d);
    MyFloat() {this->value = 0;};
    friend std::ostream &operator<<(ostream& os, const MyFloat& myfloat);
    bool operator==(const MyFloat &other) const;
    MyFloat operator+(MyFloat const &other) const;
    MyFloat operator-(MyFloat const &other) const;
    void operator+=(MyFloat const &other);
    void operator-=(MyFloat const &other);
    MyFloat operator*(MyFloat const &other) const;
    MyFloat operator/(MyFloat const &other) const;
    void operator/=(MyFloat const &other);
    void operator*=(MyFloat const &other);
    bool operator!=(const MyFloat &rhs) const;
    bool operator>(const MyFloat &other) const;
    bool operator<(const MyFloat &other) const;
};

static MyFloat zero(0);
static MyFloat one(1);

double get_abs_tol();
double get_rel_tol();
bool is_close(const double &a, const double &b);
bool is_close(const complex<double> &a, const complex<double> &b);

double round_to(double value, int decimals);

inline double round_to(double value, int decimals) {
    double power = pow(10.0, decimals);
    auto result = std::round(value * power) / power;
    if (is_close(result, 0)) {
        return 0;
    }
    return std::round(value * power) / power;
}


void split_str(string const &str, const char &delim, vector<string> &out);
void split_str(string const &str, const string &delim, vector<string> &out);
void trim(std::string& s);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// pomdp file parser
int pf_get_num_states(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_actions(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_observations(const vector<string> &lines, const POMDPFormat &file_format);

// AB-HSVI parser
int get_basic_abhsvi(string line);


// QUANTUM utils
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

string to_binary(int basis);
bool are_matrices_equal(const vector<vector<complex<double>>> &arr1, const vector<vector<complex<double>>> &arr2);
bool is_matrix_in_list(const vector<vector<complex<double>>> & matrix, const vector<vector<vector<complex<double>>>> &matrix_list);
vector<vector<complex<double>>> multiply_matrices(const vector<vector<complex<double>>> &left, const vector<vector<complex<double>>> &right);

pair<double, pair<complex<double>, complex<double>>> get_kraus_matrix_probability(const vector<vector<complex<double>>> &matrix, const complex<double> &a, const complex<double> &b);
set<int> get_intersection(const set<int> &set1, const set<int> &set2);
#endif
