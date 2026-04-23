#ifndef SOLVERS_H
#define SOLVERS_H

#include "pomdp.hpp"
#include "hull.hpp"
#include "Belief.hpp"
#include <functional>
#include <unordered_map>
#include <memory>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/convex_hull_d.h>


using namespace std;

static shared_ptr<POMDPAction> halt_action = make_shared<POMDPAction>("HALT");
static shared_ptr<Belief> empty_belief = make_shared<Belief>();
namespace mp = boost::multiprecision;
using cpp_int = mp::cpp_int;

// using strategy_score_type = pair<shared_ptr<Strategy>, shared_ptr<MWP>>;

class Solver {
protected:
    const MyFloat zero;
    unordered_set<int> target_vertices;
    POMDP pomdp;
    MyFloat get_reward(const shared_ptr<Belief> &b) const;
    map<int, shared_ptr<Belief>> get_successor_beliefs(const shared_ptr<Belief> &belief, const shared_ptr<POMDPAction> &action);
    vector<shared_ptr<Multibelief>> get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action);
    shared_ptr<MWP> get_mwp(const shared_ptr<Multibelief>&beliefs);
    vector<shared_ptr<MWP>> get_achievable_mwps(shared_ptr<MWP> &current_score, const vector<set<shared_ptr<MWP>, MWPPtrComp>> &multibelief_points, int mb_index=0);
    double solve_lp_maximin(const int &n_initial_states, const set<shared_ptr<MWP>, MWPPtrComp>& scores);
public:
    virtual ~Solver() = default;

    virtual double solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
                         const int &horizon);
    virtual double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
        const int &horizon) = 0;
};

class ParetoSolver : public Solver {
    Hull dominant_points;
    bool convexify;
protected:
    virtual set<shared_ptr<MWP>, MWPPtrComp> get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon);
    public:
        ParetoSolver(const POMDP &pomdp, unordered_set<int> &target_vertices, const bool &convexify);
        double solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) override;
};

#endif