#ifndef SOLVERS_H
#define SOLVERS_H

#include "pomdp.hpp"
#include "hull.hpp"
#include "Belief.hpp"
#include <functional>
#include <chrono>
#include <memory>
#include <absl/time/internal/cctz/include/cctz/time_zone.h>


using namespace std;

static shared_ptr<Belief> empty_belief = make_shared<Belief>();
namespace mp = boost::multiprecision;
using cpp_int = mp::cpp_int;

// using strategy_score_type = pair<shared_ptr<Strategy>, shared_ptr<MWP>>;

class Solver {
protected:
    chrono::time_point<chrono::steady_clock, chrono::steady_clock::duration> start_time;
    bool is_timeout;
    const MyFloat zero;
    POMDP pomdp;
    MyFloat get_reward(const shared_ptr<Belief> &b, const shared_ptr<POMDPAction> &action) const;
    map<int, shared_ptr<Belief>> get_successor_beliefs(const shared_ptr<Belief> &belief, const shared_ptr<POMDPAction> &action);
    vector<shared_ptr<Multibelief>> get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action);
    shared_ptr<MWP> get_mwp(const shared_ptr<Multibelief>&beliefs, const shared_ptr<POMDPAction> &action) const;
    vector<shared_ptr<MWP>> get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<shared_ptr<Hull>> &multibelief_points, int mb_index=0);

    double solve_lp_maximin(const int &n_initial_states, const Hull& scores);

    void check_time();
public:
    double running_time;
    static long long timelimit; // seconds
    virtual ~Solver() = default;

    virtual double solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
                         const int &horizon);
    virtual double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
        const int &horizon) = 0;
};

class ParetoSolver : public Solver {
    bool convexify;
protected:
    shared_ptr<Hull> get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon);
    public:
        int final_hull_size;
        ParetoSolver(const POMDP &pomdp, const bool &convexify);
        double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) override;
};

#endif