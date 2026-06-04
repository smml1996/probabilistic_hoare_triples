//
// Created by Stefanie Muroya Lei on 01.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "experiments.hpp"
#include "solvers.hpp"
#include "src/experiments/superdense_coding.cpp"

// TEST(SuperdenseCodExpTest, Solver) {
//     int horizon = 2;
//     SuperdenseCoding0102 superdense_coding;
//     Config::is_debug = true;
//     superdense_coding.init();
//
//     HardwareSpecification hardware_spec_ = HardwareSpecification(QuantumHardware::Almaden, false, true);
//     vector<shared_ptr<MixedStrategy>> algorithms;
//
//     vector<Embedding> embeddings{
//         superdense_coding.get_embeddings(hardware_spec_)[0],
//         superdense_coding.get_embeddings(hardware_spec_)[1]
//     };
//
//     for (auto embedding : embeddings) {
//         auto hardware_spec = hardware_spec_.get_normalized(embedding);
//         auto actions = superdense_coding.get_actions(hardware_spec);
//         auto pomdp = superdense_coding.build_pomdp(hardware_spec, actions);
//
//         ParetoSolver solver(pomdp, false);
//         auto result = solver.solve(pomdp.initial_states, horizon);
//         algorithms.push_back(result.first);
//     }
//
//     assert(algorithms.size() == 2);
//
//     cout << "******" << endl;
//     cout << to_string(*algorithms[0]) << endl;
//     cout << "******" << endl;
//     cout << "******" << endl;
//     cout << to_string(*algorithms[1]) << endl;
//     cout << "******" << endl;
//     cout << (*algorithms[1] == *algorithms[0]) << endl;
// }


TEST(SuperdenseCodExpTest, MeasAction) {
    SuperdenseCoding superdense_coding;
    Config::is_debug = true;
    superdense_coding.init();

    HardwareSpecification hardware_spec_ = HardwareSpecification(QuantumHardware::Almaden, false, true);
    vector<shared_ptr<MixedStrategy>> algorithms;

    auto embedding = superdense_coding.get_embeddings(hardware_spec_)[0];



    auto hardware_spec = hardware_spec_.get_normalized(embedding);
    auto actions = superdense_coding.get_actions(hardware_spec);
    auto action = actions[actions.size()-1];
    cout << action->name << endl;

    auto qs = make_shared<QuantumState>(vector<int>{0,1});
    qs = qs->apply_instruction(Instruction(GateName::H, 0));
    qs = qs->apply_instruction(Instruction(GateName::Cnot, vector<int>{0}, 1));
    shared_ptr<HybridState> hs = make_shared<HybridState>(qs, make_shared<ClassicalState>(), 0);
    shared_ptr<QVertex> v = make_shared<QVertex>(hs);

    auto result = action->get_successor_states(hardware_spec, v);
    result.print();
}