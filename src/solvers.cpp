#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <cassert>

using namespace std;

long long Solver::timelimit = 3600; // 1 hour timelimit

vector<shared_ptr<Multibelief>> Solver::get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action)  {
    // we first compute which beliefs we can reach for each belief in the multibelief
    vector<map<int, shared_ptr<Belief>>> successor_beliefs; // this vector should be (at the end) the same length as the multibelief.
                            // I.e., each index corresponds to the beliefs that can be reached by the corresponding belief
    set<int> reachable_obs;
    for (auto belief : current->beliefs) {
        if (belief->is_unreached) {
                successor_beliefs.push_back({});
        } else {
            assert(!belief->is_unreached);
            auto current_successors = this->get_successor_beliefs(belief, action);
            successor_beliefs.push_back(current_successors);
            for (auto successor_belief : current_successors) {
                reachable_obs.insert(successor_belief.first);
            }
        }

    }
    assert(successor_beliefs.size() == current->beliefs.size());

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
        auto new_multibelief = make_shared<Multibelief>(elements_multibelief, obs);
        result.push_back(new_multibelief);
    }
    return result;
}

shared_ptr<MWP> Solver::get_mwp(const shared_ptr<Multibelief> &multibelief, const shared_ptr<POMDPAction> &action) const {
    shared_ptr<MWP> current_mwp = make_shared<MWP>(multibelief->beliefs.size());
    int i = 0;
    for (auto belief: multibelief->beliefs) {
        current_mwp->values[i] = this->get_reward(belief, action);
        i+=1;
    }
    return current_mwp;
}

ParetoSolver::ParetoSolver(const POMDP &pomdp, const bool &convexify) {
    this->pomdp = pomdp;
    this->convexify = convexify;
    this->is_timeout = false;
    this->final_hull_size = 0;
}


shared_ptr<Hull> Solver::get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<shared_ptr<Hull>> &multibelief_points, int mb_index) { // mb_index (multibelief index)
    check_time();
    shared_ptr<Hull> current_points = make_shared<Hull>(current_score->values.size(), this->convexify);
    if (this->is_timeout) return current_points;

    for (auto current_mwp : multibelief_points[mb_index]->upper_hull) { // for each point current_mwp that a multibelief can reach, we create a new point new_score = current_score + current_mwp
        auto new_score = *current_score + *current_mwp;
        current_points->add_point(new_score);
    }

    if (mb_index == multibelief_points.size()-1) {
        // we are in the last set of points (of the multibeliefs that can be reached)
        return current_points;
    }

    shared_ptr<Hull> result = make_shared<Hull>(current_score->values.size(), this->convexify);
    // in this loop we combine each point generated (current_points) with all the points that the other multibeliefs can reach (thus mb_index+1)
    for (auto mwp : current_points->upper_hull) {
        auto succ_strategies = this->get_achievable_mwps(mwp,
            multibelief_points, mb_index +1);

        for (auto ss : succ_strategies->upper_hull) {
            result->add_point(ss);
        }
    }
    return result;
}

MyFloat Solver::get_reward(const shared_ptr<Belief> &b, const shared_ptr<POMDPAction> &action) const {
    if (b->is_unreached) return MyFloat(0.0);
     MyFloat result(0);
    for (auto p : b->probs) {
            result += p.second * this->pomdp.get_reward(p.first, action);
    }
    return result;
}

shared_ptr<Hull> ParetoSolver::get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon) {
    // this function returns all strategies that are reachable from the given beliefs and are of length horizon
    // we assume that all beliefs here have the same observation
    this->check_time();
    shared_ptr<Hull> result = make_shared<Hull>(multibelief->beliefs.size(), this->convexify);

    // consider strategy that halts immediately
    auto mwp_halt = get_mwp(multibelief, halt_action);
    result->add_point(mwp_halt);
    // ****************

    if (horizon == 0 || this->is_timeout) {
        return result;
    } else {
        assert(horizon > 0);
        for (auto action : this->pomdp.actions) {
            // compute reachable multibeliefs
            vector<shared_ptr<Multibelief>> multibelief_successors = this->get_multibelief_successors(multibelief, action);

            // compute the best mwps that can be achieved with any strategy with (horizon-1) for each successor multibelief
            vector<shared_ptr<Hull>> successor_points; // each successor multibelief has a set of best points

            for (auto succ_mb : multibelief_successors) {
                successor_points.push_back(this->get_points(succ_mb, horizon-1));
            }

            shared_ptr<MWP> current_score_ = this->get_mwp(multibelief, action); // initialize MWP filled with zeros
            if(successor_points.size()  == 0) {
                result->add_point(current_score_);
            } else {
                auto achievable_mwps = this->get_achievable_mwps(current_score_, successor_points); // we have to do an all vs all points
                if (achievable_mwps->upper_hull.size() == 0) {
                    result->add_point(current_score_);
                } else {
                    for (auto mwp : achievable_mwps->upper_hull) {
                        result->add_point(mwp);
                    }
                }
            }
        }

        return result;
    }

}

