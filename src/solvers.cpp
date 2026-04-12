#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <cassert>

using namespace std;


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

MWP::MWP(const int &size, const int &precision) {
    this->precision = precision;
    for (int i = 0; i <size; i++) {
        this->values.push_back(MyFloat("0", precision));
    }
}

double MWP::get(const int &index) {
    if (index >= values.size()) {
        throw std::out_of_range("index out of range");
    }

    return to_double(this->values[index]);

}

vector<shared_ptr<Multibelief>> ConvexDistributionSolver::get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action)  {
    // we first compute which beliefs we can reach for each belief in the multibelief
    vector<map<cpp_int, shared_ptr<Belief>>> successor_beliefs; // this vector should be (at the end) the same length as the multibelief.
                            // I.e., each index corresponds to the beliefs that can be reached by the corresponding belief
    set<cpp_int> reachable_obs;
    bool reached_found = false;
    for (auto belief : current->beliefs) {
        if (belief->is_unreached) {
                successor_beliefs.push_back({});
        } else {
            reached_found = true;
            assert(!belief->is_unreached);
            auto current_successors = this->get_successor_beliefs(belief, action);
            successor_beliefs.push_back(current_successors);
            for (auto successor_belief : current_successors) {
                reachable_obs.insert(successor_belief.first);
            }
        }

    }
    assert(reached_found);
    assert(reachable_obs.size() > 0);
    assert(successor_beliefs.size() == current->beliefs.size());

    // empty multibelief (an observation cannot be reached in this world)
    shared_ptr<Belief> empty_belief = make_shared<Belief>(true); // TODO: put as static variable

    // create a multibelief for each observable that can be reached
    vector<shared_ptr<Multibelief>> result;
    for (auto obs : reachable_obs) {
        vector<shared_ptr<Belief>> elements_multibelief;
        for (auto m_successor_belief : successor_beliefs) {
            if (m_successor_belief.find(obs) == m_successor_belief.end()) {
                elements_multibelief.push_back(empty_belief);
            } else {
                elements_multibelief.push_back(m_successor_belief.find(obs)->second);
            }
        }
        result.push_back(make_shared<Multibelief>(elements_multibelief, obs));
    }

    return result;
}

