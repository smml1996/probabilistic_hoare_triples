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

bool is_close(const complex<double> &a, const complex<double> &b) {
    const double rel_tol = get_rel_tol();
    const double abs_tol = get_abs_tol();

    const double diff = std::abs(a - b);          // |a - b|
    const double scale = std::max(std::abs(a), std::abs(b)); // max(|a|, |b|)

    return diff <= std::max(rel_tol * scale, abs_tol);
}

void split_str(string const &str, const char &delim, vector<string> &out) {
    stringstream s(str);

    string s2;

    while(getline(s, s2, delim)){
        trim(s2);
        out.push_back(s2);
    }
}

void split_str(string const &str, const string &delim, vector<string> &out) {

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

// QUANTUM
string to_binary(int basis) {
    if (basis < 0) {
        throw invalid_argument("basis must be non-negative");
    }
    if (basis == 0) return "0";
    std::string result;
    while (basis > 0) {
        int temp = (basis % 2);
        result.push_back(temp + '0');
        basis /= 2;
    }
    return result;  // already reversed because of push_back order
}

bool are_matrices_equal(const vector<vector<complex<double>>> &arr1, const vector<vector<complex<double>>> &arr2) {
    if (arr1.size() != arr2.size()) return false;

    for (int row_index = 0; row_index < arr1.size(); row_index++) {
        const auto row1 = arr1.at(row_index);
        const auto row2 = arr2.at(row_index);
        if (row1.size() != row2.size()) return false;

        for (int v_element= 0 ; v_element < row1.size(); v_element++) {
            auto v1 = row1[v_element];
            auto v2 = row2[v_element];
            if (!is_close(v1, v2)) {
                return false;
            }
        }
    }
    return true;
}

bool is_matrix_in_list(const vector<vector<complex<double>>> & matrix, const vector<vector<vector<complex<double>>>> &matrix_list) {
    for (auto m : matrix_list) {
        if (are_matrices_equal(matrix, m))
            return true;
    }

    return false;
}

vector<vector<complex<double>>> multiply_matrices(const vector<vector<complex<double>>> &left,
    const vector<vector<complex<double>>> &right) {

    // assume both are 2x2
    vector<vector<complex<double>>> result(2, vector<complex<double>>(2, {0.0, 0.0}));

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {
                result[i][j] += left[i][k] * right[k][j];
            }
        }
    }

    return result;
}

pair<double, pair<complex<double>, complex<double>>> get_kraus_matrix_probability(
    const vector<vector<complex<double>>> &matrix, const complex<double> &a0, const complex<double> &a1) {
    assert (is_close(a0*conj(a0)+ a1*conj(a1), 1.0));

    auto a = matrix[0][0];
    auto b = matrix[0][1];
    auto c = matrix[1][0];
    auto d = matrix[1][1];
    auto new_a0 = a*a0 + b*a1;
    auto new_a1 = c*a0 + d*a1;
    auto prob_ = new_a0*conj(new_a0) + new_a1*conj(new_a1);
    assert(is_close(prob_.imag(), 0));
    auto prob = prob_.real();

    if (prob > 1.0) {
        assert (is_close(prob, 1.0));
        prob = 1.0;
    }
    return {prob, make_pair(new_a0, new_a1)};
}

set<int> get_intersection(const set<int> &set1, const set<int> &set2) {
    set<int> result;
    for (auto a : set1) {
        if (set2.find(a) != set2.end()) {
            result.insert(a);
        }
    }
    return result;
}


