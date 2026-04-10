#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <iostream>

#include <cassert>

SingleDistributionSolver::SingleDistributionSolver(const POMDP &pomdp, const f_reward_type &get_reward, int precision, const unordered_map<int, int> & embedding) : max_horizon(
    0) {
    this->pomdp = pomdp;
    this->get_reward = get_reward;
    this->precision = precision;
    this->embedding = embedding;
    assert(precision != -1);
    this->error = MyFloat("0", this->precision);
}

pair<shared_ptr<Algorithm>, MyFloat> SingleDistributionSolver::get_bellman_value(const Belief &current_belief, const int &horizon){

    MyFloat curr_belief_val = this->get_reward(current_belief, this->embedding);
    
    cpp_int current_classical_state = get_belief_cs(current_belief);
    assert(current_classical_state >= 0);
    auto halt_algorithm = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), current_classical_state, 0);
    if (horizon == 0) {
        return make_pair(halt_algorithm, curr_belief_val);
    }

    vector< pair< shared_ptr<Algorithm>, MyFloat > > bellman_values;

    bellman_values.emplace_back(halt_algorithm, curr_belief_val);
    
    for (auto & it : pomdp.actions) {
        const auto& action = it;

        // build next_beliefs, separate them by different observables
        map<cpp_int, Belief> obs_to_next_beliefs;
        const MyFloat zero("0", this->precision);

        for(auto & prob : current_belief.probs) {
            auto current_v = prob.first;
            assert(prob.second > zero);
            for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
                if (it_next_v.second > zero) {
                    auto successor = it_next_v.first;
                    assert(prob.first->hybrid_state != nullptr);
                    assert(successor != nullptr);
                    assert(successor->hybrid_state != nullptr);
                    assert (it_next_v.first->hybrid_state != nullptr);
                    assert (it_next_v.first->hybrid_state->classical_state != nullptr);
                    obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(successor,
                                                                              prob.second * it_next_v.second);
                }
            }
        }
        
        if (!obs_to_next_beliefs.empty()) {
            auto new_alg_node = new Algorithm(action, current_classical_state, this->precision);
            MyFloat bellman_val("0", this->precision);

            int max_depth = 0;
            for(auto & obs_to_next_belief : obs_to_next_beliefs) {
                auto temp = get_bellman_value(obs_to_next_belief.second, horizon-1);
                new_alg_node->children.push_back(temp.first);
                max_depth = max(temp.first->depth, max_depth);
                bellman_val = bellman_val + temp.second;
            }

            new_alg_node->depth = max_depth + 1;
            bellman_values.emplace_back(new_alg_node, bellman_val);
        }
    }

    MyFloat max_val("0", this->precision); // this is initialized as zero
    for(auto & bellman_value : bellman_values) {
        // auto val_1 = to_double(bellman_value.second);
        // auto val_2 = to_double(max_val);
        // assert((val_1 < val_2) == (bellman_value.second < max_val));
        // assert((val_1 > val_2) == (bellman_value.second > max_val));
        // assert((val_1 == val_2) == (bellman_value.second == max_val));
        max_val = max(max_val, bellman_value.second);
    }

    int shortest_alg_with_max_val = -1;
    for(auto & bellman_value : bellman_values) {
        if (bellman_value.second == max_val) {
            if (shortest_alg_with_max_val == -1) {
                shortest_alg_with_max_val = bellman_value.first->depth;
            } else {
                shortest_alg_with_max_val = min(shortest_alg_with_max_val, bellman_value.first->depth);
            }
        }
    }

    for(auto & bellman_value : bellman_values) {
        if (bellman_value.second == max_val && bellman_value.first->depth == shortest_alg_with_max_val) {
            return bellman_value;
        }
    }
    assert(false);
}

double MWP::get(const int &index) {
    if (index >= values.size()) {
        throw std::out_of_range("index out of range");
    }

    return to_double(this->values[index]);

}

void ConvexDistributionSolver::get_multibelief_successors(
    const vector<vector<shared_ptr<Belief>>> &successors_per_belief, multibelief_type &current,  vector<multibelief_type> &result, const int &from_index)  {
    if (from_index == successors_per_belief.size()) {
        assert(current.size() == successors_per_belief.size());
        result.push_back(current);
    }

    for (auto successor : successors_per_belief[from_index]) {
        current.push_back(successor);
        this->get_multibelief_successors(successors_per_belief, current, result, from_index+1);
        current.pop_back();
    }

}

