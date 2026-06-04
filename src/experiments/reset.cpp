//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#ifndef RESET_H
#define RESET_H
#include <cassert>

#include "experiments.hpp"
using namespace std;
class ResetProblem : public QuantumExperiment {
    const int target_qubit = 0;
public:
    void set_horizons() override {
        this->min_horizon = 1;
        if (Config::is_debug) {
            this->max_horizon = 7;
        } else {
            this->max_horizon = 8;
        }

    }

    void set_experiment_name() override {
        this->name = "reset";
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        if (v->hybrid_state->quantum_state->is_qubit_0()) {
            return MyFloat(1);
        }
        return MyFloat(0);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        auto X0 = Instruction(GateName::X, target_qubit);

        // prepare first bell state
        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.emplace_back(new HybridState(state0, classical_state, 0));

        // prepare second bell state
        auto state1 = state0->apply_instruction(X0);
        assert (!(*state1 == *state0));
        result.emplace_back(make_shared<HybridState>(state1, classical_state, 0));

        return result;

    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        auto X0 = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::X, target_qubit)}));

        auto P0 = make_shared<QAction>(hardware_specification,
            vector<Instruction>({Instruction(GateName::Meas, target_qubit, target_qubit)}));

        return {X0, P0};
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{target_qubit, 1}}};
        }


        vector<Embedding> embeddings;

        if (Config::is_debug) {
            auto pivot_qubits = get_meas_pivot_qubits(hw, 0);
            for (int q : pivot_qubits) {
                embeddings.emplace_back(Embedding{{target_qubit,q}});
            }
        } else {
            for (auto q= 0; q < hw.num_qubits; ++q) {
                embeddings.emplace_back(Embedding{{target_qubit,q}});
            }
        }

        return embeddings;
    }

    void set_qubits_used() override {
        this->qubits_used = {target_qubit};
    }
};
#endif