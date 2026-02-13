#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <iostream>

#include <cassert>

bool SingleDistributionSolver::is_belief_visited(const Belief &belief) const {
    for (const auto& it : this->beliefs_to_rewards) {
        if (it.first == belief) {
                return true;
        }
    }
    return false;
}

MyFloat SingleDistributionSolver::get_closest_L1(const Belief &belief) const {
    MyFloat result("-1", this->precision);
    bool first = true;
    for (const auto& it : this->beliefs_to_rewards) {
        auto existing_belief = it.first;
        auto current_l1_norm = l1_norm(existing_belief, belief, this->precision);
        if (first || result > current_l1_norm) {
            first = false;
            result = current_l1_norm;
        }
    }

    if (first) {
        assert (this->beliefs_to_rewards.empty());
        return MyFloat("0", this->precision);
    }

    return result;
}

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
            // this->beliefs_to_rewards[curr_belief_normalized][horizon] = bellman_value;
            return bellman_value;
        }
    }
    assert(false);
}


pair<shared_ptr<Algorithm>, MyFloat> SingleDistributionSolver::PBVI_solve(const Belief &current_belief, const int &horizon) {
    Belief curr_belief_normalized = normalize_belief(current_belief, this->precision);
    auto temp_it = this->beliefs_to_rewards.find(curr_belief_normalized);
    if (temp_it != this->beliefs_to_rewards.end()) {
        auto horizon_it = temp_it->second.find(horizon);
        if (horizon_it != temp_it->second.end()) {
            return horizon_it->second;
        }
    } else {
        this->beliefs_to_rewards.insert({curr_belief_normalized,{}
    });
    }

    MyFloat curr_belief_val = this->get_reward(current_belief, this->embedding);

    cpp_int current_classical_state = -1;
    for(auto & prob : current_belief.probs) {
        if (current_classical_state == -1) {
            current_classical_state = prob.first->hybrid_state->classical_state->get_memory_val();
        } else {
            assert(prob.first->hybrid_state->classical_state->get_memory_val() == current_classical_state);
        }
    }
    assert(current_classical_state >= 0);
    auto halt_algorithm = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), current_classical_state, 0);
    if (horizon == 0) {
        return make_pair(halt_algorithm, curr_belief_val);
    }

    vector<pair<Belief, map<int, Belief>>> candidate_beliefs;

    // for picking candidate belief that has the highest distance
    unordered_map<cpp_int, Belief> temp;
    temp[current_classical_state] = current_belief;
    pair<Belief, unordered_map<cpp_int, Belief>> best_candidate = make_pair(curr_belief_normalized, temp);
    MyFloat furthest_value("0", this->precision);
    auto best_action = make_shared<POMDPAction>(HALT_ACTION);


    for (auto & it : pomdp.actions) {
        const auto& action = it;

        unordered_map<cpp_int, Belief> obs_to_next_beliefs; // for each belief we compute the sub-distributions to which it leads
        Belief next_belief; // we also compute the real belief which should be normalized

        for(auto & prob : current_belief.probs) {
            MyFloat zero("0", this->precision);
            auto current_v = prob.first;

            assert(prob.second > zero);
            if (pomdp.transition_matrix.find(current_v) != pomdp.transition_matrix.end()) {
                if (pomdp.transition_matrix.at(current_v).find(action) != pomdp.transition_matrix.at(current_v).end()) {
                    for (const auto &it_next_v: pomdp.transition_matrix.at(current_v).at(action)) {
                        if (it_next_v.second > zero) {
                            auto successor = it_next_v.first;
                            obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(successor,
                                                                                  prob.second * it_next_v.second);
                            next_belief.add_val(successor,prob.second * it_next_v.second);
                        }
                    }
                }
            }
        }

        next_belief = normalize_belief(next_belief, this->precision);

        MyFloat closest_value = this->get_closest_L1(next_belief);
        if (furthest_value < closest_value) {
            furthest_value = closest_value;
            best_candidate = make_pair(next_belief, obs_to_next_beliefs);
            best_action = action;
        }

    }

    auto new_alg_node = make_shared<Algorithm>(best_action, current_classical_state, this->precision);
    MyFloat bellman_val("0", this->precision);
    if (best_action->name == HALT_ACTION.name) {
        bellman_val = curr_belief_val;
    } else {
        int max_depth = 0;
        for(auto & obs_to_next_belief : best_candidate.second) {
            auto temp2 = this->PBVI_solve(obs_to_next_belief.second, horizon-1);
            new_alg_node->children.push_back(temp2.first);
            max_depth = max(temp2.first->depth, max_depth);
            bellman_val = bellman_val + temp2.second;
        }
        new_alg_node->depth = max_depth + 1;
    }

    this->beliefs_to_rewards.at(curr_belief_normalized).insert({horizon, {new_alg_node, bellman_val}});
    this->error = max(this->error, furthest_value);
    return this->beliefs_to_rewards.at(curr_belief_normalized).at(horizon);
}