vector<vector<pair<shared_ptr<Belief>, shared_ptr<POMDPAction>>>> ConvexDistributionSolver::get_one_step_strategies(
    const multibelief_type &beliefs) {
    // goal: assign an action to each belief in a multibelief state
    vector<vector<pair<shared_ptr<Belief>, shared_ptr<POMDPAction>>>> result;

    // find observations

    unordered_set<cpp_int> unique_obs;
    vector<cpp_int> tuple_obs;


    // if (from_index == beliefs.size() -1) {
    //     for (auto action : this->pomdp.actions) {
    //         vector<pair<shared_ptr<Belief>, shared_ptr<POMDPAction>>> current {
    //             make_pair(beliefs[from_index], action)
    //         };
    //         result.push_back(current);
    //     }
    //     return result;
    // }
    //
    // auto other_one_step_strategies = get_one_step_strategies(beliefs, from_index+1);
    //
    // for (auto action : this->pomdp.actions) {
    //     for (auto one_step_strat : other_one_step_strategies) {
    //         vector<pair<shared_ptr<Belief>, shared_ptr<POMDPAction>>> current {
    //             make_pair(beliefs[from_index], action)
    //         };
    //
    //         for (auto belief_strat : one_step_strat) {
    //             current.push_back(belief_strat);
    //         }
    //         result.push_back(current);
    //     }
    //
    return result;
}

shared_ptr<MWP> ConvexDistributionSolver::get_mwp(const vector<shared_ptr<Belief>> &beliefs) {
    shared_ptr<MWP> current_mwp = make_shared<MWP>();
    for (auto belief: beliefs) {
        current_mwp->values.push_back(this->precise_get_reward(*belief, this->embedding));
    }
    return current_mwp;
}

ConvexDistributionSolver::ConvexDistributionSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward,
                                                   const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> &embedding) {
    this->pomdp = pomdp;
    this->get_reward = get_reward;
    this->precise_get_reward = precise_get_reward;
    this->precision = precision;
    this->embedding = embedding;
    this->scores = map<shared_ptr<Strategy>, shared_ptr<MWP>>{};
    this->halt_action = make_shared<POMDPAction>(HALT_ACTION);
    this->strat_to_length = unordered_map<shared_ptr<Strategy>, int>{};
    this->zero = MyFloat("0", this->precision);
}

MyFloat get_algorithm_acc(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const Belief &current_belief, const f_reward_type &get_reward, const unordered_map<int, int> &embedding, int precision) {
    MyFloat curr_belief_val = get_reward(current_belief, embedding);
    if (algorithm == nullptr) {
        return curr_belief_val;
    }
    auto action = algorithm->action;
    if (*action == HALT_ACTION) {
        return curr_belief_val;
    }

    // build next_beliefs, separate them by different observables
    unordered_map<cpp_int, Belief> obs_to_next_beliefs;

    MyFloat zero("0", precision);
    for(auto & prob : current_belief.probs) {
        auto current_v = prob.first;
        if(prob.second > zero) {
            for (auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
                if (it_next_v.second > zero) {
                    obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(it_next_v.first,
                                                                              prob.second * it_next_v.second);
                }else {
                    // cout << it_next_v.second << endl;
                    // assert(it_next_v.second == zero);
                }
            }
        }
    }

    // assert(algorithm->children.size() <= obs_to_next_beliefs.size());

    if (!obs_to_next_beliefs.empty()) {
        MyFloat bellman_val("0", precision);
        set<cpp_int> visited_cstates;
        for (int i = 0; i < algorithm->children.size(); i++) {
            if(obs_to_next_beliefs.find(algorithm->children[i]->classical_state) != obs_to_next_beliefs.end()) {
                visited_cstates.insert(algorithm->children[i]->classical_state);
                bellman_val = bellman_val + get_algorithm_acc(pomdp, algorithm->children[i], obs_to_next_beliefs[algorithm->children[i]->classical_state], get_reward, embedding, precision);
            }
        }

        for (auto it: obs_to_next_beliefs) {
            if (visited_cstates.find(it.first) == visited_cstates.end()) {
                bellman_val = bellman_val + get_reward(it.second, embedding);
            }
        }
        return bellman_val;
    } else {
        return curr_belief_val;
    }
}

