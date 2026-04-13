#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "Belief.hpp"
#include "hardware_specification.hpp"
#include <filesystem>

#include "algorithm.hpp"
#include "verifier.hpp"

using namespace std;
namespace fs = std::filesystem;

enum MethodType {
    SingleDistBellman,
    SingleDistPBVI,
    ConvexDist,
    ConvexDistHull,
    MethodCount
};

string get_method_string(MethodType method);
string gate_to_string(const set<MethodType> &methods);
set<string> get_solver_methods_strings();

string gate_to_string(const MethodType &method);
MethodType str_to_method_type(const string &method);

class Experiment {
protected:
        bool with_thermalization;
        int min_horizon;

        set<MethodType> method_types;
        set<QuantumHardware> hw_list;
        unordered_map<int, bool> target_vertices;
        bool optimize;

    fs::path get_wd() const;
    bool setup_working_dir() const;


    vector<HardwareSpecification> get_hardware_specs() const;
    Belief get_initial_belief(const POMDP &pomdp) const;
    vector<shared_ptr<POMDPVertex>> get_initial_states(const POMDP &pomdp) const;

    // textbook algorithms helper functions
        static void update_classical_state(const shared_ptr<Algorithm> &algorithm,const cpp_int &classical_state);
    shared_ptr<Algorithm> build_meas_sequence(const int &total_meas, const int &write_address,
        const shared_ptr<POMDPAction> &meas_action,
        const shared_ptr<ClassicalState> &current_cstate,
        const shared_ptr<Algorithm> &on_most0,
        const shared_ptr<Algorithm> &on_most1,
        int ones_count=0,
        int zeros_count=0);

    public:
    int nqvars = -1, ncvars = -1;
    int precision;
    string name;
    bool set_hidden_index;
    int max_horizon;
    static int round_in_file;
    fs::path get_final_wd() const;
    Experiment(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        bool set_hidden_index, const set<MethodType> &method_types, const set<QuantumHardware> &hw_list, bool optimize);
    virtual ~Experiment() = default;
    Experiment() = default;

    static vector<int> get_qubits_used(const unordered_map<int, int> &embedding);
    virtual set<QuantumHardware> get_allowed_hardware() const;
    virtual void run();
    virtual void verify();
    virtual bool guard(const shared_ptr<POMDPVertex>&, const unordered_map<int, int>&, const shared_ptr<POMDPAction>&) const;
    virtual void make_setup_file() const;
    virtual string get_postcondition(const MethodType &method);

    // for an experiment we need to define at least these functions
    virtual vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const = 0;
    virtual MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) = 0;
    virtual double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) = 0;
    virtual vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const = 0;
    virtual vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const = 0;
    map<string, shared_ptr<POMDPAction>> get_actions_dictionary(HardwareSpecification &hardware_spec, const int &) const;
    virtual string get_precondition(const MethodType &method) = 0;
    virtual string get_target_postcondition(const double &threshold) = 0;

    // textbook algorithm
    virtual shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon);
};

class ReadoutNoise {
    public:
    int target;
    double success0, success1, diff, acc_err, abs_diff;
    ReadoutNoise(int target, double success0, double success1);
};

set<int> get_meas_pivot_qubits(const HardwareSpecification &hardware_spec, const int &min_indegree);


// utils for running experiments in server
void generate_experiment_file(const string& experiment_name, const string& method, int min_horizon, int max_horizon); // synthesis
void generate_all_experiments_file();
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);
fs::path get_final_wd(const string &name);

double verify_algorithm(POMDP &pomdp, Experiment &experiment, const Algorithm &algorithm, HardwareSpecification &hardware_spec,
    unordered_map<int, int> &embedding, bool is_convex, int max_horizon);
MyFloat precise_verify_algorithm(POMDP &pomdp, Experiment &experiment, const Algorithm &algorithm, unordered_map<int, int> &embedding, bool is_convex, int max_horizon);

class StatsLine {
public:
    QuantumHardware quantum_hardware;
    unordered_map<int, int> embedding;
    int embedding_index;
    int horizon;
    MethodType method;
    int algorithm_index;
    Algorithm algorithm;
    double threshold;
    StatsLine(const string &exp_name, const string &line, const unordered_map<QuantumHardware, vector<unordered_map<int, int>>> &embeddings);
};

class StatsFile {
public:
    string experiment_name;
    vector<StatsLine> stats;
    StatsFile(const string &experiment_name_, const Experiment &experiment);
};
#endif