double SingleDistributionSolver::get_error(const int &horizon) const {
    return to_double(this->error) * horizon;
}

void SingleDistributionSolver::print_all_beliefs() const {

    for (auto it : this->beliefs_to_rewards) {
        it.first.print();
    }
}


ConvexDistributionSolver::ConvexDistributionSolver(const POMDP &pomdp, const f_reward_type &precise_get_reward, const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> & embedding, const guard_type &g) {
    this->pomdp = pomdp;
    this->get_reward = get_reward;
    this->precise_get_reward = precise_get_reward;
    this->precision = precision;
    this->embedding = embedding;
    this->initial_classical_state = -1;
    this->guard = g;
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

void ConvexDistributionSolver::set_minimax_values(
    const shared_ptr<Algorithm> &algorithm,
    const vector<shared_ptr<POMDPVertex>> &initial_states,
    unordered_map<int, unordered_map<int, double>> &minimax_matrix,
    unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm) {
    
    // Algorithm *old_algorithm = algorithm_exists(mapping_index_algorithm, algorithm);
    // if(old_algorithm != nullptr) {
    //     write_algorithm_file(old_algorithm, "temp_old.json");
    //     write_algorithm_file(algorithm, "temp_new.json");
    //     assert(false);
    // }

    int current_alg_index = minimax_matrix.size();

    mapping_index_algorithm[current_alg_index] = algorithm;
    
    // the current algorithm index should not exist
    assert(minimax_matrix.find(current_alg_index) == minimax_matrix.end());
    minimax_matrix[current_alg_index] = unordered_map<int, double>();

    for (int index = 0; index < initial_states.size(); index++) {
        assert(minimax_matrix[current_alg_index].find(index) == minimax_matrix[current_alg_index].end());

        Belief initial_belief;
        initial_belief.set_val(initial_states[index], MyFloat("1", this->precision));
        auto acc = get_algorithm_acc(pomdp, algorithm, initial_belief, this->precise_get_reward, this->embedding, this->precision);
        minimax_matrix[current_alg_index][index] = to_double(acc);

    }
    // cout << current_alg_index << endl;
    // fs::path p = fs::path("..") / "temp" / ("a" + to_string(current_alg_index) + ".txt");
    // dump_to_file(p, algorithm);
}

bool ConvexDistributionSolver::is_action_allowed(shared_ptr<POMDPAction> &action,
                                                 const vector<shared_ptr<POMDPVertex>> &states) {
    for (auto state : states) {
        if (this->guard(state, this->embedding, action)) {
            return true;
        }
    }
    return false;
}


cpp_int get_succ_classical_state(const shared_ptr<Algorithm> &current) {
    assert(!current->has_meas());
    if (current->is_unitary()) {
        return current->classical_state;
    }

    assert(current->has_classical_instruction());
    assert(!current->has_meas());

    cpp_int answer = current->classical_state;
    for(const Instruction& instruction : current->action->instruction_sequence) {
        
        if(instruction.instruction_type == InstructionType::Classical) {
            if (instruction.gate_name == GateName::Write1) {
                answer = answer | (1 << instruction.c_target);
            } else {
                assert(instruction.gate_name == GateName::Write0);
                answer &= ~(1 << instruction.c_target);
            }
        }
    }
    return answer;
}

unordered_map<cpp_int, vector<shared_ptr<POMDPVertex>>> get_reachable_states(const POMDP &pomdp, const vector<shared_ptr<POMDPVertex>>&current_states,
    const shared_ptr<POMDPAction> &action) {
    unordered_set<shared_ptr<POMDPVertex>, POMDPVertexHash, POMDPVertexPtrEqualID> reachable_vertices;
    unordered_map<cpp_int, vector<shared_ptr<POMDPVertex>>> result;

    for (auto curr_state : current_states) {
        auto curr_state_it = pomdp.transition_matrix.find(curr_state);
        if (curr_state_it != pomdp.transition_matrix.end()) {
            auto action_it = curr_state_it->second.find(action);
            if (action_it != curr_state_it->second.end()) {
                for (auto succ_state_it : action_it->second) {
                    auto succ_state = succ_state_it.first;
                    if (reachable_vertices.find(succ_state) == reachable_vertices.end()) {
                        reachable_vertices.insert(succ_state);
                        cpp_int succ_cstate = succ_state->hybrid_state->classical_state->get_memory_val();
                        if (result.find(succ_cstate) == result.end()) {
                            result.insert(make_pair(succ_cstate, vector<shared_ptr<POMDPVertex>>()));
                        }
                        result[succ_cstate].push_back(succ_state);
                    }
                }
            }
        }
    }

    return result;

}

void ConvexDistributionSolver::get_matrix_maximin(const vector<shared_ptr<POMDPVertex>> &initial_states,
    const shared_ptr<Algorithm> &current_algorithm,
    unordered_map<int, unordered_map<int, double>> &minimax_matrix,
    const int &max_horizon,
    unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm) {

    if (current_algorithm == nullptr) {
        for (auto action : pomdp.actions) {
            if (is_action_allowed(action, initial_states)) {
                auto new_node = make_shared<Algorithm>(action, this->initial_classical_state, this->precision, 1); // assumes initial classical state is 0
                new_node->reachable_states = initial_states;
                get_matrix_maximin(initial_states, new_node, minimax_matrix, max_horizon, mapping_index_algorithm);
            }
        }
    } else {
        auto copy_curr_algorithm = normalize_algorithm(current_algorithm);
        if(algorithm_exists(mapping_index_algorithm, copy_curr_algorithm) != -1) {
            return;
        }
        this->set_minimax_values(copy_curr_algorithm, initial_states, minimax_matrix, mapping_index_algorithm);

        if (*current_algorithm->action == HALT_ACTION) {
            return;
        }
        vector<shared_ptr<Algorithm>> end_nodes;
        get_algorithm_end_nodes(current_algorithm, end_nodes);
        for (const auto& end_node : end_nodes) {
            if (end_node->depth < max_horizon) {
                unordered_set<cpp_int> successor_cstates;
                cpp_int max_succ_cstate = -1;
                for (auto child_it : end_node->children) {
                    assert(child_it->classical_state >= 0);
                    max_succ_cstate = max(max_succ_cstate, child_it->classical_state);
                    successor_cstates.insert(child_it->classical_state);
                }
                assert(end_node->reachable_states.size() > 0);
                unordered_map<cpp_int, vector<shared_ptr<POMDPVertex>>> reachable_states = get_reachable_states(this->pomdp, end_node->reachable_states, end_node->action);

                for (auto action : pomdp.actions) {
                    for (auto it_reach : reachable_states) {
                        if (is_action_allowed(action, it_reach.second)) {
                            auto next_cstate = it_reach.first;
                            if (successor_cstates.find(next_cstate) == successor_cstates.end()) {
                                if (next_cstate > max_succ_cstate) {
                                    auto new_node = make_shared<Algorithm>(action, next_cstate, this->precision,end_node->depth + 1);
                                    new_node->reachable_states = it_reach.second;
                                    end_node->children.push_back(new_node);
                                    get_matrix_maximin(initial_states, current_algorithm, minimax_matrix, max_horizon, mapping_index_algorithm);
                                    end_node->children.pop_back();
                                }
                            }
                        }
                    }

                }
            }
        }
    }
}

cpp_int get_classical_state(const vector<shared_ptr<POMDPVertex>> &states) {
    cpp_int answer = -1;
    for (auto state: states) {
        if (answer == -1) {
            answer = state->hybrid_state->classical_state->get_memory_val();
        } else {
            assert(answer == state->hybrid_state->classical_state->get_memory_val());
        }
    }

    return answer;
}

pair<vector<double>, double> ConvexDistributionSolver::solve_lp_maximin(const unordered_map<int, unordered_map<int, double>> &maximin_matrix, const int &n_algorithms, const int &n_initial_states) {
    operations_research::MPSolver solver("max_v", operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING);
    solver.SetSolverSpecificParametersAsString(
    "primal_feasibility_tolerance:1e-9 dual_feasibility_tolerance:1e-9");
    // Variables: x_i >= 0
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
            auto temp = *maximin_matrix.find(i);
            c->SetCoefficient(x[i], ((temp.second.find(j)))->second);
        }
        c->SetCoefficient(v, -1.0); // sum_i(...) - v >= 0  → sum_i(...) >= v
    }

    // Objective: maximize v
    operations_research::MPObjective* objective = solver.MutableObjective();
    objective->SetCoefficient(v, 1.0);
    objective->SetMaximization();

    // Solve
    auto result = solver.Solve();
    vector<double> mixed_algorithm;
    // cout << "v: " << v->solution_value() << endl;
    double final_value = v->solution_value();
    if (result == operations_research::MPSolver::OPTIMAL) {
        for (int i = 0; i < n_algorithms; ++i) {
            mixed_algorithm.push_back(x[i]->solution_value());
        }
    }

    return make_pair(mixed_algorithm, final_value);
}

