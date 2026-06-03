//
// Created by Stefanie Muroya Lei on 01.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "experiments.hpp"
#include "solvers.hpp"
#include "src/experiments/discrimination.cpp"
//
// TEST(ResetExpTest, Setup) {
//    ResetProblem reset_problem;
//
//    reset_problem.init();
//
//    assert(reset_problem.name == "reset");
//    assert(reset_problem.min_horizon == 1);
//    // assert(reset_problem.max_horizon == 8);
//
//    shared_ptr<QuantumState> qs0 = make_shared<QuantumState>(vector<int>{0});
//    shared_ptr<ClassicalState> cs = make_shared<ClassicalState>();
//    shared_ptr<HybridState> hs0 = make_shared<HybridState>(qs0, cs, 0);
//    shared_ptr<HybridState> hs1 = hs0->apply_instruction(Instruction(X, 0));
//    auto v1 = make_shared<const QVertex>(hs0);
//    auto v2 = make_shared<const QVertex>(hs1);
//
//    assert(reset_problem.get_reward(v1) == one);
//    assert(reset_problem.get_reward(v2) == zero);
//
//    auto initial_states = reset_problem.get_initial_states();
//
//    assert(*initial_states[0] == *hs0);
//    assert(*initial_states[1] == *hs1);
//
//    assert(reset_problem.qubits_used.size() == 1);
// }


TEST(ZeroPlusExpTest, Solver) {
    ZeroPlusDiscrimination discr_problem;
    Config::is_debug = true;
    discr_problem.init();

    HardwareSpecification hardware_spec_ = HardwareSpecification(QuantumHardware::Algiers, false, true);

    auto embedding = discr_problem.get_embeddings(hardware_spec_)[0];
    auto hardware_spec = hardware_spec_.get_normalized(embedding);
    auto actions = discr_problem.get_actions(hardware_spec);
    auto pomdp = discr_problem.build_pomdp(hardware_spec, actions);

    ParetoSolver solver(pomdp, false);
    int horizon = 4;
    // for (int horizon = 1; horizon <= 3; horizon++) {
    // cout << "------- horizon=" << horizon << " -------" << endl;
    auto result = solver.solve(pomdp.initial_states, horizon);
    // cout << result.second << endl;
    // cout << to_string(*result.first) << endl;
    // }

    auto algorithm = result.first->value[0].first;
    cout << to_string(*algorithm) << endl;
    cout << "*****" << endl;
    algorithm = algorithm->obs_to_strategies.at(0)->obs_to_strategies.at(1);
    cout << to_string(*algorithm) << endl;
    cout << "****" << endl;
    auto b0 = algorithm->obs_to_strategies.at(0);
    auto b1 = algorithm->obs_to_strategies.at(1);
    cout << to_string(*b0) << endl;
    cout << "*****" << endl;
    cout << to_string(*b1) << endl;
    cout << "*****" << endl;
    cout << (*b0 == *b1) << endl;

}