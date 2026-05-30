//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#include "experiments.hpp"

class SuperdenseCoding : public QuantumExperiment {
    const int c0 = 0;
    const int c1 = 1;

    const int q0 = 0;
    const int q1 = 1;
    const int qhidden = 2;

    const int Message00 = 0;
    const int Message01 = 1;
    const int Message02 = 2;
    const int Message03 = 3;

    const vector<int> ALL_MESSAGES = {Message00, Message01, Message02, Message03};

    shared_ptr<HybridState> get_message_hs(const int &message) const {
        auto classical_state = make_shared<ClassicalState>();

        auto qs = make_shared<QuantumState>(this->qubits_used);

        // prepare Bell state
        auto H = Instruction(GateName::H, q0);
        auto CX = Instruction(GateName::Cnot, vector<int>{q0}, q1);

        qs = qs->apply_instruction(H);
        qs = qs->apply_instruction(CX);

        if (message == Message01 || message == Message03) {
            auto X = Instruction(GateName::X, qhidden);
            qs = qs->apply_instruction(X);
        }

        if (message == Message02 || message == Message03) {
            auto H = Instruction(GateName::H, qhidden);
            qs = qs->apply_instruction(H);
        }

        return make_shared<HybridState>(qs, classical_state);

    }
protected:
    void set_experiment_name() override {
        this->name = "superdense_coding";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 7;
    }

    void set_qubits_used() override {
        this->qubits_used.push_back(q0);
        this->qubits_used.push_back(q1);
        this->qubits_used.push_back(qhidden);
    }

    vector<shared_ptr<HybridState>> get_initial_states() override {
        vector<shared_ptr<HybridState>> result;

        for (auto message : ALL_MESSAGES) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }

    MyFloat get_reward(shared_ptr<QVertex> &v) const override {
        auto pt_current = v->quantum_state()->multi_partial_trace({q0, q1});
        int real_message = this->get_real_message(pt_current);
        return MyFloat(v->hybrid_state->classical_state->get_memory_val() == real_message);
    }

    vector<shared_ptr<QAction>> get_actions(HardwareSpecification &hardware_specification) const override {
        auto H0 = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::H, target_qubit)}));

        auto P0 = make_shared<QAction>(hardware_specification,
            vector<Instruction>({Instruction(GateName::Meas, target_qubit, c_target)}));

        vector<shared_ptr<QAction>> result = {X0, Z0};

        for (int i = 0; i < 2; i++) {

            GateName write_ins0;
            if (i == 0) {
                write_ins0 = GateName::Write0;
            } else {
                write_ins0 = Write1;
            }

            for (int j = 0; j < 2; j++) {
                GateName write_ins1;
                if (j == 0) {
                    write_ins1 = GateName::Write0;
                } else {
                    write_ins1 = GateName::Write1;
                }
                result.push_back(
                    make_shared<QAction>(hardware_specification,
                        vector<Instruction>({
                            Instruction(write_ins0, c0),
                            Instruction(write_ins1, c1),
                            })
                        )
                );

            }

        }
        return result;
    }
};