pair<shared_ptr<Algorithm>, double> ConvexDistributionSolver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon) {
    this->pomdp.actions.push_back(make_shared<POMDPAction>(HALT_ACTION));
    unordered_map<int, unordered_map<int, double>> maximin_matrix;
    unordered_map<int, shared_ptr<Algorithm>> mapping_index_algorithm;
    
    this->initial_classical_state = get_classical_state(initial_states);
    this->get_matrix_maximin(initial_states, nullptr, maximin_matrix, horizon, mapping_index_algorithm);

    auto result = this->solve_lp_maximin(maximin_matrix, maximin_matrix.size(), initial_states.size());

    // check result
    double sum_ = 0.0;
    auto val = result.first;
    for (int i = 0 ; i < val.size() ; i++) {
        sum_ += val[i];
    }

    if (!is_close(sum_, 1.0, 8)) {
        throw runtime_error("sum_ is incorrect: " + to_string(sum_));
    }
    for (int i = 0 ; i < result.first.size() ; i++) {
        result.first[i] /= sum_;
    }
    auto mixed_algorithm = get_mixed_algorithm(result.first, mapping_index_algorithm, this->initial_classical_state);
    this->pomdp.actions.pop_back();
    return make_pair(mixed_algorithm, result.second);
}

set<shared_ptr<Point>> AntichainSolver::get_antichain(const vector<Belief> &beliefs, const int &horizon) {
    assert(beliefs.size() <= 2);
    assert(beliefs[0].get_obs() == beliefs[1].get_obs());
    auto curr_cstate = beliefs[0].get_obs();
    auto halt_algorithm = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), curr_cstate, 0);
    set<shared_ptr<Point>> antichain;
    auto current_point = get_point(beliefs, halt_algorithm);
    this->insert_point(antichain, current_point);
    if (horizon > 0) {
        for (auto action : this->pomdp.actions) {
            vector<map<cpp_int, Belief>> all_obs_to_beliefs;
            for (auto belief : beliefs) {
                all_obs_to_beliefs.push_back(this->get_next_beliefs(belief, action));
            }
            map<cpp_int, int> obs_count;

            for (auto obs_to_belief : all_obs_to_beliefs) {
                for (auto pair_ : obs_to_belief) {
                    if (obs_count.find(pair_.first) != obs_count.end()) {
                        obs_count[pair_.first] += 1;
                    } else {
                        obs_count[pair_.first] = 1;
                    }
                }
            }

            assert (all_obs_to_beliefs.size() == 2);

            map<cpp_int, set<shared_ptr<Point>>> obs_to_points;
            for (auto pair_ : obs_count) {
                if (pair_.second == 1) {
                    SingleDistributionSolver bellman_solver(this->pomdp, this->precise_get_reward, this->precision, this->embedding);
                    Belief belief;
                    int index;
                    if (all_obs_to_beliefs[0].find(pair_.first) != all_obs_to_beliefs[0].end()) {
                        belief = all_obs_to_beliefs[0][pair_.first];
                        index = 0;
                    } else {
                        assert(all_obs_to_beliefs[1].find(pair_.first) != all_obs_to_beliefs[1].end());
                        belief = all_obs_to_beliefs[1][pair_.first];
                        index = 1;
                    }
                    auto result = bellman_solver.get_bellman_value(belief, horizon-1);
                    vector<MyFloat> values;
                    if (index == 0) {
                        values[1] = MyFloat(0, this->precision);
                        values[index] = result.second;
                    } else {
                        values[0] = MyFloat(0, this->precision);
                        values[index] = result.second;
                    }

                    auto point = make_shared<Point>(values, result.first);
                    set<shared_ptr<Point>> current_points;
                    current_points.insert(point);
                    obs_to_points[pair_.first] = current_points;
                } else {
                    assert(pair_.second == 2);
                    // this is an observation that appears in both worlds.
                    vector<Belief> next_beliefs;
                    next_beliefs.push_back(all_obs_to_beliefs[0][pair_.first]);
                    next_beliefs.push_back(all_obs_to_beliefs[1][pair_.first]);
                    auto reached_points = this->get_antichain(next_beliefs, horizon-1);
                    obs_to_points[pair_.first] = reached_points;
                }
            }

            shared_ptr<Algorithm> current_algorithm = make_shared<Algorithm>(action, beliefs[0].get_obs(), this->precision);
            shared_ptr<Point> curr_point = make_shared<Point>(current_algorithm, beliefs.size());

            this->combine_obs_points(obs_to_points.begin(),
                obs_to_points.end(), curr_point, antichain);
        }
    }
    return antichain;
}

