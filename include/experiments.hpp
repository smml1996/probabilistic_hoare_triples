#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "Belief.hpp"
#include "hardware_specification.hpp"
#include <filesystem>

#include "algorithm.hpp"


using namespace std;
namespace fs = std::filesystem;

enum MethodType {
    SingleDistBellman,
    Convex,
    Naive,
    MethodCount
};

string get_method_string(MethodType method);
string gate_to_string(const set<MethodType> &methods);
set<string> get_solver_methods_strings();

string gate_to_string(const MethodType &method);
MethodType str_to_method_type(const string &method);

class Experiment {
    const int LIMIT_NAIVE_STRATS = 100000;
protected:
        bool with_thermalization = false;
        int min_horizon = -1;

        set<MethodType> method_types;
        set<QuantumHardware> hw_list;
        unordered_map<int, bool> target_vertices;
        bool optimize = true;
        bool uses_cnot = false;
        int num_batches = 10;

    [[nodiscard]] fs::path get_wd() const;
    [[nodiscard]] bool setup_working_dir() const;


    [[nodiscard]] vector<HardwareSpecification> get_hardware_specs() const;
    [[nodiscard]] Belief get_initial_belief(const POMDP &pomdp) const;
    [[nodiscard]] vector<shared_ptr<POMDPVertex>> get_initial_states(const POMDP &pomdp) const;

    // textbook algorithms helper functions
        static void update_classical_state(const shared_ptr<Algorithm> &algorithm,const cpp_int &classical_state);
    shared_ptr<Algorithm> build_meas_sequence(const int &total_meas, const int &write_address,
        const shared_ptr<POMDPAction> &meas_action,
        const shared_ptr<ClassicalState> &current_cstate,
        const shared_ptr<Algorithm> &on_most0,
        const shared_ptr<Algorithm> &on_most1,
        int ones_count=0,
        int zeros_count=0);

    // setup
    void set_with_thermalization();
    void set_optimize();
    virtual void set_hidden_index_to();
    virtual void set_uses_cnot();
    void set_precision();
    void setup_params();
    void check_params() const;

    virtual void set_global_equality();
    virtual void set_num_batches();
    virtual void set_min_max_horizon(const MethodType &method) = 0;
    virtual void set_methods() = 0;
    virtual void set_num_vars() = 0;
    int count_naive_strats(POMDP &pomdp, Belief &current_belief, const int &horizon);
    [[nodiscard]] int get_naive_stats(const MethodType &method, POMDP &pomdp, const int &horizon);

    public:
    const static set<string> experiment_names;
        int nqvars = -1, ncvars = -1;
        int precision = -1;
        string name;
        bool set_hidden_index = false;
        int max_horizon = -1;
        static int round_in_file;
        static bool is_parse;
    [[nodiscard]] fs::path get_final_wd() const;
    Experiment(const string& name, const set<QuantumHardware> &hw_list);
    virtual ~Experiment() = default;
    Experiment() = default;

    static vector<int> get_qubits_used(const unordered_map<int, int> &embedding);
    [[nodiscard]] set<QuantumHardware> get_allowed_hardware() const;
    void run();
    void generate_script();
    void parse_results();
    double get_verify_time(const MethodType &method, POMDP &pomdp, shared_ptr<Algorithm> &algorithm, const double &actual_prob, const
                           unordered_map<int, int> &embedding);
    double verify(const MethodType &method, POMDP &pomdp, shared_ptr<Algorithm> &algorithm, const double &actual_prob, const
                  unordered_map<int, int> &embedding);
    [[nodiscard]] virtual bool guard(const shared_ptr<POMDPVertex>&, const unordered_map<int, int>&, const shared_ptr<POMDPAction>&) const;

    // for an experiment we need to define at least these functions
    virtual vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const = 0;
    virtual MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) = 0;
    virtual vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const = 0;
    [[nodiscard]] virtual vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const = 0;
    map<string, shared_ptr<POMDPAction>> get_actions_dictionary(HardwareSpecification &hardware_spec, const int &) const;
    MyFloat verify_at_belief(POMDP &pomdp, shared_ptr<Algorithm> &algorithm, const Belief &belief, const unordered_map<int, int> &
                             embedding);

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
std::string join(const std::vector<std::string>& parts, const std::string& delimiter);
fs::path get_final_wd(const string &name);

#endif