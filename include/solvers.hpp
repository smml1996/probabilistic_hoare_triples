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

class Solver {
protected:
    bool convexify = false;
    chrono::time_point<chrono::steady_clock, chrono::steady_clock::duration> start_time;
    bool is_timeout = false;
    const MyFloat zero;
    POMDP pomdp = POMDP();
    MyFloat get_reward(const shared_ptr<Belief> &b, const shared_ptr<const POMDPAction> &action) const;
    map<int, shared_ptr<Belief>> get_successor_beliefs(const shared_ptr<Belief> &belief, const shared_ptr<const POMDPAction> &action);
    vector<shared_ptr<Multibelief>> get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<const POMDPAction> &action);
    shared_ptr<MWP> get_mwp(const shared_ptr<Multibelief>&beliefs, const shared_ptr<const POMDPAction> &action) const;
    shared_ptr<Hull> get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<shared_ptr<Hull>> &multibelief_points, int mb_index=0);

    pair<shared_ptr<MixedStrategy>, double> solve_lp_maximin(const int &n_initial_states, const Hull& scores) const;

    void check_time();
public:
    virtual ~Solver() = default;

    double running_time;
    static long long timelimit; // seconds
    virtual pair<shared_ptr<MixedStrategy>, double>  solve(const vector<shared_ptr<const POMDPVertex>> &initial_states,
                         const int &horizon);
    virtual pair<shared_ptr<MixedStrategy>, double>  solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
        const int &horizon) = 0;
};

class ParetoSolver final : public Solver {
    protected:
        shared_ptr<Hull> get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon);
    public:
        int final_hull_size;
        ParetoSolver(const POMDP &pomdp, const bool &convexify);
        pair<shared_ptr<MixedStrategy>, double>  solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) override;
};

#endif