//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#include "experiments.hpp"

class SuperdenseCoding : public QuantumExperiment {
    const int c0 = 0;
    const int c1 = 1;

    const int q0 = 0;
    const int q1 = 1;

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
            Instruction X(GateName::X, q0);
            qs = qs->apply_instruction(X);
        }

        if (message == Message02 || message == Message03) {
            Instruction Z(GateName::Z, q0);
            qs = qs->apply_instruction(Z);
        }

        return make_shared<HybridState>(qs, classical_state, message);

    }
public:
    void set_experiment_name() override {
        this->name = "superdense_coding";
    }

    void set_horizons() override {
        if (Config::is_debug) {
            this->min_horizon = 2;
            this->max_horizon = 4;
        } else {
            this->min_horizon = 2;
            this->max_horizon = 6;
        }
    }

    void set_qubits_used() override {
        this->qubits_used.push_back(q0);
        this->qubits_used.push_back(q1);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : ALL_MESSAGES) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        return MyFloat(v->hybrid_state->classical_state->get_memory_val() == v->hidden_index());
    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        auto H0 = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::H, q0)}));
        auto H1 = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::H, q1)}));
        vector<shared_ptr<const QAction>> result = {H0, H1};
        if(hardware_specification.digraph.find(q0) != hardware_specification.digraph.end()) {
            if (hardware_specification.digraph.at(q0).find(q1) != hardware_specification.digraph.at(q0).end()) {
                auto CX = make_shared<QAction>(hardware_specification,
                vector<Instruction>({Instruction(GateName::Cnot, vector<int>{q0}, q1)}));
                result.push_back(CX);
            }
        }

        if(hardware_specification.digraph.find(q1) != hardware_specification.digraph.end()) {
            if (hardware_specification.digraph.at(q1).find(q0) != hardware_specification.digraph.at(q1).end()) {
                auto CX = make_shared<QAction>(hardware_specification,
                vector<Instruction>({Instruction(GateName::Cnot, vector<int>{q1}, q0)}));
                result.push_back(CX);
            }
        }



        for (auto q : vector<pair<int, int>>{{q0, c0}, {q1, c1}}) {
            auto meas_action = make_shared<QAction>(hardware_specification, vector<Instruction>({
                Instruction(GateName::Meas, q.first, q.second)
            }));
            result.push_back(meas_action);

            result.push_back(make_shared<QAction>(hardware_specification, vector<Instruction>({
                Instruction(GateName::Toggle, q.second)
            })));
        }
        return result;
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{q0, 0}, {q1, 1}}};
        }
        vector<Embedding> result;
        for (int c_qubit = 0; c_qubit < hw.num_qubits; c_qubit++) {
            for (auto t_qubit : hw.digraph.at(c_qubit)) {
                assert(c_qubit != t_qubit);
                if (t_qubit > c_qubit) {
                    result.push_back(Embedding{{q0, c_qubit}, {q1, t_qubit}});
                }
            }
        }

        return result;
    }
};
