#ifndef SOLVERS_H
#define SOLVERS_H

#include "pomdp.hpp"
#include "hull.hpp"
#include "Belief.hpp"
#include <chrono>
#include <memory>


using namespace std;

static shared_ptr<Belief> empty_belief = make_shared<Belief>();
namespace mp = boost::multiprecision;
using cpp_int = mp::cpp_int;

// using strategy_score_type = pair<shared_ptr<Strategy>, shared_ptr<MWP>>;

class Solver {
protected:
    bool convexify;
    chrono::time_point<chrono::steady_clock, chrono::steady_clock::duration> start_time;
    bool is_timeout;
    const MyFloat zero;
    POMDP pomdp;
    MyFloat get_reward(const shared_ptr<Belief> &b, const shared_ptr<POMDPAction> &action) const;
    map<int, shared_ptr<Belief>> get_successor_beliefs(const shared_ptr<Belief> &belief, const shared_ptr<POMDPAction> &action);
    vector<shared_ptr<Multibelief>> get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action);
    shared_ptr<MWP> get_mwp(const shared_ptr<Multibelief>&beliefs, const shared_ptr<POMDPAction> &action) const;
    shared_ptr<Hull> get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<shared_ptr<Hull>> &multibelief_points, int mb_index=0);

    double solve_lp_maximin(const int &n_initial_states, const Hull& scores);

    void check_time();
public:
    double running_time;
    static long long timelimit; // seconds

    virtual double solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
                         const int &horizon);
    virtual double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
        const int &horizon) = 0;
};

class ParetoSolver : public Solver {
protected:
    shared_ptr<Hull> get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon);
    public:
        int final_hull_size;
        ParetoSolver(const POMDP &pomdp, const bool &convexify);
        double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) override;
};

class QInspiredSolver : public Solver {
    vector<shared_ptr<Multistate>> all_multistates;
    vector<shared_ptr<ActionKernel>> all_action_kernels;
    unordered_map<int, unordered_map<int, unordered_map<int, float>>> ms_transition_probs;
    double get_trans_prob(const int &from_m, const int &action_kernel, const int &to_m, const int &obs);
    double get_ms_reward(const shared_ptr<POMDPVertex> &v, const shared_ptr<ActionKernel> &ak);
    string get_prob_var_name(const int &ms_index, const int &ak_index, const int &obs, const int& horizon);
    string get_prob_ka_name(const int &ak_index, const int& horizon, const int &obs);
    string get_reward_var_name(const int &horizon, const int &initial_state_index);
    void helper_get_multistates(Multistate &current, const int &size, vector<shared_ptr<Multistate>>&result, const vector<shared_ptr<POMDPVertex>> &states, const int &current_index=0);
    void helper_get_kernels(ActionKernel &current, set<int>::iterator obs_it, vector<shared_ptr<ActionKernel>> &result);
    vector<shared_ptr<Multistate>> get_all_multistates(const int &size, const int &max_horizon);
    vector<shared_ptr<ActionKernel>> get_all_action_kernels();
    double solve_lp(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon);
public:
    QInspiredSolver(const POMDP &pomdp);
    double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) override;
};

#endif