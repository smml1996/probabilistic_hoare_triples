//
// Created by Stefanie Muroya Lei on 01.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "experiments.hpp"
#include "solvers.hpp"
#include "src/experiments/superdense_coding.cpp"

TEST(SuperdenseCodExpTest, Solver) {
    SuperdenseCoding superdense_coding;
    Config::is_debug = true;
    superdense_coding.init();

    HardwareSpecification hardware_spec_ = HardwareSpecification(QuantumHardware::Algiers, false, true);

    auto embedding = superdense_coding.get_embeddings(hardware_spec_)[0];
    auto hardware_spec = hardware_spec_.get_normalized(embedding);

    auto actions = superdense_coding.get_actions(hardware_spec);
    auto pomdp = superdense_coding.build_pomdp(hardware_spec, actions);

    ParetoSolver solver(pomdp, false);
    int horizon = 3;

    auto result = solver.solve(pomdp.initial_states, horizon);
    cout << result.second << endl;
    cout << to_string(*result.first) << endl;


    cout << endl << "PROBABILITIES: " <<endl;
    for (auto e : result.first->value) {
        cout << e.second << endl;
    }


}