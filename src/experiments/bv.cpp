//
// Created by Stefanie Muroya Lei on 31.05.26.
//

#include "experiments.hpp"

class BernsteinVazirani : public QuantumExperiment {
public:
    // quantum addresses
    int hidden0 = 0;
    int q0 = 1;

    // classical addresses
    int c0 = 0;
    virtual void set_experiment_name() override {
        this->name = "bern_vazirani";
    }

    virtual void set_qubits_used() override {
        this->qubits_used.push_back(hidden0);
        this->qubits_used.push_back(q0);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        // secret key = 0
        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.push_back(make_shared<HybridState>(state0, classical_state, 0));

        // secret key = 1
        auto X = Instruction(GateName::X, hidden0);
        auto state1 = state0->apply_instruction(X);
        result.push_back(make_shared<HybridState>(state1, classical_state, 1));

        return result;
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        auto current_cs_val = v->classical_state()->get_memory_val();
        assert(current_cs_val == 0 || current_cs_val == 1);
        return current_cs_val == v->hidden_index();
    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        vector<shared_ptr<const QAction>> actions;

        // hadamard gates
        auto H = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::H, q0)}));
        actions.push_back(H);


        // to prepare |-> at ancilla
        auto X = make_shared<QAction>(hardware_specification, vector<Instruction>({Instruction(GateName::X, q0)}));
        actions.push_back(X);

        // oracle
        auto CX = make_shared<QAction>(hardware_specification,
            vector<Instruction>({
                Instruction(GateName::Cnot, vector<int>{hidden0}, q0)
            }));
        actions.push_back(CX);

        // measurement
        auto meas = make_shared<QAction>(
            hardware_specification,
            vector<Instruction>({Instruction(GateName::Meas, q0)}));
        actions.push_back(meas);


        // correction instructions
        auto toggle_ins = make_shared<QAction>(
            hardware_specification,
            vector<Instruction>({Instruction(GateName::Toggle, c0)}));
        actions.push_back(toggle_ins);

        return actions;
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{hidden0, 0}, {q0, 1}}};
        }
        vector<Embedding> result;
        for (int c_qubit = 0; c_qubit < hw.num_qubits; c_qubit++) {
            for (auto t_qubit : hw.digraph.at(c_qubit)) {
                assert(c_qubit != t_qubit);
                result.push_back(Embedding{{hidden0, c_qubit}, {q0, t_qubit}});
            }
        }
        return result;
    }
};
