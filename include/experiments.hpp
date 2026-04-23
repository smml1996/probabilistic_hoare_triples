#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "Belief.hpp"
#include <set>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

inline auto benchmarks_path = fs::path("..") / "benchmarks";

enum MethodType {
    Pareto,
    ConvexPareto,
    Pspace,
    MethodCount
};

string methods_to_string(const set<MethodType> &methods);
set<string> get_solver_methods_strings();

string method_to_string(const MethodType &method);
MethodType str_to_method_type(const string &method);

class Benchmark {
    string file;
    POMDP pomdp;
protected:
    public:
    static int round_in_file;
    Benchmark(const string &file);
};


// utils for running experiments in server
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

#endif