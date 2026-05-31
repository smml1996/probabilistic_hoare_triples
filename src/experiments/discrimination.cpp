//
// Created by Stefanie Muroya Lei on 29.05.26.
//


#include "experiments.hpp"

class ZeroPlusDiscrimination : public QuantumExperiment {
    const int c_target = 0;
    const int target_qubit = 0;
    const int hidden0 = 0;
    const int hiddenP = 1;
protected:
    void set_experiment_name() override {
        this->name = "zero_plus";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 8;
    }

    void set_qubits_used() override {
        this->qubits_used.push_back(target_qubit);
    }

    vector<shared_ptr<HybridState>> get_initial_states() override {
        vector<shared_ptr<HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        auto H0 = Instruction(GateName::H, target_qubit);

        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.push_back(make_shared<HybridState>(state0, classical_state, this->hidden0));

        // prepare first bell state
        auto state1 = state0->apply_instruction(H0);
        result.push_back(make_shared<HybridState>(state1, classical_state, this->hiddenP));

        return result;
    }

    MyFloat get_reward(shared_ptr<QVertex> &v) const override {
        auto current_cs_val = v->classical_state()->get_memory_val();
        assert(current_cs_val == 0 || current_cs_val == 1);
        return current_cs_val == v->hidden_index();
    }

    vector<shared_ptr<QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        auto H0 = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::H, target_qubit)}));

        auto P0 = make_shared<QAction>(hardware_specification,
            vector<Instruction>({Instruction(GateName::Meas, target_qubit, c_target)}));

        auto determine0 = make_shared<QAction>(hardware_specification,
            vector<Instruction>({Instruction(GateName::Write0, c_target)}));

        auto determinePlus = make_shared<QAction>(hardware_specification,
            vector<Instruction>({Instruction(GateName::Write1, c_target)}));
        return {H0, P0, determine0, determinePlus};
    }


    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        vector<Embedding> result;
        for (int qubit = 0; qubit < hw.num_qubits; qubit++) {
            result.push_back(Embedding{{0, qubit}});
        }

        return result;
    }
};
