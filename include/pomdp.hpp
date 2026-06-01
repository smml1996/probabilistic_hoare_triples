#ifndef POMDP_H
#define POMDP_H

#include <filesystem>
#include <string>
#include <unordered_set>
#include <map>

#include "channels.hpp"
#include "hardware_specification.hpp"
#include "hybrid_state.hpp"
#include "utils.hpp"


using namespace std;

inline auto abhsvi_benchmarks_path = filesystem::path("..") / "AB-HSVI_NeurIPS_2025" / "Models";
inline auto f1_benchmarks_path = filesystem::path("..") / "benchmarks" ;

class POMDPVertex {
    protected:
        static int local_counter;
    public:
        int id;
        POMDPVertex();
        POMDPVertex(const int &id);
        virtual bool operator==(const POMDPVertex &other) const;
        static shared_ptr<POMDPVertex> get(const int &id);
};

class QVertex : public POMDPVertex {
public:
    shared_ptr<HybridState> hybrid_state;
    QVertex(const shared_ptr<HybridState> &hybrid_state);
    bool operator==(const QVertex &other) const;
    int get_obs() const;
    shared_ptr<QuantumState> quantum_state() const;
    shared_ptr<ClassicalState> classical_state() const;
    int hidden_index() const;
};

// Custom hash
struct POMDPVertexHash {
    std::size_t operator()(const shared_ptr<POMDPVertex> &v) const;
};

struct POMDPVertexPtrEqual {
    bool operator()(const shared_ptr<POMDPVertex> &a, const shared_ptr<POMDPVertex> &b) const;
};

class POMDPAction {
    public:
        static int local_counter;
    public:
        string name;
        int id;
        POMDPAction(const string &name="");
        POMDPAction(const int &id, const string &name);
        bool operator==(const POMDPAction &other) const;
};

class QAction : public POMDPAction {
public:
    bool with_noise;
    vector<Instruction> pseudo_seq;
    vector<Instruction> instruction_sequence;

    QAction(HardwareSpecification &hw_spec, const vector<Instruction> &pseudo_instruction_seq, bool with_noise=true);

    void __handle_measure_instruction(const Instruction &instruction, const MeasurementChannel &channel, const shared_ptr<HybridState> &vertex, QEnsemble &result, bool is_meas1=true) const;

    void __handle_unitary_instruction(const Instruction &instruction, const QuantumChannel &channel, const shared_ptr<HybridState> &vertex, QEnsemble &result) const;

    void __handle_reset_instruction(const Instruction &instruction, const QuantumChannel &channel, const shared_ptr<HybridState> &vertex, QEnsemble &result, bool is_meas1=true) const;

    QEnsemble __dfs(HardwareSpecification &hardware_specification, const shared_ptr<HybridState> &vertex, const int &index_ins) const;

    QEnsemble get_successor_states(HardwareSpecification &hardware_specification, const shared_ptr<QVertex> &current_vertex) const;
};

// Custom hash
struct POMDPActionHash {
    std::size_t operator()(const shared_ptr<POMDPAction> &action) const;
};

struct POMDPActionPtrEqual {
    bool operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const;
};

static shared_ptr<POMDPAction> HALT_ACTION = make_shared<POMDPAction>(-1, "HALT");

static shared_ptr<POMDPAction> INVALID_ACTION = make_shared<POMDPAction>(-2, "INVALID");
static const shared_ptr<POMDPAction> RANDOM_BRANCH = make_shared<POMDPAction>(-3, "RANDOM_BRANCH");

class POMDP {
    POMDPFormat file_format;
    string file_name;

