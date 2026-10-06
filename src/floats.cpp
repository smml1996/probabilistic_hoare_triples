//
// Created by Stefanie Muroya Lei on 28.01.24.
//
#include "floats.hpp"
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;

int MyFloat::precision = 80;
int MyFloat::tolerance = 80;

MyFloat::MyFloat(const double &probability__, int custom_precision)  : MyFloat(to_string(probability__), custom_precision) {};

void MyFloat::check_digit(const short &digit) {
    if (digit < -0 || digit > 9) {
        throw invalid_argument("digit can only be a number between 0 and 9");
    }
}

string MyFloat::remove_initial_zeros(const string &original) {
    string result;
    bool found_non_zero = false;
    bool dot_found = false;
    for(auto c : original) {

        if (dot_found or found_non_zero) {
            result += c;
        }else if (c == '.'){
            dot_found = true;
            if (result.empty()) {
                result += '0';
            }
            result += c;
        } else if(c != '0') {
            found_non_zero = true;
            result += c;
        }
    }
    if (result.empty()){
        result += '0';
    }
    return result;
}

vector<short> MyFloat::integer_addition(const vector<short> &v1, const vector<short> &v2) {
    vector<short> result;
    assert(v1.size() == v2.size());

    int carry = 0;
    for(int i = 0; i < v1.size(); i++) {
        short val = static_cast<short>(v1[i] + v2[i] + carry);
        result.push_back(static_cast<short>(val % 10));
        carry = val > 9 ? 1 : 0;
    }

    assert(carry == 0);
    return result;
}

pair<short, short> MyFloat::digit_substraction(const short &s1, const short &s2, const short &carry){
    MyFloat::check_digit(s1);
    MyFloat::check_digit(s2);
    short result;
    short carry_r = carry;
    if (carry == 0) {
        if (s1 >= s2) {
            result = s1 - s2;
        } else {
            carry_r = 1;
            result = s1 + 10 - s2;
        }
    } else {
        if(s1 > 0) {
            short v = s1 - carry;
            carry_r = 0;
            if (v >= s2) {
                result = v-s2;
            } else {
                carry_r = 1;
                result = v + 10 - s2;
            }
        } else {
            // carry does not change
            result = 10 - s2;
        }
    }
    return make_pair(result, carry_r);
}

MyFloat MyFloat::subtraction(const MyFloat& f1, const MyFloat& f2) {
    MyFloat result;
    result.mantissa.clear();
    result.exponent.clear();
    MyFloat abs_f1 = MyFloat::abs(f1);
    MyFloat abs_f2 = MyFloat::abs(f2);

    if (abs_f1 == abs_f2) {
        return MyFloat("0", max(abs_f1.precision, abs_f2.precision));
    }
    MyFloat small_f = min(abs_f1, abs_f2);
    MyFloat big_f = max(abs_f1, abs_f2);

    assert(small_f != big_f);
    assert(small_f.mantissa.size() == big_f.mantissa.size());
    if (f1.is_negative) {
        if (abs_f1 < abs_f2) {
            result.is_negative = false;
        } else {
            result.is_negative = true;
        }
    } else {
        if (abs_f1 < abs_f2) {
            result.is_negative = true;
        } else {
            result.is_negative = false;
        }
    }

    short carry = 0;
    for(int i = 0; i < big_f.mantissa.size(); i++) {

        assert(carry == 0 || carry == 1);

        pair<short, short> digit_result = MyFloat::digit_substraction(big_f.mantissa[i], small_f.mantissa[i], carry);

        result.mantissa.push_back(digit_result.first);
        carry = digit_result.second;
    }

    for (int i = 0; i < max(big_f.exponent.size(), small_f.exponent.size()); i++) {
        short s1, s2;
        if (i < big_f.exponent.size()) {
            s1 = big_f.exponent[i];
            if (i < small_f.exponent.size()) {
                // we consider both
                s2 = small_f.exponent[i];
            } else {
                // we just consider this->
                s2 = 0;
            }
        } else {
            // we just consider other.
            assert(small_f.exponent[i] == 0);
            s1 = 0;
            s2 = 0;
        }

        pair<short, short> digit_result = MyFloat::digit_substraction(s1, s2, carry);

        result.exponent.push_back(digit_result.first);
        carry = digit_result.second;
    }


    assert(carry == 0);
    return result;
}


