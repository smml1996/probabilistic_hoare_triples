#ifndef SOLVERS_H
#define SOLVERS_H

#include "pomdp.hpp"
#include "Belief.hpp"
#include "algorithm.hpp"
#include <functional>

using f_reward_type = std::function<MyFloat(const Belief&, const unordered_map<int, int> &)>;
using f_reward_type_double = std::function<double(const VertexDict&, const unordered_map<int, int> &)>;

class SingleDistributionSolver {
    POMDP pomdp;
    f_reward_type get_reward;
    int precision;
    unordered_map<int, int> embedding;
    MyFloat error;
    int max_horizon;

    // PBVI methods
    [[nodiscard]] bool is_belief_visited(const Belief &belief) const;
    [[nodiscard]] MyFloat get_closest_L1(const Belief &belief) const;
    public:
        unordered_map<Belief, unordered_map<int, pair<shared_ptr<Algorithm>, MyFloat>>, BeliefHash> beliefs_to_rewards;
        SingleDistributionSolver(const POMDP &pomdp, const f_reward_type &get_reward, int precision, const unordered_map<int, int> & embedding);
        pair<shared_ptr<Algorithm>, MyFloat> get_bellman_value(const Belief &current_belief, const int &horizon);

        // PBVI
        pair<shared_ptr<Algorithm>, MyFloat> PBVI_solve(const Belief &current_beliefs, const int &horizon);
        [[nodiscard]] double get_error(const int &horizon) const;
        void print_all_beliefs() const;

};


class ConvexDistributionSolver {
    POMDP pomdp;
    f_reward_type_double get_reward;
    f_reward_type precise_get_reward;
    int precision;
    unordered_map<int, int> embedding;
    cpp_int initial_classical_state;
    guard_type guard;
    void get_matrix_maximin(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const shared_ptr<Algorithm> &current_algorithm,
            unordered_map<int, unordered_map<int, double>> &minimax_matrix,
            const int &max_horizon,
            unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm);

    void set_minimax_values(
            const shared_ptr<Algorithm> &algorithm,
            const vector<shared_ptr<POMDPVertex>> &initial_states,
            unordered_map<int, unordered_map<int, double>> &minimax_matrix,
            unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm);
    bool is_action_allowed(shared_ptr<POMDPAction> &action, const vector<shared_ptr<POMDPVertex>> &states);
    static pair<vector<double>, double> solve_lp_maximin(const unordered_map<int, unordered_map<int, double>> &maximin_matrix, const int &n_algorithms, const int &n_initial_states);
    public:
        ConvexDistributionSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward, const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> & embedding, const guard_type &g);
        pair<shared_ptr<Algorithm>, double> solve(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon);
        // pair<shared_ptr<Algorithm>, double> antichain_solver(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon);
};

MyFloat get_algorithm_acc(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const Belief &current_belief, const f_reward_type &get_reward, const unordered_map<int, int> &embedding, int precision);
double get_algorithm_acc_double(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const VertexDict &current_belief, const f_reward_type_double &get_reward, const unordered_map<int, int> &embedding);

#endif