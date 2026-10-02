#include "utils.hpp"

#include <set>
#include <unordered_set>
#include <boost/multiprecision/cpp_int.hpp>

double get_rel_tol(const int &precision) {
    return 1/(pow(10,(precision-1)));
}

double get_abs_tol(const int &precision) {
    return 1/(pow(10,(precision-1)));
}

bool is_close(const double &a, const double &b, const int &precision) {
    double rel_tol = get_rel_tol(precision);
    double abs_tol = get_abs_tol(precision);
    return std::fabs(a - b) <= std::max(rel_tol * std::max(std::fabs(a), std::fabs(b)), abs_tol);
}

bool is_close(const complex<double> &a, const complex<double> &b, const int &precision) {
    return is_close(a.real(), b.real(), precision) && is_close(a.imag(), b.imag(), precision);
}

// helper: convert integer to binary string (no leading zeros)
string to_binary(boost::multiprecision::cpp_int basis) {
    if (basis < 0) {
        throw invalid_argument("basis must be non-negative");
    }
    if (basis == 0) return "0";
    std::string result;
    while (basis > 0) {
        int temp = (basis % 2).convert_to<int>();
        result.push_back(temp + '0');
        basis /= 2;
    }
    return result;  // already reversed because of push_back order
}

bool are_matrices_equal(const vector<vector<complex<double>>> &arr1, const vector<vector<complex<double>>> &arr2, int precision) {
    if (arr1.size() != arr2.size()) return false;
    
    for (int row_index = 0; row_index < arr1.size(); row_index++) {
        const auto& row1 = arr1.at(row_index);
        const auto& row2 = arr2.at(row_index);
        if (row1.size() != row2.size()) return false;

        for (int v_element= 0 ; v_element < row1.size(); v_element++) {
            auto v1 = row1[v_element];
            auto v2 = row2[v_element];
            if (!is_close(v1, v2, precision)) {
                return false;
            }
        }
    }
    return true;
}

bool is_matrix_in_list(const vector<vector<complex<double>>> & matrix, const vector<vector<vector<complex<double>>>> &matrix_list, int precision) {
    for (const auto& m : matrix_list) {
        if (are_matrices_equal(matrix, m, precision))
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
    assert (is_close(a0*conj(a0)+ a1*conj(a1), 1.0, 10));

    auto a = matrix[0][0];
    auto b = matrix[0][1];
    auto c = matrix[1][0];
    auto d = matrix[1][1];
    auto new_a0 = a*a0 + b*a1;
    auto new_a1 = c*a0 + d*a1;
    auto prob_ = new_a0*conj(new_a0) + new_a1*conj(new_a1);
    assert(is_close(prob_.imag(), 0, 10));
    auto prob = prob_.real();

    if (prob > 1.0) {
        assert (is_close(prob, 1.0, 10));
        prob = 1.0;
    }
    return {prob, make_pair(new_a0, new_a1)};
}

set<int> get_intersection(const set<int>& set1, set<int> set2) {
    set<int> result;
    for (auto a : set1) {
        if (set2.find(a) != set2.end()) {
            result.insert(a);
        }
    }
    return result;
}

void split_str(string const &str, const char &delim, vector<string> &out) {
    stringstream s(str);

    string s2;

    while(getline(s, s2, delim)){
        out.push_back(s2);
    }
}
