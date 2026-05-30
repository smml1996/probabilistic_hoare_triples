//
// Created by Stefanie Muroya Lei on 29.05.26.
//


#include "experiments.hpp"

class ZeroPlusDiscrimination : public QuantumExperiment {
    vector<vector<complex<double>>> partial0;
    vector<vector<complex<double>>> partialP;
    const int c_target = 0;
    const int target_qubit = 0;
    const int hidden_qubit = 1;
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
        this->qubits_used.push_back(hidden_qubit);
    }

    vector<shared_ptr<HybridState>> get_initial_states() override {
        vector<shared_ptr<HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        auto H0 = Instruction(GateName::H, target_qubit);
        auto H1 = Instruction(GateName::H, hidden_qubit);

        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.push_back(make_shared<HybridState>(state0, classical_state));
         this->partial0 = state0->multi_partial_trace({target_qubit});

        // prepare first bell state
        auto state1 = state0->apply_instruction(H0);
        state1 = state1->apply_instruction(H1);
        result.push_back(make_shared<HybridState>(state1, classical_state));
        this->partialP = state1->multi_partial_trace({target_qubit});

        return result;
    }

    MyFloat get_reward(shared_ptr<QVertex> &v) const override {
        auto pt_current = v->quantum_state()->multi_partial_trace({target_qubit});
        if (are_matrices_equal(pt_current, this->partialP)) {
            return MyFloat(v->hybrid_state->classical_state->read(c_target));
        }
        assert(are_matrices_equal(pt_current, this->partial0));
        return MyFloat(!v->hybrid_state->classical_state->read(c_target));
    }

    vector<shared_ptr<QAction>> get_actions(HardwareSpecification &hardware_specification) const override {
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
            if (qubit == 0) {
                result.push_back(Embedding{{0,qubit}, {1, qubit+1}});
            } else {
                result.push_back(Embedding{{0,qubit}, {1, 0}});
            }
        }

        return result;
    }


};