shared_ptr<MWP> ConvexDistributionSolver::get_mwp(const shared_ptr<Multibelief> &multibelief) {
    shared_ptr<MWP> current_mwp = make_shared<MWP>(multibelief->beliefs.size(), this->precision);
    int i = 0;
    for (auto belief: multibelief->beliefs) {
        current_mwp->values[i] = this->precise_get_reward(*belief, this->embedding);
        i+=1;
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
    this->halt_action = make_shared<POMDPAction>(HALT_ACTION);
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

bool ConvexDistributionSolverHull::update_result_set(const shared_ptr<Strategy> &strategy, const shared_ptr<MWP> &mwp,
    map<shared_ptr<Strategy>, shared_ptr<MWP>> &scores) {
    if (scores.size() < 2) {
        return ConvexDistributionSolver::update_result_set(strategy, mwp, scores);
    }

    return true;

}

bool ConvexDistributionSolver::update_result_set(const shared_ptr<Strategy> &strategy, const shared_ptr<MWP> &mwp, map<shared_ptr<Strategy>, shared_ptr<MWP>> &scores) {
    unordered_set<shared_ptr<Strategy>> to_remove;
    for (auto p : scores) {
        if (*mwp <= *p.second) {
            assert(to_remove.empty());
            return false;
        }

        if (*p.second <= *mwp) {
            to_remove.insert(p.first);
        }
    }

    scores.insert(make_pair(strategy, mwp));
    for (auto r_strat : to_remove) {
        scores.erase(r_strat);
    }
    return true;
}

vector<pair<shared_ptr<Strategy>, shared_ptr<MWP>>> ConvexDistributionSolver::get_final_strategies(shared_ptr<Strategy> &current_strategy,
    shared_ptr<MWP> &current_score,
    const vector<map<shared_ptr<Strategy>, shared_ptr<MWP>>> &m_strategy_score,  int from_index) {

    vector<pair<shared_ptr<Strategy>, shared_ptr<MWP>>> temp;
    for (auto current_m : m_strategy_score[from_index]) {
        auto temp_strategy = make_shared<Strategy>(Strategy(*current_strategy));
        temp_strategy->insert(current_m.first);
        auto new_score = *current_score + *current_m.second;
        temp.push_back(make_pair(temp_strategy, new_score));
    }

    if (from_index == m_strategy_score.size()-1) {
        return temp;
    }

    vector<pair<shared_ptr<Strategy>, shared_ptr<MWP>>> result;
    for (auto strategy : temp) {
        auto succ_strategies = this->get_final_strategies(strategy.first, strategy.second,
            m_strategy_score, from_index +1);

        for (auto ss : succ_strategies) {
            result.push_back(ss);
        }
    }
    return result;
}

map<shared_ptr<Strategy>, shared_ptr<MWP>> ConvexDistributionSolver::get_matrix_maximin(const shared_ptr<Multibelief> &multibelief, const int &horizon) {
    // this function returns all strategies that are reachable from the given beliefs and are of length horizon
    // we assume that all beliefs here have the same observation

    map<shared_ptr<Strategy>, shared_ptr<MWP>> result;

    // consider strategy that halts immediatly
    shared_ptr<Strategy> halt_strategy = make_shared<Strategy>(horizon, this->halt_action, multibelief->get_obs());
    auto mwp_halt = get_mwp(multibelief);
    result.insert(make_pair(halt_strategy, mwp_halt));
    // ****************

    if (horizon == 0) {
        return result;
    } else {
        assert(horizon > 0);
        for (auto action : this->pomdp.actions) {
            if (!(*action == *(this->halt_action))) {
                // compute reachable multibeliefs
                vector<shared_ptr<Multibelief>> multibelief_successors = this->get_multibelief_successors(multibelief, action);

                // get strategies for each successor
                vector<map<shared_ptr<Strategy>, shared_ptr<MWP>>> succ_strategies;

                for (auto succ_mb : multibelief_successors) {
                    succ_strategies.push_back(this->get_matrix_maximin(succ_mb, horizon-1));
                }

                shared_ptr<Strategy> current_strategy = make_shared<Strategy>(horizon, action, multibelief->get_obs());
                shared_ptr<MWP> current_score_ = make_shared<MWP>(multibelief->beliefs.size(), this->precision);
                vector<pair<shared_ptr<Strategy>, shared_ptr<MWP>>> new_strategies = this->get_final_strategies(current_strategy, current_score_, succ_strategies);

                for (auto strategy_score : new_strategies) {
                    // update set of strategies
                    auto strategy = strategy_score.first;
                    auto current_score = strategy_score.second;
                    this->update_result_set(strategy, current_score, result);
                }
            }
        }

        return result;
    }

}

pair<shared_ptr<MixedStrategy>, double> ConvexDistributionSolver::solve_lp_maximin(const int &n_initial_states, const map<shared_ptr<Strategy>, shared_ptr<MWP>>& scores) {
    operations_research::MPSolver solver("max_v", operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING);
    solver.SetSolverSpecificParametersAsString(
    "primal_feasibility_tolerance:1e-9 dual_feasibility_tolerance:1e-9");
    // Variables: x_i >= 0
    int n_algorithms = scores.size();

    unordered_map<int, shared_ptr<Strategy>> index_to_strat;

    int strat_index__ = 0;
    for (auto strat_p : scores) {
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
            shared_ptr<MWP> mwp = scores.find(strategy)->second;
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

    return make_pair(make_shared<MixedStrategy>(probs, index_to_strat), final_value);
}

map<cpp_int, shared_ptr<Belief>> ConvexDistributionSolver::get_successor_beliefs(const shared_ptr<Belief> &current_belief,
    const shared_ptr<POMDPAction> &action) {
    assert (!(*action == *this->halt_action));

    map<cpp_int, shared_ptr<Belief>> obs_to_next_beliefs;
    assert(current_belief->probs.size() > 0);
    for(auto & prob : current_belief->probs) {
        auto current_v = prob.first;
        assert(prob.second > this->zero);
        for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
            if (it_next_v.second > this->zero) {
                auto successor = it_next_v.first;
                auto obs = it_next_v.first->hybrid_state->classical_state->get_memory_val();
                if (obs_to_next_beliefs.find(obs) == obs_to_next_beliefs.end()) {
                    obs_to_next_beliefs[obs] = make_shared<Belief>();
                    obs_to_next_beliefs[obs]->obs = obs;
                }
                obs_to_next_beliefs[obs]->add_val(successor, prob.second * it_next_v.second);
            }
        }
    }

    assert(obs_to_next_beliefs.size() > 0);

    return obs_to_next_beliefs;
}

pair<shared_ptr<Algorithm>, double> ConvexDistributionSolver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon) {


    auto result = this->solve_strategy(initial_states, horizon);

    return make_pair(result.first->to_algorithm(), result.second);
}

pair<shared_ptr<MixedStrategy>, double> ConvexDistributionSolver::solve_strategy(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon) {
    this->pomdp.actions.push_back(this->halt_action);

    vector<shared_ptr<Belief>> initial_beliefs;

    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>();
        belief->set_val(initial_states[i], MyFloat(1, this->precision));
        belief->obs = initial_states[i]->hybrid_state->classical_state->get_memory_val();
        initial_beliefs.push_back(belief);
    }

    return this->solve_strategy_beliefs(initial_beliefs, horizon);
}


pair<shared_ptr<MixedStrategy>, double> ConvexDistributionSolver::solve_strategy_beliefs(
    const vector<shared_ptr<Belief>> &initial_beliefs, const int &horizon) {

    cpp_int obs = -1;

    for (auto belief: initial_beliefs) {
        if (obs == -1) {
            obs = belief->obs;
        } else if (obs != belief->obs) {
            throw runtime_error("Beliefs have different observations");
        }
    }

    shared_ptr<Multibelief> multibelief = make_shared<Multibelief>(initial_beliefs, obs);

    auto strategies = this->get_matrix_maximin(multibelief, horizon);
    this->pomdp.actions.pop_back();

    return this->solve_lp_maximin(initial_beliefs.size(), strategies);
}

pair<shared_ptr<Algorithm>, double> ConvexDistributionSolver::solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs,
            const int &horizon) {

    auto temp = this->solve_strategy_beliefs(initial_beliefs, horizon);

    return make_pair(temp.first->to_algorithm(), temp.second);

}
