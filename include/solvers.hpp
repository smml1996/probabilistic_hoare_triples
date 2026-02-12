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


class Point {
public:
    vector<MyFloat> values;
    shared_ptr<Algorithm> algorithm;
    Point(const shared_ptr<Algorithm> &algorithm) {
        this->algorithm = algorithm;
    };
    Point(const vector<MyFloat> &values, const shared_ptr<Algorithm> &algorithm) {
        this->values = values;
        this->algorithm = algorithm;
    }

    bool operator==(const Point &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] != other.values[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator<=(const Point &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] > other.values[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator>=(const Point &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] < other.values[i]) {
                return false;
            }
        }
        return true;
    }

};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        auto v = p.values;
        std::size_t seed = v.size();
        for (const auto& x : v) {
            std::size_t h = std::hash<std::string>{}(to_string(x));
            seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


class ConvexDistributionSolver {
protected:
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
        ConvexDistributionSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward,
            const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> & embedding,
            const guard_type &g);
        virtual pair<shared_ptr<Algorithm>, double> solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon);
};

class AntichainSolver : public ConvexDistributionSolver {
    pair<shared_ptr<Algorithm>, MyFloat> get_algorithm(const set<shared_ptr<Point>> &points) const;
    void insert_point(set<shared_ptr<Point>> &antichain, shared_ptr<Point> &point);
    set<shared_ptr<Point>> get_antichain(const vector<Belief>& beliefs, const int &horizon);
    shared_ptr<Point> get_point(const vector<Belief> &beliefs);
    map<cpp_int, Belief> get_next_beliefs(const Belief &belief);
    void combine_obs_points(const shared_ptr<Algorithm> &current_algorithm, const map<cpp_int, shared_ptr<Point>> &obs_to_points, set<shared_ptr<Point>> &antichain);
public:
    AntichainSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward, const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> & embedding, const guard_type &g) :
    ConvexDistributionSolver(pomdp, precise_get_reward, get_reward, precision, embedding, g) {};
    pair<shared_ptr<Algorithm>, MyFloat> solve_exact(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon);
};

MyFloat get_algorithm_acc(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const Belief &current_belief, const f_reward_type &get_reward, const unordered_map<int, int> &embedding, int precision);
double get_algorithm_acc_double(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const VertexDict &current_belief, const f_reward_type_double &get_reward, const unordered_map<int, int> &embedding);

#endif