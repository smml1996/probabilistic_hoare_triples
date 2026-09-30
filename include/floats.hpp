//
// Created by Stefanie Muroya Lei on 28.01.24.
//
#ifndef FLOATS_H
#define FLOATS_H
#include <vector>
#include <string>

using namespace std;

class MyFloat {
    public:
    static int precision;
    static int tolerance;

    vector<short> exponent;
    vector<short> mantissa;
    bool is_negative = false;

    MyFloat(const double &d_prob, int custom_precision);
    explicit MyFloat(const string& s_prob = "0", int custom_precision=-1);

    static void check_digit(const short &digit);

    static string remove_initial_zeros(const string &original);

    static vector<short> integer_addition(const vector<short> &v1, const vector<short> &v2);

    static pair<short, short> digit_substraction(const short &s1, const short &s2, const short &carry);

    static MyFloat subtraction(const MyFloat& f1, const MyFloat& f2);
    
    static bool are_vectors_equal(const vector<short> &v1, const vector<short> &v2);

    static bool is_vector_greater(vector<short> v1, vector<short> v2);

    static MyFloat abs(MyFloat const &n);

    friend std::ostream &operator<<(ostream& os, const MyFloat& myfloat);

    MyFloat operator+(MyFloat const &other) const;

    MyFloat operator*(MyFloat const &other) const;

    bool operator==(const MyFloat &rhs) const;

    bool operator!=(const MyFloat &rhs) const;

    bool operator>(const MyFloat &other) const;

    bool operator<(const MyFloat &other) const;
};

MyFloat max(MyFloat const &a, MyFloat const &b);

MyFloat min(MyFloat const &a, MyFloat const &b);
MyFloat abs(const MyFloat &f);

std::string to_string(const MyFloat& myfloat);

double to_double(const MyFloat &myfloat);
#endif