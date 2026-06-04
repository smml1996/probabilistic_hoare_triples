//
// Created by Stefanie Muroya Lei on 04.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "experiments.hpp"
#include "solvers.hpp"
#include "src/experiments/bv.cpp"


TEST(BVTest, Solver) {
    int horizon = 3;
    BernsteinVaziraniNoiselessOracle bernstein_vazirani;

    bernstein_vazirani.init();

    HardwareSpecification hardware_spec_ = HardwareSpecification(QuantumHardware::Algiers, false, true);

    auto embeddings = bernstein_vazirani.get_embeddings(hardware_spec_);
    auto embedding = embeddings[0];

    auto hardware_spec = hardware_spec_.get_normalized(embedding);
    auto actions = bernstein_vazirani.get_actions(hardware_spec);
    auto action = actions[3];
    cout << action->name << endl;

    auto qs = make_shared<QuantumState>(vector<int>{0,1});
    qs = qs->apply_instruction(Instruction(GateName::H, 0));
    qs = qs->apply_instruction(Instruction(GateName::Cnot, vector<int>{0}, 1));
    shared_ptr<HybridState> hs = make_shared<HybridState>(qs, make_shared<ClassicalState>(), 0);
    shared_ptr<QVertex> v = make_shared<QVertex>(hs);

    auto result = action->get_successor_states(hardware_spec, v);
    result.print();
}