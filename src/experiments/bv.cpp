//
// Created by Stefanie Muroya Lei on 31.05.26.
//
#ifndef BV_EXP_H
#define BV_EXP_H

#include "experiments.hpp"

class BernsteinVazirani : public QuantumExperiment { // 1-qubit oracle
public:
    // quantum addresses
    int s0 = 0; // secret key qubit
    int q0 = 1; // input and output register

    // classical addresses
    int c0 = 0; // should store the secret key

    // addresses for flags
    const int write_c0_flag = 1;
    const int oracle_flag = 2;
    const int meas_flag = 3;
    const int unitary_flag = 4;

    bool with_noise;

    BernsteinVazirani(const bool &with_noise) : QuantumExperiment() {
        this->with_noise = with_noise;
    };

    unordered_map<int, int> action_ids_to_flag;

protected:
    vector<shared_ptr<const QAction>> get_actions_(HardwareSpecification &hardware_specification) override {
        Instruction forbid_oracle = Instruction(GateName::Write1, oracle_flag);
        Instruction forbid_meas = Instruction(GateName::Write1, meas_flag);
        Instruction forbid_write_c0 = Instruction(GateName::Write1, write_c0_flag);
        Instruction forbid_unitary = Instruction(GateName::Write1, unitary_flag);

        Instruction enable_write_c0 = Instruction(GateName::Write0, write_c0_flag);


        vector<shared_ptr<const QAction>> actions;
        // 1-qubit unitary gates
        auto H0 = Instruction(GateName::H, q0);
        auto H0_action = make_shared<QAction>(hardware_specification,
            vector<Instruction>({
                H0,
                forbid_write_c0
            }));
        actions.push_back(H0_action);
        this->action_ids_to_flag[H0_action->id] = unitary_flag;

        // oracle
        auto CZ = make_shared<QAction>(hardware_specification,
            vector<Instruction>({
                H0,
                Instruction(GateName::Cnot, vector<int>{s0}, q0),
                H0,
                forbid_oracle,
                forbid_write_c0
            }),
            BernsteinVazirani::with_noise
            );
        actions.push_back(CZ);
        this->action_ids_to_flag[CZ->id] = oracle_flag;

        // Measurement
        auto meas = make_shared<QAction>(
            hardware_specification,
            vector<Instruction>({
                Instruction(GateName::Meas, q0, c0),
                enable_write_c0
        }));
        actions.push_back(meas);
        this->action_ids_to_flag[meas->id] = meas_flag;


        // correction instructions
        auto toggle_ins = make_shared<QAction>(
            hardware_specification,
            vector<Instruction>({
                Instruction(GateName::Toggle, c0),
                forbid_write_c0,
                forbid_oracle,
                forbid_meas,
                forbid_unitary,
            }));
        actions.push_back(toggle_ins);
        this->action_ids_to_flag[toggle_ins->id] = write_c0_flag;

        return actions;
    }

public:
    void set_experiment_name() override {
        this->name = "bern_vazirani" + to_string(BernsteinVazirani::with_noise);
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

    void set_qubits_used() override {
        this->qubits_used.push_back(s0);
        this->qubits_used.push_back(q0);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        vector<shared_ptr<const HybridState>> result;

        auto classical_state = make_shared<ClassicalState>();

        // secret key = 0
        auto state0 = make_shared<QuantumState>(this->qubits_used);
        result.push_back(make_shared<HybridState>(state0, classical_state, 0));

        // secret key = 1
        auto X = Instruction(GateName::X, s0);
        auto state1 = state0->apply_instruction(X);
        result.push_back(make_shared<HybridState>(state1, classical_state, 1));

        return result;
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        return MyFloat(v->classical_state()->read(c0) == v->hidden_index());
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{s0, 0}, {q0, 1}}};
        }
        vector<Embedding> result;

        if (Config::is_debug) {
            auto pivot_qubits = get_meas_pivot_qubits(hw, 1);
            for (int pivot_q : pivot_qubits) {
                auto couplers = hw.get_sorted_qubit_couplers(pivot_q);
                auto p1 = couplers[0].first;
                auto p2 = couplers[couplers.size()-1].first;

                result.push_back(Embedding{{s0, p1}, {q0, pivot_q}});

                if (p1 != p2) {
                    result.push_back(Embedding{{s0, p2}, {q0, pivot_q}});
                }
            }
        } else {
            for (int c_qubit = 0; c_qubit < hw.num_qubits; c_qubit++) {
                for (auto t_qubit : hw.digraph.at(c_qubit)) {
                    assert(c_qubit != t_qubit);
                    result.push_back(Embedding{{s0, c_qubit}, {q0, t_qubit}});
                }
            }
        }
        return result;
    }

    void set_horizons() override {
        this->min_horizon = 1;
        this->max_horizon = 6;
    }

    bool guard(const shared_ptr<const QVertex> &v, const shared_ptr<const QAction> &a) const override {
        if (this->action_ids_to_flag.find(a->id) != this->action_ids_to_flag.end()) {
            return !v->classical_state()->read(this->action_ids_to_flag.at(a->id));
        }
        assert(false);
    }
};
#endif