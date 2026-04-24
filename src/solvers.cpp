#include "solvers.hpp"
#include "ortools/linear_solver/linear_solver.h"
#include <cassert>

using namespace std;


vector<shared_ptr<Multibelief>> Solver::get_multibelief_successors(const shared_ptr<Multibelief> &current, const shared_ptr<POMDPAction> &action)  {
    // we first compute which beliefs we can reach for each belief in the multibelief
    vector<map<int, shared_ptr<Belief>>> successor_beliefs; // this vector should be (at the end) the same length as the multibelief.
                            // I.e., each index corresponds to the beliefs that can be reached by the corresponding belief
    set<int> reachable_obs;
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

shared_ptr<MWP> Solver::get_mwp(const shared_ptr<Multibelief> &multibelief, const shared_ptr<POMDPAction> &action) const {
    shared_ptr<MWP> current_mwp = make_shared<MWP>(multibelief->beliefs.size());
    int i = 0;
    for (auto belief: multibelief->beliefs) {
        current_mwp->values[i] = this->get_reward(belief, action);
        i+=1;
    }
    return current_mwp;
}

ParetoSolver::ParetoSolver(const POMDP &pomdp, const bool &convexify) : dominant_points(0, false) {
    this->pomdp = pomdp;
    this->convexify = convexify;
}


vector<shared_ptr<MWP>> Solver::get_achievable_mwps(const shared_ptr<MWP> &current_score, const vector<set<shared_ptr<MWP>, MWPPtrComp>> &multibelief_points, int mb_index) { // mb_index (multibelief index)
    vector< shared_ptr<MWP>> current_points;
    for (auto current_mwp : multibelief_points[mb_index]) { // for each point current_mwp that a multibelief can reach, we create a new point new_score = current_score + current_mwp
        auto new_score = *current_score + *current_mwp;
        current_points.push_back(new_score);
    }

    if (mb_index == multibelief_points.size()-1) {
        // we are in the last set of points (of the multibeliefs that can be reached)
        return current_points;
    }

    vector<shared_ptr<MWP>> result;
    // in this loop we combine each point generated (current_points) with all the points that the other multibeliefs can reach (thus mb_index+1)
    for (auto mwp : current_points) {
        auto succ_strategies = this->get_achievable_mwps(mwp,
            multibelief_points, mb_index +1);

        for (auto ss : succ_strategies) {
            result.push_back(ss);
        }
    }
    return result;
}

MyFloat Solver::get_reward(const shared_ptr<Belief> &b, const shared_ptr<POMDPAction> &action) const {
     MyFloat result(0);
    for (auto p : b->probs) {
            result += p.second * this->pomdp.get_reward(p.first, action);
    }
    return result;
}

 set<shared_ptr<MWP>, MWPPtrComp> ParetoSolver::get_points(const shared_ptr<Multibelief> &multibelief, const int &horizon) {
    // this function returns all strategies that are reachable from the given beliefs and are of length horizon
    // we assume that all beliefs here have the same observation

     set<shared_ptr<MWP>, MWPPtrComp> result;

    // consider strategy that halts immediately
    auto mwp_halt = get_mwp(multibelief, halt_action);
    result.insert(mwp_halt);
    // ****************

    if (horizon == 0) {
        return result;
    } else {
        assert(horizon > 0);
        for (auto action : this->pomdp.actions) {
            // compute reachable multibeliefs
            vector<shared_ptr<Multibelief>> multibelief_successors = this->get_multibelief_successors(multibelief, action);

            // compute the best mwps that can be achieved with any strategy with (horizon-1) for each successor multibelief
            vector< set<shared_ptr<MWP>, MWPPtrComp>> successor_points; // each successor multibelief has a set of best points

            for (auto succ_mb : multibelief_successors) {
                successor_points.push_back(this->get_points(succ_mb, horizon-1));
            }

            shared_ptr<MWP> current_score_ = this->get_mwp(multibelief, action); // initialize MWP filled with zeros
            vector<shared_ptr<MWP>> achievable_mwps = this->get_achievable_mwps(current_score_, successor_points); // we have to do an all vs all points

            for (auto mwp : achievable_mwps) {
                this->dominant_points.add_point(mwp);
            }
        }

        return result;
    }

}

double Solver::solve_lp_maximin(const int &n_initial_states, const  set<shared_ptr<MWP>, MWPPtrComp>& scores) {
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

    if (!is_close(sum_, 1.0)) {
        throw runtime_error("sum_ is incorrect: " + to_string(sum_));
    }

    return final_value;
}

map<int, shared_ptr<Belief>> Solver::get_successor_beliefs(const shared_ptr<Belief> &current_belief,
    const shared_ptr<POMDPAction> &action) {
    assert (!(*action == *halt_action));

    map<int, shared_ptr<Belief>> obs_to_next_beliefs;
    assert(current_belief->probs.size() > 0);
    for(auto & prob : current_belief->probs) {
        auto current_v = prob.first;
        assert(prob.second > this->zero);
        for (const auto &it_next_v: pomdp.transition_matrix[current_v][action]) {
            assert(it_next_v.second > this->zero);
            auto successor = it_next_v.first;
            for (auto obs : this->pomdp.observations) {
                if (obs_to_next_beliefs.find(obs) == obs_to_next_beliefs.end()) {
                    obs_to_next_beliefs[obs] = make_shared<Belief>(obs);
                    obs_to_next_beliefs[obs]->obs = obs;
                }
                auto prob_obs = pomdp.get_obs_prob(action, successor, obs);
                if (prob_obs > zero) {
                    obs_to_next_beliefs[obs]->add_val(successor, prob.second * it_next_v.second * prob_obs);
                }
            }
        }
    }
    assert(obs_to_next_beliefs.size() > 0);
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
    this->dominant_points.clear(initial_beliefs.size(), this->convexify);

    shared_ptr<Multibelief> multibelief = make_shared<Multibelief>(initial_beliefs, -1);

    auto strategies = this->get_points(multibelief, horizon);
    this->pomdp.actions.pop_back();

    return this->solve_lp_maximin(initial_beliefs.size(), strategies);
}