shared_ptr<Point> AntichainSolver::get_point(const vector<Belief> &beliefs, const shared_ptr<Algorithm> &algorithm) {
    vector<MyFloat> values;
    for (auto belief : beliefs) {
        values.push_back(this->precise_get_reward(belief, this->embedding));
    }
    return make_shared<Point>(values, algorithm);
}

map<cpp_int, Belief> AntichainSolver::get_next_beliefs(const Belief &curr_belief, const shared_ptr<POMDPAction> &action) {
    map<cpp_int, Belief> obs_to_next_beliefs;
    const MyFloat zero("0", this->precision);

    for (auto &prob : curr_belief.probs) {
        auto current_v = prob.first;
        assert(prob.second > zero);
        for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
            if (it_next_v.second > zero) {
                auto successor = it_next_v.first;
                obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(successor,
                                                                          prob.second * it_next_v.second);
            }
        }
    }

    return obs_to_next_beliefs;
}

void AntichainSolver::combine_obs_points(const map<cpp_int, set<shared_ptr<Point>>>::iterator &current_obs,
    const map<cpp_int, set<shared_ptr<Point>>>::iterator &iterator_end, const shared_ptr<Point> &curr_point_, set<shared_ptr<Point>> &antichain) {

    if (current_obs == iterator_end) {
        this->insert_point(antichain, curr_point_);
    } else {
        for (auto new_point : current_obs->second) {
            auto curr_point = deep_copy(curr_point_);
            *(curr_point)+=(*new_point);
            combine_obs_points(next(current_obs), iterator_end, curr_point, antichain);
        }
    }
}

shared_ptr<Point> deep_copy(const shared_ptr<Point> &point, const int& precision) {
    vector<MyFloat> values;
    MyFloat ZERO = MyFloat("0", precision);
    for (auto val : point->values) {
        values.push_back(val + ZERO); // force a new float
    }

    return make_shared<Point>(values, deep_copy_algorithm(point->algorithm));
}


pair<shared_ptr<Algorithm>, MyFloat> AntichainSolver::solve_exact(const vector<shared_ptr<POMDPVertex>> &initial_states, const int &horizon) {
    this->pomdp.actions.push_back(make_shared<POMDPAction>(HALT_ACTION));

    vector<Belief> beliefs;
    for (const auto& initial_state : initial_states) {
        beliefs.emplace_back(initial_state, this->precision);
    }

    auto points = this->get_antichain(beliefs, horizon);
    return this->get_algorithm(points);
}


