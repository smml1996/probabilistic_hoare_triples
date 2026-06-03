#ifndef UTILS_H
#define UTILS_H

#include <complex>
#include <fstream>
#include <random>
#include <set>
#include <nlohmann/json.hpp>


using namespace std;

using ComplexMatrix = vector<std::vector<std::complex<double>>>;
using json = nlohmann::json;
typedef unordered_map<int, int> Embedding;

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
    friend std::ostream &operator<<(ostream& os, const MyFloat& value);
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
    bool operator<=(const MyFloat &other) const;
    bool operator>=(const MyFloat &other) const;
};

const inline static MyFloat zero(0);
const inline static MyFloat one(1);

double get_abs_tol();
double get_rel_tol();
bool is_close(const double &a, const double &b);
bool is_close(const complex<double> &a, const complex<double> &b);

double round_to(const double &value);

inline double round_to(double value, int decimals) {
    double power = pow(10.0, decimals);
    auto result = std::round(value * power) / power;
    if (is_close(result, 0)) {
        return 0;
    }
    return std::round(value * power) / power;
}

double round(const double &value);


void split_str(string const &str, const char &delim, vector<string> &out);
void split_str(string const &str, const string &delim, vector<string> &out);
void trim(std::string& s);
string matrix_to_string(const ComplexMatrix& matrix);

template <typename Container>
std::string join(const Container& container, const std::string& delimiter) {
    std::ostringstream oss;

    auto it = container.begin();
    if (it != container.end()) {
        oss << *it;
        ++it;
    }

    for (; it != container.end(); ++it) {
        oss << delimiter << *it;
    }

    return oss.str();
}

// pomdp file parser
int pf_get_num_states(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_actions(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_observations(const vector<string> &lines, const POMDPFormat &file_format);

// ABHSVI parser
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


ComplexMatrix json_to_matrix(const json &json_val);

json to_json(const ComplexMatrix& matrix);

template<typename T>
class Enumerate {
    T& iterable;

public:
    explicit Enumerate(T& iterable) : iterable(iterable) {}

    struct Iterator {
        std::size_t index;
        decltype(std::begin(std::declval<T&>())) iter;

        bool operator!=(const Iterator& other) const {
            return iter != other.iter;
        }

        void operator++() {
            ++index;
            ++iter;
        }

        auto operator*() const {
            return std::pair<std::size_t, decltype(*iter)>(index, *iter);
        }
    };

    Iterator begin() {
        return {0, std::begin(iterable)};
    }

    Iterator end() {
        return {0, std::end(iterable)};
    }
};

template<typename T>
Enumerate<T> enumerate(T& iterable) {
    return Enumerate<T>(iterable);
}

class Config {
public:
    static bool is_debug;
    static bool print_info;
};

class LOGFile {
    ofstream logfile;
    void write_ln(const string &line);
    vector<string> contexts;
public:
    LOGFile() = default;
    void open(const filesystem::path &path);
    void close();
    void add_context(const string &context);
    void pop_context();
    void write_debug_ln(const string &line);
    void write_info_ln(const string &line);

    // template <typename Container>
    // void write_debug_ln(const Container& container) {
    //     auto it = container.begin();
    //     if (it != container.end()) {
    //         this->logfile << "[DEBUG] " << *it;
    //         ++it;
    //     }
    //
    //     for (; it != container.end(); ++it) {
    //         this->logfile << " , " << *it;
    //     }
    //
    // }
    //
    // template <typename Container>
    // void write_info_ln(const Container& container) {
    //     auto it = container.begin();
    //     if (it != container.end()) {
    //         this->logfile << "[IN] " << *it;
    //         ++it;
    //     }
    //
    //     for (; it != container.end(); ++it) {
    //         this->logfile << " , " << *it;
    //     }
    // }

};
extern LOGFile LOG;
#endif
