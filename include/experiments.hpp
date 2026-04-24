#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "Belief.hpp"
#include <set>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

// inline auto benchmarks_path = fs::path("..") / "benchmarks";

inline auto results_path = fs::path("..")/ "results";

inline vector<string> f_names_pomdps = {
"RockSample_POMDP_N3_G1_K2_R18.txt",
"RockSample_POMDP_N3_G1_K3_R13.txt",
"RockSample_POMDP_N3_G1_K4_R83.txt",
"RockSample_POMDP_N3_G2_K3_R4.txt",
"RockSample_POMDP_N4_G1_K2_R33.txt",
"RockSample_POMDP_N5_G1_K2_R96.txt",
"RockSample_POMDP_N6_G1_K2_R77.txt"
};

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

std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

// dumping
void dump_pomdps();


#endif