double Solver::solve_lp_maximin(const int &n_initial_states, const Hull& scores) {
    if (this->is_timeout) return -1;
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
    operations_research::MPVariable* v = solver.MakeNumVar(-INFINITY, INFINITY, "v");

    // Constraint: sum_i x_i = 1
    operations_research::MPConstraint* prob_sum = solver.MakeRowConstraint(1.0, 1.0);
    for (int i = 0; i < n_algorithms; ++i) {
        prob_sum->SetCoefficient(x[i], 1.0);
    }

    // Constraints: sum_i x_i * M_ij >= v  for all j
    for (int j = 0; j < n_initial_states; ++j) {
        operations_research::MPConstraint* c = solver.MakeRowConstraint(0.0, solver.infinity());
        int i = 0;
        for (auto mwp : scores.upper_hull) {
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
    auto result = solver.Solve();
    double final_value = v->solution_value();
    assert (result == operations_research::MPSolver::OPTIMAL);

    return final_value;
}

void Solver::check_time() {
    auto now = chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::duration<double>>(now-this->start_time).count() > Solver::timelimit) {
        this->is_timeout = true;
    }
}

map<int, shared_ptr<Belief>> Solver::get_successor_beliefs(const shared_ptr<Belief> &current_belief,
                                                           const shared_ptr<POMDPAction> &action) {
    assert (!(*action == *halt_action));

    map<int, shared_ptr<Belief>> obs_to_next_beliefs;
    assert(!current_belief->is_unreached);
    assert(current_belief->probs.size() > 0);
    map<int, int> obs_to_b_size;
    for(auto & prob : current_belief->probs) {
        auto current_v = prob.first;
        assert(prob.second > this->zero);
        for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
            assert(it_next_v.second > this->zero);
            auto successor = it_next_v.first;
            for (auto obs : this->pomdp.observations) {
                auto prob_obs = pomdp.get_obs_prob(action, successor, obs);
                if (prob_obs > zero) {
                    if (obs_to_next_beliefs.find(obs) == obs_to_next_beliefs.end()) {
                        obs_to_next_beliefs[obs] = make_shared<Belief>(obs);
                        obs_to_next_beliefs[obs]->obs = obs;
                    }
                    obs_to_next_beliefs[obs]->add_val(successor, prob.second * it_next_v.second * prob_obs);
                    obs_to_b_size[obs] = obs_to_next_beliefs[obs]->probs.size();
                }
            }
        }
    }

    for (auto p : obs_to_b_size) {
        if (p.second == 0) {
            obs_to_next_beliefs.erase(p.first);
        }
    }
    // assert(obs_to_next_beliefs.size() > 0);
    return obs_to_next_beliefs;
}

double Solver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon) {
    vector<shared_ptr<Belief>> initial_beliefs;

    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>(-1);
        belief->set_val(initial_states[i], MyFloat(1));
        initial_beliefs.push_back(belief);
    }

    return this->solve_beliefs(initial_beliefs, horizon);
}

double ParetoSolver::solve_beliefs(
    const vector<shared_ptr<Belief>> &initial_beliefs, const int &horizon) {

    shared_ptr<Multibelief> multibelief = make_shared<Multibelief>(initial_beliefs, -1);
    this->is_timeout = false;

    this->start_time = chrono::steady_clock::now();
    auto strategies = this->get_points(multibelief, horizon);
    auto solution = this->solve_lp_maximin(initial_beliefs.size(), *strategies);
    auto end_time = chrono::steady_clock::now();
    this->running_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - this->start_time).count();
    this->final_hull_size = strategies->size();
    return solution;
}

