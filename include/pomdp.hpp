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
    static int local_counter;
    public:
        int id;
        POMDPVertex();
        virtual ~POMDPVertex() = default;
        POMDPVertex(const int &id);
        virtual bool operator==(const POMDPVertex &other) const;
        static shared_ptr<POMDPVertex> get(const int &id);
        virtual string str() const;
};

class QVertex final : public POMDPVertex {
public:
    shared_ptr<const HybridState> hybrid_state;
    QVertex(const shared_ptr<const HybridState> &hybrid_state);
    bool operator==(const QVertex &other) const;
    int get_obs() const;
    shared_ptr<QuantumState> quantum_state() const;
    shared_ptr<ClassicalState> classical_state() const;
    int hidden_index() const;
    string str() const override;
};

// Custom hash
struct POMDPVertexHash {
    std::size_t operator()(const shared_ptr<const POMDPVertex> &v) const;
};

struct POMDPVertexPtrEqual {
    bool operator()(const shared_ptr<const POMDPVertex> &a, const shared_ptr<const POMDPVertex> &b) const;
};

class POMDPAction {
public:
        static const shared_ptr<const POMDPAction> HALT_ACTION;
        static const shared_ptr<const POMDPAction> INVALID_ACTION;
        static const shared_ptr<const POMDPAction> RANDOM_BRANCH;
        static int local_counter;
        string name;
        int id;
        POMDPAction(const string &name="");
        virtual ~POMDPAction() = default;
        POMDPAction(const int &id, const string &name);
        bool operator==(const POMDPAction &other) const;
        bool operator!=(const POMDPAction &other) const;
        virtual string str() const;
};

class QAction : public POMDPAction {
public:
    bool with_noise;
    vector<Instruction> pseudo_seq;
    vector<Instruction> instruction_sequence;

    QAction(HardwareSpecification &hw_spec, const vector<Instruction> &pseudo_instruction_seq, bool with_noise=true);

    void __handle_measure_instruction(const Instruction &instruction, const MeasurementChannel &channel, const shared_ptr<const HybridState> &vertex, QEnsemble &result, bool is_meas1=true) const;

    void __handle_unitary_instruction(const Instruction &instruction, const QuantumChannel &channel, const shared_ptr<const HybridState> &vertex, QEnsemble &result) const;

    void __handle_reset_instruction(const Instruction &instruction, const QuantumChannel &channel, const shared_ptr<const HybridState> &vertex, QEnsemble &result, bool is_meas1=true) const;

    QEnsemble __dfs(HardwareSpecification &hardware_specification, const shared_ptr<const HybridState> &vertex, const int &index_ins) const;

    QEnsemble get_successor_states(HardwareSpecification &hardware_specification, const shared_ptr<const QVertex> &current_vertex) const;
    string str() const override;
};

// Custom hash
struct POMDPActionHash {
    std::size_t operator()(const shared_ptr<const POMDPAction> &action) const;
};

struct POMDPActionPtrEqual {
    bool operator()(const shared_ptr<const POMDPAction> &a, const shared_ptr<const POMDPAction> &b) const;
};

class POMDP {
    POMDPFormat file_format;
    string file_name;

    void parse_transitions(const vector<string> &lines);
    void parse_reward_function(const vector<string> &lines);
    void parse_observation_function(const vector<string> &lines);
    void parse_initial_tuples(const vector<string> &lines);
    [[nodiscard]] vector<shared_ptr<const POMDPVertex>> get_goal_states() const;
    vector<shared_ptr<const POMDPVertex>> get_states_with_depth(const vector<shared_ptr<const POMDPVertex>> &start_states, const int &horizon);
    vector<shared_ptr<const POMDPVertex>> get_random_initial_states(const int &n_states, const int &horizon);
    unordered_map<int, unordered_set<int>> get_bfs_distances(const shared_ptr<const POMDPVertex> &initial_state);

protected:
    void normalize_transitions();
    void normalize_obs_function();


public:
    vector<shared_ptr<const POMDPVertex>> initial_states;
    vector<shared_ptr<const POMDPVertex>>states;
    unordered_set<int> observations;
    vector<shared_ptr<const POMDPAction>> actions;
    unordered_map<shared_ptr<const POMDPVertex>, unordered_map<shared_ptr<const POMDPAction>, MyFloat, POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash, POMDPVertexPtrEqual> f_reward;
    [[nodiscard]] shared_ptr<const POMDPAction> get_action(const string &str_a) const ;
    unordered_map<shared_ptr<const POMDPVertex>, unordered_map<shared_ptr<const POMDPAction>, unordered_map<shared_ptr<const POMDPVertex>,
    MyFloat, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash,
    POMDPVertexPtrEqual> transition_matrix;
    unordered_map<shared_ptr<const POMDPAction>, unordered_map<shared_ptr<const POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>
    obs_transitions;


