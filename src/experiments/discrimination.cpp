//
// Created by Stefanie Muroya Lei on 29.05.26.
//


#include "experiments.hpp"

class ZeroPlusDiscrimination : public QuantumExperiment {
    const int c_target = 0;
    const int target_qubit = 0;
    const int hidden0 = 0;
    const int hiddenP = 1;
public:
    void set_experiment_name() override {
        this->name = "zero_plus";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 5;
    }

    void set_qubits_used() override {
        this->qubits_used.push_back(target_qubit);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        auto H0 = Instruction(GateName::H, target_qubit);

        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.push_back(make_shared<HybridState>(state0, classical_state, this->hidden0));

        // prepare first bell state
        auto state1 = state0->apply_instruction(H0);
        result.push_back(make_shared<HybridState>(state1, classical_state, this->hiddenP));

        return result;
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        auto current_cs_val = v->classical_state()->get_memory_val();
        assert(current_cs_val == 0 || current_cs_val == 1);
        return current_cs_val == v->hidden_index();
    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
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
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{ {target_qubit, 0}}};
        }

        vector<Embedding> result;
        if (Config::is_debug) {
            auto pivot_qubits = get_meas_pivot_qubits(hw, 0);
            for (int q : pivot_qubits) {
                result.emplace_back(Embedding{{target_qubit,q}});
            }
        } else {
            for (auto q= 0; q < hw.num_qubits; ++q) {
                result.emplace_back(Embedding{{target_qubit,q}});
            }
        }
        return result;
    }
};
