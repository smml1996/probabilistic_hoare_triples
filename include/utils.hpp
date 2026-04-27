#ifndef UTILS_H
#define UTILS_H

#include <complex>
#include <random>
#include <boost/multiprecision/cpp_int.hpp>

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

double round_to(double value, int decimals);

inline double round_to(double value, int decimals) {
    double power = pow(10.0, decimals);
    auto result = std::round(value * power) / power;
    if (is_close(result, 0)) {
        return 0;
    }
    return std::round(value * power) / power;
}


void split_str(string const &str, const char delim, vector<string> &out);
void split_str(string const &str, const string delim, vector<string> &out);
void trim(std::string& s);
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// pomdp file parser
int pf_get_num_states(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_actions(const vector<string> &lines, const POMDPFormat &file_format);
int pf_get_num_observations(const vector<string> &lines, const POMDPFormat &file_format);

// AB-HSVI parser
int get_basic_abhsvi(string line);
#endif
