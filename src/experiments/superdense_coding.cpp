//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#include "experiments.hpp"

class SuperdenseCoding : public QuantumExperiment {
public:
    const int c0 = 0;
    const int c1 = 1;
    const int c2 = 2;
    const int c3 = 3;

    const int q0 = 0;
    const int q1 = 1;

    const int Message00 = 0;
    const int Message01 = 1;
    const int Message02 = 2;
    const int Message03 = 3;

    const vector<int> ALL_MESSAGES = {Message00, Message01, Message02, Message03};

    int write_id2 = -1;
    int write_id3 = -1;

    shared_ptr<HybridState> get_message_hs(const int &message) const {
        auto classical_state = make_shared<ClassicalState>();

        auto qs = make_shared<QuantumState>(this->qubits_used);

        // prepare Bell state
        auto H = Instruction(GateName::H, q0);
        auto CX = Instruction(GateName::Cnot, vector<int>{q0}, q1);

        qs = qs->apply_instruction(H);
        qs = qs->apply_instruction(CX);


        if (message == Message02 || message == Message03) {
            Instruction X(GateName::X, q0);
            qs = qs->apply_instruction(X);
        }

        if (message == Message01 || message == Message03) {
            Instruction Z(GateName::Z, q0);
            qs = qs->apply_instruction(Z);
        }

        return make_shared<HybridState>(qs, classical_state, message);

    }

    void set_experiment_name() override {
        this->name = "superdense_coding";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 5;
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
        int val = v->classical_state()->read(c0) * (1 << c0) + v->classical_state()->read(c1) * (1 << c1);
        return MyFloat(val == v->hidden_index());
    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        vector<shared_ptr<const QAction>> result;


        auto H0 = Instruction(GateName::H, q0);
        result.push_back(make_shared<QAction>(hardware_specification,
            vector<Instruction>{H0}));
        Instruction CX01 = Instruction(GateName::Cnot, vector<int>{q0}, q1);
        auto CX = make_shared<QAction>(hardware_specification,
            vector<Instruction>{CX01});
        result.push_back(CX);

        for (auto [q, c_a] : vector<pair<pair<int, int>, int>>{{{q0, c0}, c2}, {{q1, c1}, c3}}) {
            vector<Instruction> meas_seq;
            meas_seq.push_back(Instruction(GateName::Meas, q.first, q.second));
            auto meas_action = make_shared<QAction>(hardware_specification, meas_seq);
            result.push_back(meas_action);

            auto c_action = make_shared<QAction>(hardware_specification, vector<Instruction>({
                Instruction(GateName::Toggle, q.second),
                Instruction(GateName::Write1, c_a),
            }));
            result.push_back(c_action);
            if (c_a == c2) {
                write_id2 = c_action->id;
            } else {
                assert(c_a == c3);
                write_id3 = c_action->id;
            }
        }
        return result;
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{q0, 0}, {q1, 1}}};
        }

        vector<Embedding> result;

        if (Config::is_debug) {
            auto couplers = hw.get_sorted_qubit_couplers();
            assert(couplers.size() > 0);
            auto p1 = couplers[0].first;
            auto p2 = couplers[couplers.size()-1].first;

            result.push_back(Embedding{{q0, p1.first}, {q1, p1.second}});

            if (p1.first != p2.first || p1.second != p2.second) {
                result.push_back(Embedding{{q0, p2.first}, {q1, p2.second}});
            }
        } else {
            for (int c_qubit = 0; c_qubit < hw.num_qubits; c_qubit++) {
                for (auto t_qubit = c_qubit + 1; t_qubit < hw.num_qubits; t_qubit++) {
                    if (hw.are_adjacent(c_qubit, t_qubit)) {
                        result.push_back(Embedding{{q0, c_qubit}, {q1, t_qubit}});
                    }
                }
            }
        }
        return result;
    }

    void set_quantum_hardware() override {
        for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
            QuantumHardware quantum_hardware = static_cast<QuantumHardware>(i);
            HardwareSpecification hs(quantum_hardware, this->with_thermalization);
            if (hs.basis_gates_type != BasisGates::TYPE5 && hs.basis_gates_type != BasisGates::TYPE2) {
                this->hardware_list.insert(quantum_hardware);
            }
        }
    }

    bool guard(const shared_ptr<const QVertex> &v, const shared_ptr<const QAction> &a) const override {
        auto cs = v->classical_state();
        assert(write_id2 != -1 && write_id3 != -1);
        if (a->id == write_id2) {
            return !cs->read(c2) && !cs->read(c3);
        }

        if (a->id == write_id3) {
            return !cs->read(c3);
        }

        return !cs->read(c2) && !cs->read(c3);
    }
};
