#include "utils.hpp"
#include <unordered_set>
#include <boost/multiprecision/cpp_int.hpp>

#include "pomdp.hpp"


int MyFloat::precision = 5;

double get_rel_tol(const int &precision) {
    return 1/(pow(10,(precision-1)));
}

MyFloat::MyFloat(double d) {
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

MyFloat MyFloat::operator/(MyFloat const &other) const {
    return MyFloat(this->value / other.value);
}

void MyFloat::operator/=(MyFloat const &other) {
    this->value /= other.value;
    this->value = round_to(this->value, MyFloat::precision);
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
        trim(s2);
        out.push_back(s2);
    }
}

void split_str(string const &str, const string delim, vector<string> &out) {

    size_t start = 0;
    size_t end;

    while ((end = str.find(delim, start)) != std::string::npos) {
        string temp = str.substr(start, end - start);
        trim(temp);
        out.push_back(temp);
        start = end + delim.length();
    }

    auto temp = str.substr(start);
    trim(temp);
    out.push_back(temp); // last part
}

int get_basic_abhsvi(string line) {
    trim(line);
    vector<string> tokens;
    split_str(line, ',', tokens);
    assert(tokens.size() > 0);
    return stoi(tokens[0]);
}

int pf_get_num_states(const vector<string> &lines, const POMDPFormat &file_format) {
    if (file_format == POMDPFormat::ABHSVI) {
        return get_basic_abhsvi(lines[0]);
    }
    assert(file_format == POMDPFormat::F1);
    for (auto line : lines) {
        if (line.size()>0) {
            vector<string> tokens;
            split_str(line, ' ', tokens);
            if (tokens[0] == "states:") {
                if (tokens.size() > 2) {
                    return tokens.size() -1;
                } else {
                    return stoi(tokens[1]);
                }
            }
        }
    }

    assert(false);
}

int pf_get_num_actions(const vector<string> &lines, const POMDPFormat &file_format) {
    if (file_format == POMDPFormat::ABHSVI) {
        return get_basic_abhsvi(lines[2]);
    }

    assert(file_format == POMDPFormat::F1);
    for (auto line : lines) {
        if (line.size()>0) {
            vector<string> tokens;
            split_str(line, ' ', tokens);
            if (tokens[0] == "actions:") {
                if (tokens.size() > 2) {
                    return tokens.size() -1;
                } else {
                    return stoi(tokens[1]);
                }
            }
        }
    }

    assert(false);
}

int pf_get_num_observations(const vector<string> &lines, const POMDPFormat &file_format) {
    if (file_format == POMDPFormat::ABHSVI) {
        return get_basic_abhsvi(lines[3]);
    }

    assert(file_format == POMDPFormat::F1);
    for (auto line : lines) {
        if (line.size()>0) {
            vector<string> tokens;
            split_str(line, ' ', tokens);
            if (tokens[0] == "observations:") {
                if (tokens.size() > 2) {
                    return tokens.size() -1;
                } else {
                    return stoi(tokens[1]);
                }
            }
        }
    }

    assert(false);
}

std::ostream & operator<<(ostream &os, const MyFloat &myfloat) {
    os << myfloat.value;
    return os;
}

void trim(std::string& s) {
    const std::string whitespace = " \t\n\r\f\v";

    size_t start = s.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        s.clear(); // all whitespace
        return;
    }

    size_t end = s.find_last_not_of(whitespace);
    s = s.substr(start, end - start + 1);
}

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << parts[i];
    }
    return oss.str();
}

