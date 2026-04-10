#ifndef SOLVERS_H
#define SOLVERS_H

#include "pomdp.hpp"
#include "Belief.hpp"
#include "algorithm.hpp"
#include <functional>

using f_reward_type = std::function<MyFloat(const Belief&, const unordered_map<int, int> &)>;
using f_reward_type_double = std::function<double(const VertexDict&, const unordered_map<int, int> &)>;

using multibelief_type = vector<shared_ptr<Belief>>;

class SingleDistributionSolver {
    POMDP pomdp;
    f_reward_type get_reward;
    int precision;
    unordered_map<int, int> embedding;
    MyFloat error;
    int max_horizon;
    public:
        SingleDistributionSolver(const POMDP &pomdp, const f_reward_type &get_reward, int precision, const unordered_map<int, int> & embedding);
        pair<shared_ptr<Algorithm>, MyFloat> get_bellman_value(const Belief &current_belief, const int &horizon);

};

class MWP {
public:
    vector<MyFloat> values;
    double get(const int &index);
    bool operator<=(const MWP &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] > other.values[i]) {
                return false;
            }
        }
        return true;
    }

    shared_ptr<MWP> operator+(const MWP &other) {
        shared_ptr<MWP> result = make_shared<MWP>();
        assert(this->values.size() == other.values.size());

        for (int i = 0; i< this->values.size(); i++) {
                result->values.push_back(this->values[i] + other.values[i]);
        }
        return result;
    }

};


class ConvexDistributionSolver {
    MyFloat zero;
protected:

    shared_ptr<POMDPAction> halt_action;
    POMDP pomdp;
    f_reward_type_double get_reward;
    f_reward_type precise_get_reward;
    int precision;
    unordered_map<int, int> embedding;

    unordered_map<shared_ptr<Strategy>, int> strat_to_length;
    map<shared_ptr<Strategy>, shared_ptr<MWP>> scores;
    map<shared_ptr<Strategy>, shared_ptr<MWP>> get_matrix_maximin(const vector<shared_ptr<Belief>> &beliefs, const int &horizon);
    shared_ptr<Strategy> set_minimax_values(const shared_ptr<Strategy> &strategy, const shared_ptr<MWP> &mwp, const int &horizon);
    pair<MixedStrategy, double> solve_lp_maximin(const int &n_initial_states);
    vector<shared_ptr<Belief>> get_successor_beliefs(const shared_ptr<Belief> &belief, const shared_ptr<POMDPAction> &action);
    void get_multibelief_successors(const vector<vector<shared_ptr<Belief>>> &successors_per_belief, multibelief_type &current, vector<multibelief_type>&result, const int &from_index=0);
    vector<vector<pair<shared_ptr<Belief>, shared_ptr<POMDPAction>>>>
        get_one_step_strategies(const vector<shared_ptr<Belief>> &beliefs);
    shared_ptr<MWP> get_mwp(const vector<shared_ptr<Belief>>&beliefs);
    public:
        ConvexDistributionSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward,
            const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> &embedding);
        virtual pair<shared_ptr<Algorithm>, double> solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon);
};
MyFloat get_algorithm_acc(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const Belief &current_belief, const f_reward_type &get_reward, const unordered_map<int, int> &embedding, int precision);
double get_algorithm_acc_double(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const VertexDict &current_belief, const f_reward_type_double &get_reward, const unordered_map<int, int> &embedding);

#endif