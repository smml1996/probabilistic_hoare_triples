#ifndef POMDP_H
#define POMDP_H

#include <string>
#include <unordered_set>
#include "utils.hpp"


using namespace std;

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
        bool operator==(const POMDPAction &other) const;
};

// Custom hash
struct POMDPActionHash {
    std::size_t operator()(const shared_ptr<POMDPAction> &action) const;
};

struct POMDPActionPtrEqual {
    bool operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const;
};

class POMDP {

public:
    vector<shared_ptr<POMDPVertex>>states;
    unordered_set<int> observations;
    vector<shared_ptr<POMDPAction>> actions;
    [[nodiscard]] shared_ptr<POMDPAction> get_action(const string &str_a) const ;
    unordered_map<shared_ptr<POMDPVertex>, unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>,
    MyFloat, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash,
    POMDPVertexPtrEqual> transition_matrix;
    unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>
    obs_transitions;
    POMDP() = default;
    ~POMDP();
    void print_pomdp() const;
    void add_transition(const shared_ptr<POMDPAction> &p_action, const int &from_vertex, const int &to_vertex, const double &prob_);
    void add_obs_transition(const shared_ptr<POMDPAction> &p_action, const int &to_vertex, const int &obs, const double &prob_);
    MyFloat get_obs_prob(const shared_ptr<POMDPAction> &action,  const shared_ptr<POMDPVertex> &to_vertex, const int &obs);
};
#endif