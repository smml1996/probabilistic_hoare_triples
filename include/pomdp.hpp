#ifndef POMDP_H
#define POMDP_H

#include <filesystem>
#include <string>
#include <unordered_set>
#include "utils.hpp"


using namespace std;

inline auto abhsvi_benchmarks_path = filesystem::path("..") / "AB-HSVI_NeurIPS_2025" / "Models";
inline auto f1_benchmarks_path = filesystem::path("..") / "benchmarks" ;

class POMDPVertex {
    static int local_counter;
    public:
        int id;
        ~POMDPVertex();
        POMDPVertex();
        POMDPVertex(const int &id);
        bool operator==(const POMDPVertex &other) const;
        static shared_ptr<POMDPVertex> get(const int &id);
};

// Custom hash
struct POMDPVertexHash {
    std::size_t operator()(const shared_ptr<POMDPVertex> &v) const;
};

struct POMDPVertexPtrEqual {
    bool operator()(const shared_ptr<POMDPVertex> &a, const shared_ptr<POMDPVertex> &b) const;
};

class POMDPAction {
    static int local_counter;
    public:
        string name;
        int id;
        POMDPAction(const string &name="");
        POMDPAction(const int &id, const string &name);
        bool operator==(const POMDPAction &other) const;
};

// Custom hash
struct POMDPActionHash {
    std::size_t operator()(const shared_ptr<POMDPAction> &action) const;
};

struct POMDPActionPtrEqual {
    bool operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const;
};

static shared_ptr<POMDPAction> halt_action = make_shared<POMDPAction>(-1, "HALT");

class POMDP {
    POMDPFormat file_format;

    void parse_transitions(const vector<string> &lines);
    void parse_reward_function(const vector<string> &lines);
    void parse_observation_function(const vector<string> &lines);
    void parse_initial_tuples(const vector<string> &lines);
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
    ~POMDP();
    void print_pomdp() const;
    void add_transition(const shared_ptr<POMDPAction> &p_action, const int &from_vertex, const int &to_vertex, const double &prob_);
    void add_obs_transition(const shared_ptr<POMDPAction> &p_action, const int &to_vertex, const int &obs, const double &prob_);
    void add_reward(const shared_ptr<POMDPAction> &p_action, const int &v, const double &r);
    MyFloat get_obs_prob(const shared_ptr<POMDPAction> &action,  const shared_ptr<POMDPVertex> &to_vertex, const int &obs);
    MyFloat get_reward(const shared_ptr<POMDPVertex> &v, const shared_ptr<POMDPAction> &action) const;
};
#endif