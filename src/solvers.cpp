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
    shared_ptr<Strategy> strategy = make_shared<Strategy>(action, multibelief->get_obs()); // TODO (optimization): avoid creation of new strategy here
    shared_ptr<MWP> current_mwp = make_shared<MWP>(multibelief->beliefs.size(), strategy);
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
    this->final_hull_size = -1;
}


shared_ptr<Hull> Solver::get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<shared_ptr<Hull>> &multibelief_points, int mb_index) { // mb_index (multibelief index)
    check_time();
    shared_ptr<Hull> current_points = make_shared<Hull>(current_score->values.size(), this->convexify);
    if (this->is_timeout) return current_points;

    for (auto current_mwp : multibelief_points[mb_index]->upper_hull) { // for each point current_mwp that a multibelief can reach, we create a new point new_score = current_score + current_mwp
        auto new_score = current_score->add_mwp(current_mwp);
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
    auto mwp_halt = get_mwp(multibelief, HALT_ACTION);
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

            shared_ptr<MWP> root = this->get_mwp(multibelief, action); // initialize MWP filled with zeros
            if(successor_points.size()  == 0) {
                result->add_point(root);
            } else {
                shared_ptr<MWP> current_score_ = make_shared<MWP>(multibelief->beliefs.size(), TEMP_STRATEGY); // initialize MWP filled with zeros
                auto achievable_mwps = this->get_achievable_mwps(current_score_, successor_points); // we have to do an all vs all points
                if (achievable_mwps->upper_hull.size() == 0) {
                    result->add_point(current_score_);
                } else {
                    for (auto mwp : achievable_mwps->upper_hull) {
                        result->add_point(root->add_mwp(mwp, true));
                    }
                }
            }
        }

        return result;
    }

}

pair<shared_ptr<MixedStrategy>, double> Solver::solve_lp_maximin(const int &n_initial_states, const Hull& scores) const {
    if (this->is_timeout) return make_pair(NO_SOLUTION_MIX_STRAT, -1);
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

    // Result
    double final_value = v->solution_value();
    assert (result == operations_research::MPSolver::OPTIMAL);


    vector<pair<shared_ptr<Strategy>, double>> v_probs_strats;
    int i = 0;
    for (auto mwp : scores.upper_hull) {
        auto prob = x[i]->solution_value();
        prob = round_to(prob);
        if (prob > 0) {
            v_probs_strats.push_back(make_pair(mwp->strategy, prob));
        } else {
            assert(prob == 0);
        }
        i+=1;
    }



    return make_pair(make_shared<MixedStrategy>(v_probs_strats), final_value);
}

void Solver::check_time() {
    auto now = chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::duration<double>>(now-this->start_time).count() > Solver::timelimit) {
        this->is_timeout = true;
    }
}

map<int, shared_ptr<Belief>> Solver::get_successor_beliefs(const shared_ptr<Belief> &current_belief,
                                                           const shared_ptr<POMDPAction> &action) {
    assert (!(*action == *HALT_ACTION));

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

pair<shared_ptr<MixedStrategy>, double> Solver::solve(const vector<shared_ptr<POMDPVertex>> &initial_states,
            const int &horizon) {
    vector<shared_ptr<Belief>> initial_beliefs;

    for (int i = 0; i < initial_states.size(); ++i) {
        auto belief = make_shared<Belief>(-1);
        belief->set_val(initial_states[i], MyFloat(1));
        initial_beliefs.push_back(belief);
    }

    return this->solve_beliefs(initial_beliefs, horizon);
}

pair<shared_ptr<MixedStrategy>, double> ParetoSolver::solve_beliefs(
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