bool MyFloat::are_vectors_equal(const vector<short> &v1, const vector<short> &v2) {
    int index = 0;
    while ((index < v1.size()) && (index < v2.size())) {
        if (v1[index] != v2[index]){
            return false;
        }
        index++;
    }

    while (index < v1.size()) {
        if (v1[index] != 0) {
            return false;
        }
        index++;
    }

    while (index < v2.size()) {
        if(v2[index] != 0)  {
            return false;
        }
        index++;
    }
    return true;
}

bool MyFloat::is_vector_greater(vector<short> v1, vector<short> v2) {
    while (v1.size() > v2.size()) {
        v2.push_back(0);
    }

    while (v2.size() > v1.size()) {
        v1.push_back(0);
    }

    assert(v1.size() == v2.size());

    // we have to check starting from the most significant digit
    for(int i = v1.size()-1; i >= 0; i--) {
        if(v1[i] > v2[i]) {
            return true;

        }else if(v1[i] < v2[i]) {
            return false;
        }
    }

    assert(MyFloat::are_vectors_equal(v1, v2));
    return false;
}

MyFloat MyFloat::abs(MyFloat const &n) {
    string s;


    for (short digit : n.mantissa) {
        s += to_string(digit);
    }
    s +=".";
    for(short digit : n.exponent) {
        s += to_string(digit);
    }

    reverse(s.begin(), s.end());
    return MyFloat(s, n.precision);
}

ostream& operator<<(ostream& os, const MyFloat& myfloat) {
    if (myfloat.is_negative) {
        os << "-";
    }

    // Print exponent digits (most significant first)
    for (int i = myfloat.exponent.size() - 1; i >= 0; --i) {
        os << myfloat.exponent[i]; // no need for to_string if it's int < 10
    }

    if (!myfloat.mantissa.empty()) {
        os << ".";
        // Print mantissa digits (most significant first)
        for (int i = myfloat.mantissa.size() - 1; i >= 0; --i) {
            os << myfloat.mantissa[i];
        }
    }

    return os;
}

MyFloat::MyFloat(const string& probability__, int custom_precision){
    string probability_;
    if (probability__[0] == '-') {
        this->is_negative = true;
        probability_ = probability__.substr(1);
    } else {
        this->is_negative = false;
        probability_ = probability__;
    }
    string probability = MyFloat::remove_initial_zeros(probability_);
    int actual_precision = custom_precision == -1 ? MyFloat::precision : custom_precision;
    this->precision = actual_precision;
    assert(!probability.empty());
    bool dot_found = false;
    for (auto c : probability) {
        assert((c == '.') or (c >= '0' and c <= '9'));
        if(c == '.') {
            dot_found = true;
        }else if (!dot_found) {
            this->exponent.push_back(static_cast<short>(c - '0'));
        } else if (this->mantissa.size() < actual_precision){
            this->mantissa.push_back(static_cast<short>(c - '0'));
        }
    }

    while (this->mantissa.size() < actual_precision) {
        this->mantissa.push_back(0);
    }
    reverse(this->exponent.begin(), this->exponent.end());
    reverse(this->mantissa.begin(), this->mantissa.end()); // operations need to start from the least significant decimal
}

MyFloat MyFloat::operator+(MyFloat const &other) const {

    if (this->is_negative != other.is_negative) {
        return MyFloat::subtraction(*this, other);
    }
    MyFloat result;
    result.mantissa.clear();
    result.exponent.clear();
    short carry = 0;
    assert(this->mantissa.size() == other.mantissa.size());
    for(int i = 0; i < this->mantissa.size(); i++) {
        MyFloat::check_digit(this->mantissa[i]);
        MyFloat::check_digit(other.mantissa[i]);
        const auto val = static_cast<short>(carry + this->mantissa[i] + other.mantissa[i]);
        result.mantissa.push_back(static_cast<short>(val % 10));
        if (val > 9) {
            carry = 1;
        } else {
            carry = 0;
        }
    }

    for (int i = 0; i < max(other.exponent.size(), this->exponent.size()); i++) {
        short val;
        if (i < this->exponent.size()) {
            MyFloat::check_digit(this->exponent[i]);
            if (i < other.exponent.size()) {
                // we consider both
                MyFloat::check_digit(other.exponent[i]);
                val = static_cast<short>(other.exponent[i] + this->exponent[i] + carry);
            } else {
                // we just consider this->
                val = static_cast<short>(this->exponent[i] + carry);
            }
        } else {
            // we just consider other.
            MyFloat::check_digit(other.exponent[i]);
            val = static_cast<short>(other.exponent[i] + carry);
        }
        result.exponent.push_back(val % 10);

        if (val > 9) {
            carry = 1;
        } else {
            carry = 0;
        }
    }

    if (carry > 0) {
        result.exponent.push_back(carry);
    }


    result.is_negative = this->is_negative;


    return result;
}