double QInspiredSolver::get_trans_prob(const int &from_m,
    const int &action_kernel, const int &to_m, const int &obs) {
    MyFloat result;

    auto from_m_v = this->all_multistates[from_m];
    auto action_kernel_ = this->all_action_kernels[action_kernel];
    auto to_m_v = this->all_multistates[to_m];


    MyFloat temp(1.0);
    for (int i = 0; i < from_m_v->size(); ++i) {
        temp *= pomdp.get_obs_prob(action_kernel_->mapping[obs], from_m_v->values[i], obs);
        temp *= pomdp.transition_matrix[from_m_v->values[i]][action_kernel_->mapping[obs]][to_m_v->values[i]];
    }
    result += temp;

    this->ms_transition_probs[from_m][action_kernel][to_m] = result.value;
    return result.value;
}

double QInspiredSolver::get_ms_reward(const shared_ptr<POMDPVertex> &v, const shared_ptr<ActionKernel> &ak) {
    MyFloat result;
    for (int current_obs : pomdp.observations) {
        result += pomdp.get_obs_prob(ak->mapping[current_obs], v, current_obs)*pomdp.get_reward(v, ak->mapping[current_obs]);
    }

    return result.value;
}

string QInspiredSolver::get_prob_var_name(const int &ms_index, const int &ak_index, const int &obs, const int &horizon) {
    return join({"P", to_string(ms_index), to_string(ak_index), to_string(obs), to_string(horizon)}, "_");
}

string QInspiredSolver::get_prob_ka_name(const int &ak_index, const int &horizon, const int &obs) {
    return join({"A", to_string(ak_index), to_string(horizon), to_string(obs)}, "_");
}

string QInspiredSolver::get_reward_var_name(const int &horizon, const int &initial_state_index) {
    return join({"R", to_string(horizon), to_string(initial_state_index)}, "_");
}

void QInspiredSolver::helper_get_multistates(Multistate &current, const int &size,
                                             vector<shared_ptr<Multistate>> &result, const vector<shared_ptr<POMDPVertex>> &states, const int &current_index) {
    this->check_time();
    if (this->is_timeout) return;
    if (size == 0) {
        result.push_back(make_shared<Multistate>(current));
        return;
    }

    for (int i = 0; i < states.size(); ++i) {
        auto v = states[i];
        current.values.push_back(v);
        this->helper_get_multistates(current, size-1, result, states, current_index + 1);
        current.values.pop_back();
    }
}

void QInspiredSolver::helper_get_kernels(ActionKernel &current, set<int>::iterator obs_it,
    vector<shared_ptr<ActionKernel>> &result) {
    this->check_time();
    if (this->is_timeout) return;
    if (obs_it == pomdp.observations.end()) {
        result.push_back(make_shared<ActionKernel>(current));
        return;
    }

    int current_obs = *obs_it;
    ++obs_it;
    for (auto action : this->pomdp.actions) {
        current.mapping[current_obs] = action;
        this->helper_get_kernels(current, obs_it, result);
    }
}

vector<shared_ptr<Multistate>> QInspiredSolver::get_all_multistates(const int &size, const int &max_horizon) {
    Multistate current;

    vector<shared_ptr<Multistate>> result;
    auto states = pomdp.get_reachable_states(max_horizon);
    cout << "num. reachable states: " << states.size() << endl;
    helper_get_multistates(current, size, result, states);
    return result;
}

vector<shared_ptr<ActionKernel>> QInspiredSolver::get_all_action_kernels() {

    set<int>::iterator current_obs =  this->pomdp.observations.begin();
    vector<shared_ptr<ActionKernel>> result;
    ActionKernel current;
    this->helper_get_kernels(current, current_obs, result);

    return result;
}

