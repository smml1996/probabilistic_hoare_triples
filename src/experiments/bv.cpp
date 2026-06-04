//
// Created by Stefanie Muroya Lei on 31.05.26.
//

#include "experiments.hpp"

class BernsteinVazirani : public QuantumExperiment {
    // this is an oracle of 1 qubit
public:
    // quantum addresses
    int s0 = 0; // secret key qubit
    int q0 = 1; // input and output register

    // classical addresses
    int c0 = 0; // should store the secret key
    void set_experiment_name() override {
        this->name = "bern_vazirani";
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
        auto current_cs_val = v->classical_state()->get_memory_val();
        assert(current_cs_val == 0 || current_cs_val == 1);
        return current_cs_val == v->hidden_index();
    }

    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        vector<shared_ptr<const QAction>> actions;

        // Hadamard gates
        auto H0 = Instruction(GateName::H, q0);
        actions.push_back(make_shared<QAction>(hardware_specification, vector<Instruction>({H0})));

        // oracle
        auto CZ = make_shared<QAction>(hardware_specification,
            vector<Instruction>({
                H0,
                Instruction(GateName::Cnot, vector<int>{s0}, q0),
                H0
            }));
        actions.push_back(CZ);

        // Measurement
        auto meas = make_shared<QAction>(
            hardware_specification,
            vector<Instruction>({Instruction(GateName::Meas, q0, c0)}));
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
};

class BernsteinVaziraniX : public BernsteinVazirani {
public:
    void set_experiment_name() override {
        this->name = "bern_vaziranix";
    }
    vector<shared_ptr<const QAction>> get_actions(HardwareSpecification &hardware_specification) override {
        auto temp = BernsteinVazirani::get_actions(hardware_specification);

        Instruction X_instruction(GateName::X, q0);
        temp.push_back(make_shared<QAction>(hardware_specification, vector<Instruction>({X_instruction})));
        return temp;
    }
};