MyFloat MyFloat::operator*(MyFloat const &other) const {
    // We treat it as integer multiplication
    vector<short> n1;
    vector<short> n2;
    vector<short> result;

    assert(other.mantissa.size() == this->mantissa.size());

    for(int i = 0; i < other.mantissa.size(); i++){
        n1.push_back(this->mantissa[i]);
        n2.push_back(other.mantissa[i]);
    }

    for(int i = 0; i < this->exponent.size(); i++) {
        n1.push_back(this->exponent[i]);
    }

    for(int i = 0; i < other.exponent.size(); i++) {
        n2.push_back(other.exponent[i]);
    }

    while(n1.size() < n2.size()) {
        n1.push_back(0);
    }

    while(n2.size() < n1.size()) {
        n2.push_back(0);
    }

    while(result.size() < 2*n1.size()) {
        result.push_back(0);
    }

    // actual multiplication
    for(int shift_digit = 0; shift_digit < n2.size(); shift_digit++) {
        // perform multiplication of n1 times the digit at position shift_digit in n2
        vector<short> n1_times_digit;

        n1_times_digit.reserve(shift_digit);
        for(int j = 0 ; j < shift_digit; j++) {
            n1_times_digit.push_back(0);
        }

        int carry = 0;

        for (auto d : n1) {
            int val = d * n2[shift_digit] + carry;
            n1_times_digit.push_back(static_cast<short>(val % 10));
            carry = val / 10;
        }
        n1_times_digit.push_back(static_cast<short>(carry));
        assert(n1_times_digit.size() <= 2*n1.size());

        while(n1_times_digit.size() < 2*n1.size()) {
            n1_times_digit.push_back(0);
        }
        result = MyFloat::integer_addition(n1_times_digit, result);
    }

    // convert result to string. Note: by creating a float it trims it to the actual precision
    string res;
    for(int i = 0; i < 2*this->mantissa.size(); i++) {
        res+= '0' + result[i];
    }
    res += ".";

    for(int i = 2*this->mantissa.size(); i < result.size(); i++) {
        res += '0' + result[i];
    }

    if (this->is_negative != other.is_negative) {
        // if the numbers have diff. signs
        res += "-";
    }
    reverse(res.begin(), res.end());
    // res = MyFloat::remove_initial_zeros(res);
    return MyFloat(res, this->precision);
}

bool MyFloat::operator==(const MyFloat &rhs) const {
    if (this->is_negative != rhs.is_negative) {
        return false;
    }

    // check exponent
    if(!MyFloat::are_vectors_equal(this->exponent, rhs.exponent)) {
        return false;
    }
    return MyFloat::are_vectors_equal(this->mantissa, rhs.mantissa);
}

bool MyFloat::operator!=(const MyFloat &rhs) const {
    return !(*this == rhs);
}

inline bool MyFloat::operator>(const MyFloat &other) const {
    if (other.is_negative != this->is_negative) {
        if (!this->is_negative) {
            return true;
        } else {
            return false;
        }
    }

    // at this point we know both operands have same sign

    if (!MyFloat::are_vectors_equal(this->exponent, other.exponent)) {
        if (this->is_negative) {
            return MyFloat::is_vector_greater(other.exponent, this->exponent);
        }else {
            return MyFloat::is_vector_greater(this->exponent, other.exponent);
        }
    } else if (!MyFloat::are_vectors_equal(this->mantissa, other.mantissa)){
        if (this->is_negative) {
            return MyFloat::is_vector_greater(other.mantissa, this->mantissa);
        }else {
            return MyFloat::is_vector_greater(this->mantissa, other.mantissa);
        }
    }


    // they are both equal
    return false;
}

inline bool MyFloat::operator<(const MyFloat &other) const {
    return !(*this > other) && !(*this == other);
}

MyFloat max(MyFloat const &a, MyFloat const &b) {
    if(b > a) {
        return b;
    }
    return a;
}

MyFloat min(MyFloat const &a, MyFloat const &b) {
    if (b > a) {
        return a;
    }
    return b;
}

MyFloat abs(const MyFloat &f) {
    auto result = MyFloat(to_string(f), f.precision);
    result.is_negative = false;
    return result;
}

std::string to_string(const MyFloat& myfloat) {
    std::ostringstream oss;
    oss << myfloat;  // uses your operator<<
    return oss.str();
}

double to_double(const MyFloat &myfloat) {
    return stod(to_string(myfloat));
}
