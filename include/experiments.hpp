#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "Belief.hpp"
#include "hybrid_state.hpp"
#include <set>
#include <filesystem>
#include "hardware_specification.hpp"
using namespace std;
namespace fs = std::filesystem;

// inline auto abhsvi_benchmarks_path = fs::path("..") / "benchmarks";

inline auto results_path = fs::path("..")/ "results";

inline int round_in_file = 6;

inline vector<string> abhsvi_pomdps = {
"RockSample_POMDP_N3_G1_K2_R18.txt",
"RockSample_POMDP_N3_G1_K3_R13.txt",
"RockSample_POMDP_N3_G1_K4_R83.txt",
"RockSample_POMDP_N3_G2_K3_R4.txt",
"RockSample_POMDP_N4_G1_K2_R33.txt",
"RockSample_POMDP_N5_G1_K2_R96.txt",
"RockSample_POMDP_N6_G1_K2_R77.txt"
};

inline vector<string> f1_pomdps = {
    "iff.POMDP",
    "cit.POMDP",
    "mit.POMDP",
    "pentagon.POMDP",
    "sunysb.POMDP"
};

inline vector<int> horizons = {1, 2, 3, 4, 5, 6, 7};
inline vector<int> f1_horizons = {1, 2, 3, 4, 5, 6};


enum MethodType {
    Pareto,
    ConvexPareto,
    MethodCount
};

string to_string(const set<MethodType> &methods);
set<string> get_solver_methods_strings();

string method_to_string(const MethodType &method);
MethodType str_to_method_type(const string &method);

// dumping
vector<string> get_final_f1_pomdp_names();
int get_pomdp_horizon(const string &pomdp_name);
vector<string> get_all_pomdp_names();
void dump_pomdps();

void run_experiments(const MethodType &method);
void run_exp_more_rocks(const MethodType &method);
void f1_run_experiments(const MethodType &method, const string &pomdp_name, const int &max_horizon=6);
void run_convexify_sizes_experiment(const string &pomdp_name);

void generate_f1_benchmarks();

void pomdps_to_python();

// Quantum experiments
class ReadoutNoise {
public:
    int target;
    double success0, success1, diff, acc_err, abs_diff;
    ReadoutNoise(int target, double success0, double success1);
};

set<int> get_meas_pivot_qubits(const HardwareSpecification &hardware_spec, const int &min_indegree);

class QuantumExperiment {
    static long long timelimit;

    // timeout for POMDP building
    bool is_timeout = false;
    chrono::time_point<chrono::steady_clock, chrono::steady_clock::duration> start_time;
    void check_time();

    fs::path get_wd() const;
    bool clean_wd() const;
    bool setup_working_dir(const bool &clean_wd=true) const;

    int get_or_add_algorithm(const vector<shared_ptr<MixedStrategy>> &unique_algorithms, shared_ptr<MixedStrategy> &algorithm);
    POMDP build_pomdp(HardwareSpecification &hardware_specification, const vector<shared_ptr<QAction>> &actions); // normalize hardware specification according to embedding

protected:
    vector<int> qubits_used;

    vector<int> get_unused(const Embedding &embedding, const int &n) const;
    vector<int> get_unused(unordered_set<int> used_qubits, const int &n) const;
    shared_ptr<QuantumState> get_choi_id_state(const vector<pair<int, int>> &qubit_pairs) const;

    virtual bool guard(const shared_ptr<QVertex>&, const shared_ptr<QAction>&) const;
    virtual void init();
    virtual void set_method_types();
    virtual void set_hardware_specs();
    virtual void set_thermalization();
    virtual void set_optimize();
    virtual void set_horizons();
    virtual void set_precision();

    // mandatory to define this on children class
    virtual void set_experiment_name() = 0;
    virtual vector<shared_ptr<QAction>> get_actions(HardwareSpecification &hardware_specification) = 0;
    virtual vector<Embedding> get_embeddings(const HardwareSpecification &hw) const = 0;
    virtual MyFloat get_reward(shared_ptr<QVertex> &v) const = 0;
    virtual vector<shared_ptr<HybridState>> get_initial_states() = 0;
    virtual void set_qubits_used() = 0;
public:
    string name;
    set<MethodType> method_types;
    vector<HardwareSpecification> hw_list;
    int min_horizon;
    int max_horizon;
    bool with_thermalization;
    bool optimize;

    QuantumExperiment() = default;
    void run();
    void dump_preview();
};
#endif