double get_algorithm_acc_double(POMDP &pomdp, const shared_ptr<Algorithm>& algorithm, const VertexDict &current_belief, const f_reward_type_double &get_reward, const unordered_map<int, int> &embedding) {
    double curr_belief_val = get_reward(current_belief, embedding);

    if (algorithm == nullptr) {
        return curr_belief_val;
    }
    auto action = algorithm->action;
    if (*action == HALT_ACTION) {
        return curr_belief_val;
    }

    // build next_beliefs, separate them by different observables
    unordered_map<cpp_int, VertexDict> obs_to_next_beliefs;

    for(auto & prob : current_belief.probs) {
        auto current_v = prob.first;
        if(prob.second > 0) {
            assert (pomdp.transition_matrix_[current_v].find(action) != pomdp.transition_matrix_[current_v].end());
            for (auto &it_next_v: pomdp.transition_matrix_[current_v][action]) {
                if (it_next_v.second > 0) {
                    obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(it_next_v.first,
                                                                              prob.second * it_next_v.second);
                } else {
                    // cout << it_next_v.second << endl;
                    // assert(it_next_v.second == zero);
                }
            }
        }
    }

    // assert(algorithm->children.size() <= obs_to_next_beliefs.size());
    if (!obs_to_next_beliefs.empty()) {
        double bellman_val = 0.0;
        set<cpp_int> visited_cstates;
        for (int i = 0; i < algorithm->children.size(); i++) {
            if(obs_to_next_beliefs.find(algorithm->children[i]->classical_state) != obs_to_next_beliefs.end()) {
                visited_cstates.insert(algorithm->children[i]->classical_state);
                bellman_val = bellman_val + get_algorithm_acc_double(pomdp, algorithm->children[i], obs_to_next_beliefs[algorithm->children[i]->classical_state], get_reward, embedding);
            }
        }

        for (auto it: obs_to_next_beliefs) {
            if (visited_cstates.find(it.first) == visited_cstates.end()) {
                bellman_val = bellman_val + get_reward(it.second, embedding);
            }
        }
        return bellman_val;
    } else {
        return curr_belief_val;
    }
}

shared_ptr<Strategy> ConvexDistributionSolver::set_minimax_values(const shared_ptr<Strategy> &strategy, const shared_ptr<MWP> &mwp, const int &horizon) {
    // update scores
    // strat_to_length

    unordered_set<shared_ptr<Strategy>> to_remove;
    for (auto p : this->scores) {
        if (mwp <= p.second) {
            assert(to_remove.empty());
            return p.first;
        }

        if (p.second <= mwp) {
            to_remove.insert(p.first);
        }
    }

    this->scores.insert({strategy, mwp});
    this->strat_to_length.insert({strategy, horizon});

    for (auto r_strat : to_remove) {
        this->scores.erase(r_strat);
        this->strat_to_length.erase(r_strat);
    }

    return strategy;
}

map<shared_ptr<Strategy>, shared_ptr<MWP>> ConvexDistributionSolver::get_matrix_maximin(const vector<shared_ptr<Belief>> &beliefs, const int &horizon) {
    // this function returns all strategies that are reachable from the given beliefs and are of length horizon
    cout << "horizon: " << horizon << endl;
    map<shared_ptr<Strategy>, shared_ptr<MWP>> result;
    if (horizon == 0) {
        shared_ptr<Strategy> strategy = make_shared<Strategy>();
        for (auto belief: beliefs) {
            strategy->insert(horizon, belief, this->halt_action);
        }
        auto mwp = get_mwp(beliefs);
        auto strategy_ = this->set_minimax_values(strategy, mwp, horizon);
        return {make_pair(strategy_, mwp)};
    } else {
        assert(horizon > 0);

        map<shared_ptr<Strategy>, shared_ptr<MWP>> temp_result;
        for (auto one_step_strategy : get_one_step_strategies(beliefs)) {
            shared_ptr<Strategy> strategy = make_shared<Strategy>(); // we are building this strategy
            vector<vector<shared_ptr<Belief>>> successor_beliefs;

            for (auto belief_strat_pair : one_step_strategy) {
                // belief_strat_pair specifies which action to take at each of the current beliefs
                auto current_belief = belief_strat_pair.first;
                auto action = belief_strat_pair.second;
                strategy->insert(horizon, current_belief, action);
                successor_beliefs.push_back(this->get_successor_beliefs(current_belief, action));
            }

            // successor multibeliefs we can reach
            vector<multibelief_type> multibelief_successors;
            multibelief_type dummy_var___;
            this->get_multibelief_successors(successor_beliefs, dummy_var___, multibelief_successors);
            shared_ptr<MWP> current_score;
            for (auto successor_multibelief : multibelief_successors) {
                auto reachable_strat_scores = get_matrix_maximin(
                    successor_multibelief, horizon-1);

                for (auto succ_belief : successor_multibelief) {
                    for (auto element_ : reachable_strat_scores) {
                        auto succ_strat = element_.first;
                        current_score = *current_score + *element_.second;
                        append_strategy(strategy, succ_strat); // this is the new strategy
                    }
                }
            }

            // update set of strategies
            auto strategy_index = this->set_minimax_values(strategy, current_score, horizon);
            temp_result.insert({strategy_index, current_score});
        }

        for (auto strat : temp_result) {
            if (this->scores.find(strat.first) != this->scores.end()) {
                result.insert(strat);
            }
        }

        return result;
    }

}

