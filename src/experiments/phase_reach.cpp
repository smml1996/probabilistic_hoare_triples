//
// Created by smuroyal on 30.09.26.
//

#ifndef PHASEREACH_H
#define PHASEREACH_H
#include <cassert>

#include "bitflip.cpp"
using namespace std;

class PhaseReach : public IPMA {
protected:
    void set_num_batches() override {
        this->num_batches = 20;
    }

    shared_ptr<QuantumState> get_target_state(const int &hidden_index, const unordered_map<int, int> &embedding,
                                              bool add_phase = false) const {
        assert(hidden_index <=1 && hidden_index >= 0);
        auto X1 = Instruction(GateName::X, embedding.at(1));
        auto H0 = Instruction(GateName::H, embedding.at(0));

        auto state = make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
        state = state->apply_instruction(H0);

        if (hidden_index == 1) {
            state = state->apply_instruction(X1);
        }

        if (add_phase) {
            auto Z0 = Instruction(GateName::Z, embedding.at(0));
            auto X0 = Instruction(GateName::X, embedding.at(0));

            state = state->apply_instruction(Z0);

            if (hidden_index == 1) {
                state = state->apply_instruction(X0);
            }
        }

        return state;
    }


    void set_num_vars() override {
        this->nqvars = 2;
        this->ncvars = 2;
    }

    void set_hidden_index_to() override {
        this->set_hidden_index = false;
    }

    void set_min_max_horizon(const MethodType &method) override {
        this->min_horizon = 2;
        if (method == MethodType::SingleDistBellman) {
            this->max_horizon = 8;
        } else {
            this->max_horizon = 8;
        }
    }

    void set_methods() override {
        this->method_types.clear();
        this->method_types.insert(MethodType::SingleDistBellman);
        this->method_types.insert(MethodType::Convex);
    }

    void set_global_equality() override {
        QuantumState::use_global_eq = false;
    }

public:
    PhaseReach(const string &name, const set<QuantumHardware> &hw_list) : IPMA(name, hw_list) {
        this->setup_params();
    };

    [[nodiscard]] bool guard(const shared_ptr<POMDPVertex> &vertex, const unordered_map<int, int> &embedding,
                             const shared_ptr<POMDPAction> &action) const override {
        if (*action == HALT_ACTION) return false;
        if (vertex->hybrid_state->classical_state->read(1)) {
            return action->instruction_sequence[0].gate_name != GateName::Meas;
        }
        return true;
    }

    vector<pair<shared_ptr<HybridState>, double> >
    get_initial_distribution(const unordered_map<int, int> &embedding) const override {
        assert(embedding.size() == 2);
        vector<pair<shared_ptr<HybridState>, double> > result;

        auto classical_state = make_shared<ClassicalState>();

        for (int hidden_index = 0; hidden_index < 2; hidden_index++) {
            auto state0 = this->get_target_state(hidden_index, embedding);
            result.emplace_back(new HybridState(state0, classical_state), 0.5);
        }

        return result;
    }

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        MyFloat answer("0", this->precision * (this->max_horizon + 1));

        for (const auto &it: belief.probs) {
            auto qs = it.first->hybrid_state->quantum_state;
            auto is_target = this->target_vertices.find(it.first->id);
            if (is_target != this->target_vertices.end()) {
                if (is_target->second) {
                    answer = answer + it.second;
                }
            } else {
                bool found = false;
                for (int hidden_index = 0; hidden_index < 2 && !found; hidden_index++) {
                    auto target_state = this->get_target_state(hidden_index,
                                                               embedding, true);
                    if (*qs == *target_state) {
                        answer = answer + it.second;
                        this->target_vertices[it.first->id] = true;
                        found = true;
                    }
                }

                if (!found) {
                    this->target_vertices[it.first->id] = false;
                }
            }
        }

        return answer;
    }

    vector<shared_ptr<POMDPAction> > get_actions(HardwareSpecification &hardware_spec,
                                                 const unordered_map<int, int> &embedding) const override {
        assert(embedding.size() == 2);

        vector<shared_ptr<POMDPAction> > result;

        auto Z_seq = hardware_spec.to_basis_gates_impl(Instruction(GateName::Z,
                                                     embedding.at(0)));
        Z_seq.push_back(Instruction(GateName::Write1, 1));
        const auto Z0 = make_shared<POMDPAction>("Z0", Z_seq, this->precision, vector<Instruction>({
                                                     Instruction(GateName::Z, 0)
                                                 }));
        result.push_back(Z0);

        auto X_seq = hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                                                     embedding.at(0)));
        X_seq.push_back(Instruction(GateName::Write1, 1));
        const auto X0 = make_shared<POMDPAction>("X0", X_seq, this->precision, vector<Instruction>({
                                                     Instruction(GateName::X, 0)
                                                 }));
        result.push_back(X0);

        auto P1 = make_shared<POMDPAction>("P1",
                                           vector<Instruction>({Instruction(GateName::Meas, embedding.at(1), 0)}),
                                           this->precision,
                                           vector<Instruction>({Instruction(GateName::Meas, 1, 0)}));
        result.push_back(P1);

        return result;
    }

    [[nodiscard]] vector<unordered_map<int, int> > get_hardware_scenarios(
        HardwareSpecification const &hardware_spec) const override {
        vector<unordered_map<int, int> > result;
        vector<int> pivot_qubits;
        if (hardware_spec.get_hardware() != QuantumHardware::PerfectHardware && hardware_spec.num_qubits < 14) {
            for (int qubit = 0; qubit < hardware_spec.num_qubits; qubit++) {
                pivot_qubits.push_back(qubit);
            }
        } else {
            for (auto q: get_meas_pivot_qubits(hardware_spec, 0)) {
                pivot_qubits.push_back(q);
            }
        }

        for (int q1_index = 0; q1_index < pivot_qubits.size(); q1_index++) {
            unordered_map<int, int> d_temp;
            int q0 = 0;
            while (q0 == pivot_qubits[q1_index]) {
                q0++;
            }
            d_temp[0] = q0;
            d_temp[1] = pivot_qubits[q1_index];
            result.push_back(d_temp);
        }

        return result;
    }


    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, this->nqvars);
        shared_ptr<Algorithm> on0 = make_shared<Algorithm>(action_mappings["Z0"], 0, this->precision, 1);

        if (horizon <= 2) {
            return normalize_algorithm(on0);
        }

        int tot_meas = horizon - 2;
        shared_ptr<Algorithm> on1 = make_shared<Algorithm>(action_mappings["Z0"], 0, this->precision, 1);
        on1->children.push_back(make_shared<Algorithm>(action_mappings["X0"], 3, this->precision, 1));
        auto temp =
            this->build_meas_sequence(tot_meas, 0, action_mappings["P1"], make_shared<ClassicalState>(), on0, on1);


        return normalize_algorithm(temp);
    }
};
#endif
