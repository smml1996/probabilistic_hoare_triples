//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#include "experiments.hpp"

class SuperdenseCoding : public QuantumExperiment {
public:
    const int c0 = 0;
    const int c1 = 1;

    // addresses flags for classical states
    const int write_c0_flag = 2;
    const int write_c1_flag = 3;
    const int unitary_flag = 4;
    const int meas_flag = 5;

    const int q0 = 0;
    const int q1 = 1;

    const int Message00 = 0;
    const int Message01 = 1;
    const int Message02 = 2;
    const int Message03 = 3;

    const vector<int> ALL_MESSAGES = {Message00, Message01, Message02, Message03};

    unordered_map<int, int> action_ids_to_flag;

protected:
    vector<shared_ptr<const QAction>> get_actions_(HardwareSpecification &hardware_specification) override {
        this->action_ids_to_flag.clear();
        Instruction forbid_basis_trans = Instruction(GateName::Write1, unitary_flag);
        Instruction forbid_meas = Instruction(GateName::Write1, meas_flag);
        Instruction forbid_write_c0 = Instruction(GateName::Write1, write_c0_flag);
        Instruction forbid_write_c1 = Instruction(GateName::Write1, write_c1_flag);

        Instruction enable_write_c0 = Instruction(GateName::Write0, write_c0_flag);
        Instruction enable_write_c1 = Instruction(GateName::Write0, write_c1_flag);

        vector<shared_ptr<const QAction>> result;
        // 1. Bell basis transformer
        auto H0 = Instruction(GateName::H, q0);
        Instruction CX01 = Instruction(GateName::Cnot, vector<int>{q0}, q1);
        auto basis_trans_action = make_shared<QAction>(hardware_specification,
           vector<Instruction>  {
               CX01,
               H0,
               forbid_basis_trans, // only allow one basis transformation
               forbid_write_c0,
               forbid_write_c1
           });
        result.push_back(basis_trans_action);
        this->action_ids_to_flag[basis_trans_action->id] = unitary_flag;

        vector<Instruction> meas_seq{
            Instruction(GateName::Meas, q0, c0),
            Instruction(GateName::Meas, q1, c1),
            forbid_basis_trans, // basis transformation should happen before measurement
            enable_write_c0,
            enable_write_c1
        };
        auto meas_action = make_shared<QAction>(hardware_specification, meas_seq);
        result.push_back(meas_action);
        this->action_ids_to_flag[meas_action->id] = meas_flag;

        // classical instructions for c0
        {
            vector<Instruction> classical_seq{
                Instruction(GateName::Toggle, c0),
                forbid_basis_trans,
                forbid_meas,
                forbid_write_c0,
            };
            auto c_action = make_shared<QAction>(hardware_specification, classical_seq);
            result.push_back(c_action);
            this->action_ids_to_flag[c_action->id] = write_c0_flag;
        }


        // classical instructions for c1
        {
            vector<Instruction> classical_seq{
                Instruction(GateName::Toggle, c1),
                forbid_basis_trans,
                forbid_meas,
                forbid_write_c0,
                forbid_write_c1,
            };
            auto c_action = make_shared<QAction>(hardware_specification, classical_seq);
            result.push_back(c_action);
            this->action_ids_to_flag[c_action->id] = write_c1_flag;
        }

        return result;
    }
public:
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
        this->min_horizon = 1;
        this->max_horizon = 6;
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
        int val = v->classical_state()->read(c0) + v->classical_state()->read(c1) * 2;
        return MyFloat(val == v->hidden_index());
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
        if (this->action_ids_to_flag.find(a->id) != this->action_ids_to_flag.end()) {
            return !v->classical_state()->read(this->action_ids_to_flag.at(a->id));
        }
        assert(false);
    }
};

class SuperdenseCoding0001 : public SuperdenseCoding {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0001";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message00, this->Message01}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};

class SuperdenseCoding0002 : public SuperdenseCoding0001 {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0002";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message00, this->Message02}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};


class SuperdenseCoding0003 : public SuperdenseCoding0001 {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0003";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message00, this->Message03}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};

class SuperdenseCoding0102 : public SuperdenseCoding0001 {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0102";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message01, this->Message02}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};

class SuperdenseCoding0103 : public SuperdenseCoding0001 {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0103";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message01, this->Message03}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};

class SuperdenseCoding0203 : public SuperdenseCoding0001 {
public:
    void set_experiment_name() override {
        this->name = "SuperdenseCoding0203";
    }
    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        for (auto message : {this->Message02, this->Message03}) {
            result.push_back(this->get_message_hs(message));
        }

        return result;
    }
};