pair<MixedStrategy, double> ConvexDistributionSolver::solve_lp_maximin(const int &n_initial_states) {
    operations_research::MPSolver solver("max_v", operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING);
    solver.SetSolverSpecificParametersAsString(
    "primal_feasibility_tolerance:1e-9 dual_feasibility_tolerance:1e-9");
    // Variables: x_i >= 0
    int n_algorithms = scores.size();

    unordered_map<int, shared_ptr<Strategy>> index_to_strat;

    int strat_index__ = 0;
    for (auto strat_p : this->strat_to_length) {
        index_to_strat.insert({strat_index__, strat_p.first});
        strat_index__ += 1;
    }

    std::vector<operations_research::MPVariable*> x(n_algorithms);
    for (int i = 0; i < n_algorithms; ++i) {
        x[i] = solver.MakeNumVar(0.0, 1.0, "x_" + std::to_string(i));
    }

    // Variable: v
    operations_research::MPVariable* v = solver.MakeNumVar(0.0, INFINITY, "v");

    // Constraint: sum_i x_i = 1
    operations_research::MPConstraint* prob_sum = solver.MakeRowConstraint(1.0, 1.0);
    for (int i = 0; i < n_algorithms; ++i) {
        prob_sum->SetCoefficient(x[i], 1.0);
    }

    // Constraints: sum_i x_i * M_ij >= v  for all j
    for (int j = 0; j < n_initial_states; ++j) {
        operations_research::MPConstraint* c = solver.MakeRowConstraint(0.0, solver.infinity());
        for (int i = 0; i < n_algorithms; ++i) {
            auto strategy = index_to_strat.at(i);
            shared_ptr<MWP> mwp = this->scores.find(strategy)->second;
            assert(mwp->values.size() == n_initial_states);
            c->SetCoefficient(x[i], mwp->get(j));
        }
        c->SetCoefficient(v, -1.0); // sum_i(...) - v >= 0  → sum_i(...) >= v
    }

    // Objective: maximize v
    operations_research::MPObjective* objective = solver.MutableObjective();
    objective->SetCoefficient(v, 1.0);
    objective->SetMaximization();

    // Solve
    double sum_ = 0.0;
    auto result = solver.Solve();
    vector<double> probs;
    // cout << "v: " << v->solution_value() << endl;
    double final_value = v->solution_value();
    assert (result == operations_research::MPSolver::OPTIMAL);
    for (int i = 0; i < n_algorithms; ++i) {
        probs.push_back(x[i]->solution_value());
        sum_ += probs[i];
    }

    if (!is_close(sum_, 1.0, 8)) {
        throw runtime_error("sum_ is incorrect: " + to_string(sum_));
    }
    for (int i = 0 ; i < probs.size() ; i++) {
        probs[i] /= sum_;
    }

    return make_pair(MixedStrategy(probs, index_to_strat), final_value);
}

vector<shared_ptr<Belief>> ConvexDistributionSolver::get_successor_beliefs(const shared_ptr<Belief> &current_belief,
    const shared_ptr<POMDPAction> &action) {
    if (*action == *this->halt_action) {
        return {current_belief};
    }
    map<cpp_int, Belief> obs_to_next_beliefs;

    for(auto & prob : current_belief->probs) {
        auto current_v = prob.first;
        assert(prob.second > this->zero);
        for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
            if (it_next_v.second > this->zero) {
                auto successor = it_next_v.first;
                auto obs = it_next_v.first->hybrid_state->classical_state->get_memory_val();
                obs_to_next_beliefs[obs].add_val(successor, prob.second * it_next_v.second);
            }
        }
    }

    vector<shared_ptr<Belief>> result;

    for (auto element : obs_to_next_beliefs) {
        element.second.obs = element.first;
        result.push_back(make_shared<Belief>(element.second));
    }

    return result;
}

pair<shared_ptr<Algorithm>, double> ConvexDistributionSolver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon) {
    this->pomdp.actions.push_back(make_shared<POMDPAction>(HALT_ACTION));
    this->scores.clear();


    vector<shared_ptr<Belief>> initial_beliefs;
    unordered_set<cpp_int> initial_obs;
    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>();
        belief->set_val(initial_states[i], MyFloat(1, this->precision));
        belief->obs = initial_states[i]->hybrid_state->classical_state->get_memory_val();
        assert(initial_obs.find(belief->obs) != initial_obs.end() || initial_obs.empty());
        initial_obs.insert(belief->obs);
        initial_beliefs.push_back(belief);
    }

    this->get_matrix_maximin(initial_beliefs, horizon);

    cout << "length strat to length: " << strat_to_length.size() << endl;

    auto result = this->solve_lp_maximin(initial_states.size());

    this->pomdp.actions.pop_back();
    return make_pair(result.first.to_algorithm(), result.second);
}