    POMDP() = default;
    POMDP(const string &file, const POMDPFormat &file_format);
    void print_pomdp() const;
    void print_reward_f() const;
    void add_transition(const shared_ptr<const POMDPAction> &p_action, const int &from_vertex, const int &to_vertex, const double &prob_);
    void add_transition(const shared_ptr<const POMDPAction> &p_action, const shared_ptr<const POMDPVertex> &from_vertex,
                        const shared_ptr<const POMDPVertex> &to_vertex, const MyFloat &prob_);
    void add_obs_transition(const shared_ptr<const POMDPAction> &p_action, const int &to_vertex, const int &obs, const double &prob_);
    void add_obs_transition(const shared_ptr<const POMDPAction> &p_action, const shared_ptr<const POMDPVertex> &to_vertex, const int &obs, const double &prob_);
    void add_obs_transition(const shared_ptr<const POMDPAction> &p_action, const shared_ptr<const POMDPVertex> &p_v_to, const int &obs,
        const MyFloat &prob_);
    void add_reward(const shared_ptr<const POMDPAction> &p_action, const int &v, const double &r);
    void add_reward(const shared_ptr<const POMDPAction> &p_action, const shared_ptr<const POMDPVertex> &v, const MyFloat &r);
    MyFloat get_obs_prob(const shared_ptr<const POMDPAction> &action,  const shared_ptr<const POMDPVertex> &to_vertex, const int &obs);
    [[nodiscard]] MyFloat get_reward(const shared_ptr<const POMDPVertex> &v, const shared_ptr<const POMDPAction> &action) const;
    [[nodiscard]] shared_ptr<const POMDPVertex> get_vertex_by_id(const int &id) const;
    void normalize();
    void to_abhsvi_format(vector<int> initial_states_, const int &distance, const bool &is_robot=false);
    void to_python_code(const string &pomdp_path);
    void check_transitions();
    void check_obs_function();
    void check();
    int get_reachable(const int &horizon);

};

class QPOMDP : public POMDP {
    // helpers
    shared_ptr<const QVertex> get_vertex(const shared_ptr<const HybridState> &new_hs) const;
public:
    shared_ptr<const QVertex> create_new_vertex(const shared_ptr<const HybridState> &hybrid_state);
};

class Strategy {
public:
    static const bool simplify_comp;
    static const shared_ptr<const Strategy> HALT_STRATEGY;
    static const shared_ptr<const Strategy> TEMP_STRATEGY;
    static shared_ptr<Strategy> get_temp_strategy();
    int obs;
    shared_ptr<const POMDPAction> action;
    map<int, shared_ptr<Strategy>> obs_to_strategies;
    bool insert(const shared_ptr<Strategy> &strategy);
    Strategy(const shared_ptr<const POMDPAction> &action, const int &obs);
    Strategy(const Strategy &strategy);
    bool operator==(const Strategy &strategy) const;
    void normalize();
};



class MixedStrategy {
public:
    static const shared_ptr<MixedStrategy> NO_SOLUTION_MIX_STRAT;
    vector<pair<shared_ptr<Strategy>, double>> value;
    MixedStrategy(const vector<pair<shared_ptr<Strategy>, double>> &values);
    bool operator==(const MixedStrategy &strategy) const;
    int find_strategy(const shared_ptr<Strategy> &strategy, const double &prob) const;
    bool dump(const filesystem::path &path) const;
    bool dump_raw(const filesystem::path &path) const;
    void normalize(const bool &only_probs=false);
};

string to_string(const Strategy &algorithm, const string &tabs="");
string to_string(const MixedStrategy &algorithm, string tabs ="");
#endif