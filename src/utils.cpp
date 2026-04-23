#include "utils.hpp"
#include <unordered_set>
#include <boost/multiprecision/cpp_int.hpp>


int MyFloat::precision = 5;

double get_rel_tol(const int &precision) {
    return 1/(pow(10,(precision-1)));
}

MyFloat::MyFloat(const double &d) {
    this->value = round_to(d, MyFloat::precision);
}

bool MyFloat::operator==(const MyFloat &other) const {
    return is_close(this->value, other.value);
}

MyFloat MyFloat::operator+(MyFloat const &other) const {
    return MyFloat(this->value + other.value);
}

MyFloat MyFloat::operator-(MyFloat const &other) const {
    return MyFloat(this->value - other.value);
}

void MyFloat::operator+=(MyFloat const &other) {
    this->value += other.value;
    this->value = round_to(this->value, MyFloat::precision);
}

void MyFloat::operator-=(MyFloat const &other) {
    this->value -= other.value;
    this->value = round_to(this->value, MyFloat::precision);
}

MyFloat MyFloat::operator*(MyFloat const &other) const {
    return MyFloat(this->value * other.value);
}

void MyFloat::operator*=(MyFloat const &other) {
    this->value *= other.value;
    this->value = round_to(this->value, MyFloat::precision);
}

bool MyFloat::operator!=(const MyFloat &rhs) const {
    return ! (*this == rhs);
}

bool MyFloat::operator>(const MyFloat &other) const {
    return (*this != other) && (this->value > other.value);
}

bool MyFloat::operator<(const MyFloat &other) const {
    return (*this != other) && (this->value < other.value);
}


double get_rel_tol() {
    return 1/(pow(10,(MyFloat::precision-1)));
}


double get_abs_tol() {
    return 1/(pow(10,(MyFloat::precision-1)));
}

bool is_close(const double &a, const double &b) {
    double rel_tol = get_rel_tol();
    double abs_tol = get_abs_tol();
    return std::fabs(a - b) <= std::max(rel_tol * std::max(std::fabs(a), std::fabs(b)), abs_tol);
}

void split_str(string const &str, const char delim, vector<string> &out) {
    stringstream s(str);

    string s2;

    while(getline(s, s2, delim)){
        out.push_back(s2);
    }
}
