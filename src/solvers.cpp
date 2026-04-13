#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <cassert>

using namespace std;
namespace mp = boost::multiprecision;

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

ConvexSolverPSPACE::ConvexSolverPSPACE(const POMDP &pomdp, const f_reward_type &precise_get_reward,
                                                   const f_reward_type_double &get_reward, int precision, const unordered_map<int, int> &embedding) {
    this->pomdp = pomdp;
    this->get_reward = get_reward;
    this->precise_get_reward = precise_get_reward;
    this->precision = precision;
    this->embedding = embedding;
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

bool ConvexDistributionSolverHull::update_pareto_front(const shared_ptr<MWP> &mwp,  set<shared_ptr<MWP>, MWPPtrComp>&scores) {
    auto zero = MyFloat("0", this->precision);
    if (ConvexDistributionSolver::update_pareto_front(mwp, scores)) {
        // convexify pareto front
        auto it = scores.find(mwp);
        while (true) {
            if (it == scores.begin()) break;

            auto b = it;
            auto a = std::prev(b);
            if (a == scores.begin()) break;

            auto a2 = std::prev(a);

            auto ax = (*a)->values[0];
            auto ay = (*a)->values[1];
            auto a2x = (*a2)->values[0];
            auto a2y = (*a2)->values[1];
            a2x.is_negative = true;
            a2y.is_negative = true;
            auto bx = (*b)->values[0];
            auto by = (*b)->values[1];

            auto temp = (ay + a2y) * (bx + a2x);
            temp.is_negative = true;
            auto cross =
                (ax + a2x) * (by + a2y) + temp;
            if (cross == zero || cross > zero ) {
                scores.erase(a); // remove middle point
            } else break;
        }
        return true;
    }
    return false;
}

vector<shared_ptr<MWP>> ConvexDistributionSolver::get_final_strategies(shared_ptr<MWP> &current_score, const vector<set<shared_ptr<MWP>, MWPPtrComp>> &m_strategy_score, int from_index) {
    vector< shared_ptr<MWP>> temp;
    for (auto current_m : m_strategy_score[from_index]) {
        auto new_score = *current_score + *current_m;
        temp.push_back(new_score);
    }

    if (from_index == m_strategy_score.size()-1) {
        return temp;
    }

    vector<shared_ptr<MWP>> result;
    for (auto mwp : temp) {
        auto succ_strategies = this->get_final_strategies(mwp,
            m_strategy_score, from_index +1);

        for (auto ss : succ_strategies) {
            result.push_back(ss);
        }
    }
    return result;
}

bool ConvexDistributionSolver::update_pareto_front(const shared_ptr<MWP> &mwp,  set<shared_ptr<MWP>, MWPPtrComp>&scores) {
    unordered_set<shared_ptr<MWP>> to_remove;
    for (auto p : scores) {
        if (*mwp <= *p) {
            assert(to_remove.empty());
            return false;
        }

        if (*p <= *mwp) {
            to_remove.insert(p);
        }
    }

    scores.insert(mwp);
    for (auto r_strat : to_remove) {
        scores.erase(r_strat);
    }
    return true;
}

 set<shared_ptr<MWP>, MWPPtrComp> ConvexDistributionSolver::get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon) {
    // this function returns all strategies that are reachable from the given beliefs and are of length horizon
    // we assume that all beliefs here have the same observation

     set<shared_ptr<MWP>, MWPPtrComp> result;

    // consider strategy that halts immediatly
    auto mwp_halt = get_mwp(multibelief);
    result.insert(mwp_halt);
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
                vector< set<shared_ptr<MWP>, MWPPtrComp>> succ_strategies;

                for (auto succ_mb : multibelief_successors) {
                    succ_strategies.push_back(this->get_points(succ_mb, horizon-1));
                }

                shared_ptr<MWP> current_score_ = make_shared<MWP>(multibelief->beliefs.size(), this->precision);
                vector<shared_ptr<MWP>> new_strategies = this->get_final_strategies(current_score_, succ_strategies);

                for (auto current_score : new_strategies) {
                    // update set of strategies
                    this->update_pareto_front(current_score, result);
                }
            }
        }

        return result;
    }

}

double ConvexDistributionSolver::solve_lp_maximin(const int &n_initial_states, const  set<shared_ptr<MWP>, MWPPtrComp>& scores) {
    operations_research::MPSolver solver("max_v", operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING);
    solver.SetSolverSpecificParametersAsString(
    "primal_feasibility_tolerance:1e-9 dual_feasibility_tolerance:1e-9");
    // Variables: x_i >= 0
    int n_algorithms = scores.size();

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
        int i = 0;
        for (auto mwp : scores) {
            assert(mwp->values.size() == n_initial_states);
            c->SetCoefficient(x[i], mwp->get(j));
            i+=1;
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
    double final_value = v->solution_value();
    assert (result == operations_research::MPSolver::OPTIMAL);

    if (!is_close(sum_, 1.0, 8)) {
        throw runtime_error("sum_ is incorrect: " + to_string(sum_));
    }

    return final_value;
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

double ConvexDistributionSolver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon) {
    this->pomdp.actions.push_back(this->halt_action);

    vector<shared_ptr<Belief>> initial_beliefs;

    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>();
        belief->set_val(initial_states[i], MyFloat(1, this->precision));
        belief->obs = initial_states[i]->hybrid_state->classical_state->get_memory_val();
        initial_beliefs.push_back(belief);
    }

    return this->solve_beliefs(initial_beliefs, horizon);
}

double ConvexSolverPSPACE::solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon) {
    vector<shared_ptr<Belief>> initial_beliefs;

    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>();
        belief->set_val(initial_states[i], MyFloat(1, this->precision));
        belief->obs = initial_states[i]->hybrid_state->classical_state->get_memory_val();
        initial_beliefs.push_back(belief);
    }

    return this->solve_beliefs(initial_beliefs, horizon);
}


double ConvexDistributionSolver::solve_beliefs(
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

    auto strategies = this->get_points(multibelief, horizon);
    this->pomdp.actions.pop_back();

    return this->solve_lp_maximin(initial_beliefs.size(), strategies);
}

double ConvexSolverPSPACE::solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs, const int &horizon) {
    cpp_int obs = -1;

    for (auto belief: initial_beliefs) {
        if (obs == -1) {
            obs = belief->obs;
        } else if (obs != belief->obs) {
            throw runtime_error("Beliefs have different observations");
        }
    }

    shared_ptr<Multibelief> multibelief = make_shared<Multibelief>(initial_beliefs, obs);

    // do binary search over lambda
    return this->find_lambda(multibelief, horizon);
}

double ConvexSolverPSPACE::find_lambda(const shared_ptr<Multibelief> &multibelief, const int &horizon, const double &bottom, const double &top) {

    if (bottom > top) {
        return -1;
    }

    double middle = round_to((bottom+top)/2, this->precision);

    if (this->is_feasible(multibelief, horizon, middle)) {
        auto temp = this->find_lambda(multibelief, horizon, middle + 1/pow(10, this->precision), top);
        if (temp >= 0) {
            return temp;
        }
        return middle;
    }

    return this->find_lambda(multibelief, horizon, bottom, middle - 1/pow(10, this->precision));
}

bool ConvexSolverPSPACE::is_feasible(const shared_ptr<Multibelief> &multibelief, const int &horizon, const int &lambda) {
    cpp_int max_num = mp::pow(mp::cpp_int(10), this->precision*multibelief->beliefs.size());
    for (cpp_int i = 0; i <= max_num; i++) {
        shared_ptr<MWP> target_mwp = this->get_target_mwp(i);

    }
}