    void parse_transitions(const vector<string> &lines);
    void parse_reward_function(const vector<string> &lines);
    void parse_observation_function(const vector<string> &lines);
    void parse_initial_tuples(const vector<string> &lines);
    [[nodiscard]] vector<shared_ptr<POMDPVertex>> get_goal_states() const;
    vector<shared_ptr<POMDPVertex>> get_states_with_depth(vector<shared_ptr<POMDPVertex>> &start_states, const int &horizon);
    vector<shared_ptr<POMDPVertex>> get_random_initial_states(const int &n_states, const int &horizon);
    unordered_map<int, unordered_set<int>> get_bfs_distances(const shared_ptr<POMDPVertex> &initial_state);

protected:
    void normalize_transitions();
    void normalize_obs_function();


public:
    vector<shared_ptr<POMDPVertex>> initial_states;
    vector<shared_ptr<POMDPVertex>>states;
    unordered_set<int> observations;
    vector<shared_ptr<POMDPAction>> actions;
    unordered_map<shared_ptr<POMDPVertex>, unordered_map<shared_ptr<POMDPAction>, MyFloat, POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash, POMDPVertexPtrEqual> f_reward;
    [[nodiscard]] shared_ptr<POMDPAction> get_action(const string &str_a) const ;
    unordered_map<shared_ptr<POMDPVertex>, unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>,
    MyFloat, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash,
    POMDPVertexPtrEqual> transition_matrix;
    unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>
    obs_transitions;


    POMDP() = default;
    POMDP(const string &file, const POMDPFormat &file_format);
    void print_pomdp() const;
    void add_transition(const shared_ptr<POMDPAction> &p_action, const int &from_vertex, const int &to_vertex, const double &prob_);
    void add_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &from_vertex,
                        const shared_ptr<POMDPVertex> &to_vertex, const MyFloat &prob_);
    void add_obs_transition(const shared_ptr<POMDPAction> &p_action, const int &to_vertex, const int &obs, const double &prob_);
    void add_obs_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &to_vertex, const int &obs, const double &prob_);
    void add_obs_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &p_v_to, const int &obs,
        const MyFloat &prob_);
    void add_reward(const shared_ptr<POMDPAction> &p_action, const int &v, const double &r);
    void add_reward(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &v, const MyFloat &r);
    MyFloat get_obs_prob(const shared_ptr<POMDPAction> &action,  const shared_ptr<POMDPVertex> &to_vertex, const int &obs);
    [[nodiscard]] MyFloat get_reward(const shared_ptr<POMDPVertex> &v, const shared_ptr<POMDPAction> &action) const;
    [[nodiscard]] shared_ptr<POMDPVertex> get_vertex_by_id(const int &id) const;
    void normalize();
    void to_abhsvi_format(vector<int> initial_states, const int &distance, const bool &is_robot=false);
    void to_python_code(const string &pomdp_path);
    void check_transitions();
    void check_obs_function();
    void check();
    int get_reachable(const int &horizon);

};

class QPOMDP : public POMDP {
    // helpers
    shared_ptr<QVertex> get_vertex(const shared_ptr<HybridState> &new_hs);
public:
    shared_ptr<QVertex> create_new_vertex(const shared_ptr<HybridState> &hybrid_state);
};

class Strategy {
public:
    int obs;
    shared_ptr<POMDPAction> action;
    map<int, shared_ptr<Strategy>> obs_to_strategies;
    bool insert(const shared_ptr<Strategy> &strategy);
    Strategy(const shared_ptr<POMDPAction> &action, const int &obs);
    Strategy(const Strategy &strategy);
    bool operator==(Strategy &strategy);
    void normalize();
};

static const shared_ptr<Strategy> TEMP_STRATEGY  = make_shared<Strategy>(INVALID_ACTION, -1);
static const shared_ptr<Strategy> HALT_STRATEGY  = make_shared<Strategy>(HALT_ACTION, -1);

class MixedStrategy {
    void normalize();
public:
    vector<pair<shared_ptr<Strategy>, double>> value;

    MixedStrategy(const vector<pair<shared_ptr<Strategy>, double>> &values);
    bool operator==(MixedStrategy &strategy);
    int find_strategy(const shared_ptr<Strategy> &strategy, const double &prob);
    bool dump(filesystem::path path);
    bool dump_raw(filesystem::path path);
};

static const shared_ptr<MixedStrategy> NO_SOLUTION_MIX_STRAT  = make_shared<MixedStrategy>(vector<pair<shared_ptr<Strategy>, double>>{{HALT_STRATEGY, 1.0}});

#endif