double QInspiredSolver::solve_lp(const vector<shared_ptr<Belief>> &initial_beliefs, const int &horizon) {
    this->all_multistates = this->get_all_multistates(initial_beliefs.size(), horizon);
    cout << "all multistates: " << all_multistates.size() << endl;

    this->all_action_kernels = this->get_all_action_kernels();
    cout <<"all action kernels: " << all_action_kernels.size() << endl;

    // assuming initial states only (not going to use initial beliefs) --> TODO
    Multistate initial_ms;
    int initial_ms_index = -1;
    for (auto v : pomdp.initial_states) {
        initial_ms.values.push_back(v);
    }

    bool found = false;
    for (auto ms : all_multistates) {
        initial_ms_index+=1;
        if (*ms == initial_ms) {
            found = true;
            break;
        }
    }

    assert(initial_ms_index >= 0 && found);
    if (this->is_timeout) return -1;
    operations_research::MPSolver solver("max_v", operations_research::MPSolver::GLOP_LINEAR_PROGRAMMING);
    solver.SetSolverSpecificParametersAsString(
    "primal_feasibility_tolerance:1e-9 dual_feasibility_tolerance:1e-9");

    // CREATING VARIABLES: transition probabilities
    cout << "started creating prob. variables" << endl;
    unordered_map<string, operations_research::MPVariable*> prob_vars;

    for (int k =0; k < horizon; ++k) {
        for (int m_index = 0; m_index < all_multistates.size(); ++m_index) {
            for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
                for (int obs : pomdp.observations) {
                    string name = this->get_prob_var_name(m_index, ak_index, obs, k);
                    assert(prob_vars.find(name) == prob_vars.end());
                    prob_vars[name] = solver.MakeNumVar(0.0, 1.0, name);
                }
            }
        }
    }

    // CREATING VARIABLES: kernel action probabilities
    cout << "started creating kernel action prob. variables" << endl;
    unordered_map<string, operations_research::MPVariable*> prob_kernels;

    for (int k =0; k < horizon; ++k) {
        for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
            for (auto obs : pomdp.observations) {
                string name = this->get_prob_ka_name(ak_index, k, obs);
                assert(prob_kernels.find(name) == prob_kernels.end());
                prob_kernels[name] = solver.MakeNumVar(0.0, 1.0, name);
            }
        }
    }

    // CREATING VARIABLES: rewards for each horizon
    cout << "started creating rewards" << endl;
    unordered_map<string, operations_research::MPVariable*> rewards;
    for (int k =0; k < horizon; ++k) {
        for (int initial_state_index = 0; initial_state_index < initial_ms.size(); initial_state_index++) {
            string name = this->get_reward_var_name(k, initial_state_index);
            rewards[name] = solver.MakeNumVar(-solver.infinity(), solver.infinity(), name);
        }
    }

    // CONSTRAINTS FOR HORIZON 0: put constraint on initial multistate
    cout << "started constraints init h=0" << endl;
    int initial_obs = *this->pomdp.observations.begin();
    operations_research::MPConstraint* k0_init_constraint = solver.MakeRowConstraint(1.0, 1.0);
    for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
        string name = this->get_prob_var_name(initial_ms_index, ak_index, initial_obs, 0);
        k0_init_constraint->SetCoefficient(prob_vars[name], 1.0);
    }

    // CONSTRAINTS FOR HORIZON 0: put constraint on the other multistates
    cout << "started constraints other h=0" << endl;
    for (int m_index = 0; m_index < all_multistates.size(); ++m_index) {

        for (auto obs : pomdp.observations) {
            if (m_index != initial_ms_index || obs != initial_obs) {
                for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
                    string name = this->get_prob_var_name(m_index, ak_index, obs, 0);
                    operations_research::MPConstraint* current_constraint = solver.MakeRowConstraint(0.0, 0.0);
                    current_constraint->SetCoefficient(prob_vars[name], 1.0);
                }
            }
        }
    }

    // CONSTRAINTS FOR ALL HORIZONS 0 < k < horizon (information flow): sum_a p_m_{a}_k = sum_{m',a'} p_{m'}_{a'}_{k-1}*Pr(m|m',a')  for all multistates m (right side is: ways to reack m from m' using kernel action a')
    cout << "started constraints  h>0" << endl;
    for (int k = 1; k < horizon; ++k) {
        for (int m_index = 0; m_index < all_multistates.size(); ++m_index) {
            for (auto current_obs : pomdp.observations) {
                operations_research::MPConstraint* current_constraint = solver.MakeRowConstraint(0.0, 0.0);
                for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
                    string name = this->get_prob_var_name(m_index, ak_index, current_obs, k);
                    current_constraint->SetCoefficient(prob_vars[name], 1.0);
                    for (int mm_index = 0; mm_index < all_multistates.size(); ++mm_index) {
                        for (int ak_prev = 0; ak_prev < all_action_kernels.size(); ++ak_prev) {
                            // -p(m', a'_{k-1}) * Pr(m | m', a')
                            for (auto prev_obs : pomdp.observations) {
                                string prev_name = this->get_prob_var_name(mm_index, ak_prev, prev_obs, k-1);
                                current_constraint->SetCoefficient(
                                    prob_vars[prev_name],
                                    -1.0 * this->get_trans_prob(mm_index, ak_prev, m_index, current_obs)
                                );
                            }
                        }
                    }
                }
            }
        }
    }

    // constraint action per observation
    for (int k = 1; k < horizon; ++k) {
        for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
            operations_research::MPConstraint* outer_constraint = solver.MakeRowConstraint(0.0, 0.0);
            for (int current_obs : pomdp.observations) {
                operations_research::MPConstraint* current_constraint = solver.MakeRowConstraint(0.0, 0.0);
                string ak_name = this->get_prob_ka_name(ak_index, k, current_obs);
                current_constraint->SetCoefficient(prob_kernels[ak_name], 1.0);
                for (int m_index = 0; m_index < all_multistates.size(); ++m_index) {
                    string name = this->get_prob_var_name(m_index, ak_index, current_obs, k);
                    current_constraint->SetCoefficient(prob_vars[name], -1.0);
                }

                outer_constraint->SetCoefficient(prob_kernels[ak_name], 1.0);
            }

        }
    }

    // CONSTRAINTS OF REWARDS at ALL HORIZONS 0 < k < horizon
    cout << "started constraints rewards" << endl;
    for (int k = 0; k < horizon; ++k) {
        for (int init_state_index = 0; init_state_index < initial_ms.size(); ++init_state_index) {
            operations_research::MPConstraint* current_constraint = solver.MakeRowConstraint(0.0, 0.0);
            auto reward_name = get_reward_var_name(k, init_state_index);
            current_constraint->SetCoefficient(rewards[reward_name], 1.0);
            for (int m_index = 0; m_index < all_multistates.size(); ++m_index) {
                for (int ak_index = 0; ak_index < all_action_kernels.size(); ++ak_index) {
                    for (auto obs : pomdp.observations) {
                        string name = this->get_prob_var_name(m_index, ak_index, obs, k);
                        current_constraint->SetCoefficient(prob_vars[name], -1.0*this->get_ms_reward(all_multistates[m_index]->values[init_state_index], all_action_kernels[ak_index]));
                    }
                }
            }

            if (k > 0) {
                auto other_reward_name = get_reward_var_name(k-1, init_state_index);
                current_constraint->SetCoefficient(rewards[other_reward_name], -1);
            }
        }
    }

    // declare objective variable: maximize v (min reward at target horizon)
    operations_research::MPVariable* v = solver.MakeNumVar(-INFINITY, INFINITY, "v");

    // put constraints on rewards at target horizon
    cout << "started final reward constraints" << endl;
    for (int j = 0; j < initial_ms.size(); ++j) {
        operations_research::MPConstraint* c = solver.MakeRowConstraint(0.0, solver.infinity());
        string reward_name = get_reward_var_name(horizon-1, j);
        c->SetCoefficient(rewards[reward_name], 1);

        c->SetCoefficient(v, -1.0); // sum_i(...) - v >= 0  → sum_i(...) >= v
    }
    cout << "end final reward constraints" << endl;

    operations_research::MPObjective* objective = solver.MutableObjective();
    objective->SetCoefficient(v, 1.0);
    objective->SetMaximization();

    // Solve
    cout << "started solving" << endl;
    auto result = solver.Solve();
    double final_value = v->solution_value();
    if (result != operations_research::MPSolver::OPTIMAL) {
        this->is_timeout = true;
        return -1;
    }

    return final_value;
}

QInspiredSolver::QInspiredSolver(const POMDP &pomdp) {
    this->pomdp = pomdp;
    this->convexify = false;
    this->is_timeout = false;
}

double QInspiredSolver::solve_beliefs(const vector<shared_ptr<Belief>> &initial_beliefs, const int &horizon) {
    this->all_action_kernels.clear();
    this->all_multistates.clear();
    this->ms_transition_probs.clear();
    this->is_timeout = false;
    this->start_time = chrono::steady_clock::now();

    auto solution = this->solve_lp(initial_beliefs, horizon);
    auto end_time = chrono::steady_clock::now();

    this->running_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - this->start_time).count();

    return solution;

}
