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
    shared_ptr<QuantumState> get_target_state(const int &hidden_index, const unordered_map<int, int> &embedding, bool add_phase=false) const {
        assert(hidden_index <=1 && hidden_index >= 0);

        auto X0 = Instruction(GateName::X, embedding.at(0));
        auto H0 = Instruction(GateName::H, embedding.at(0));

        auto state = make_shared<QuantumState>(get_qubits_used(embedding), this->precision);

        if (hidden_index == 1) {
            state = state->apply_instruction(X0);
        }

        state = state->apply_instruction(H0);

        if (add_phase) {
            if (hidden_index == 1) {
                state = state->apply_instruction(X0);
            }
        }

        return state;

    }


    void set_num_vars() override {
        this->nqvars = 2;
        this->ncvars = 1;
    }

    void set_hidden_index_to() override {
        this->set_hidden_index = true;
    }

    void set_min_max_horizon(const MethodType& method) override {
        this->min_horizon = 2;
        this->max_horizon = 7;
    }

    void set_methods() override {
        this->method_types.clear();
        // this->method_types.insert(MethodType::SingleDistBellman);
        this->method_types.insert(MethodType::Convex);
    }

    void set_global_equality() override {
        QuantumState::use_global_eq = false;
    }

    public:
    vector<vector<complex<double>>> DM_PLUS;
    vector<vector<complex<double>>> DM_MINUS;
    PhaseReach(const string &name, const set<QuantumHardware> &hw_list) : IPMA(name, hw_list) {
        this->setup_params();
        this->DM_PLUS = vector<vector<complex<double>>>(
                2, vector<complex<double>>(2)
            );
        this->DM_PLUS[0][0] = complex<double>(0.5, 0.0);
        this->DM_PLUS[0][1] = complex<double>(0.5, 0.0);
        this->DM_PLUS[1][0] = complex<double>(0.5, 0.0);
        this->DM_PLUS[1][1] = complex<double>(0.5, 0.0);


        this->DM_MINUS = vector<vector<complex<double>>>(
        2, vector<complex<double>>(2)
        );
        this->DM_MINUS[0][0] = complex<double>(0.5, 0.0);
        this->DM_MINUS[0][1] = complex<double>(0.5, 0.0);
        this->DM_MINUS[1][0] = complex<double>(0.5, 0.0);
        this->DM_MINUS[1][1] = complex<double>(-0.5, 0.0);
    };

   [[nodiscard]] bool guard(const shared_ptr<POMDPVertex>& vertex, const unordered_map<int, int>& embedding, const shared_ptr<POMDPAction>& action) const override {
            if (*action == HALT_ACTION) return false;
            if (action->instruction_sequence[0].gate_name != GateName::Meas) return true;
            auto qs = vertex->hybrid_state->quantum_state;
            auto P0 = Instruction(GateName::P0, embedding.at(1));
            auto P1 = Instruction(GateName::P1, embedding.at(1));

            auto qs0 = qs->apply_instruction(P0);
            auto qs1 = qs->apply_instruction(P1);

            if (qs0 != nullptr) {
                auto pt0 = qs0->multi_partial_trace(vector<int>({embedding.at(1)}));
                if (!is_matrix_in_list(pt0, {DM_PLUS, DM_MINUS}, this->precision)) return false;
            }

            if (qs1 != nullptr) {
                auto pt1 = qs1->multi_partial_trace(vector<int>({embedding.at(1)}));
                return is_matrix_in_list(pt1, {DM_PLUS, DM_MINUS}, this->precision);
            }

            return true;

        }

        vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
            assert (embedding.size() == 2);
            vector<pair<shared_ptr<HybridState>, double>> result;

            auto classical_state = make_shared<ClassicalState>();

            auto state0 = this->get_target_state(0, embedding);
            result.emplace_back(new HybridState(state0, classical_state), 0.5);


            auto state1 = this->get_target_state(1, embedding);
            result.emplace_back(make_shared<HybridState>(state1, classical_state), 0.5);

            return result;
        }

        MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
            MyFloat answer("0", this->precision*(this->max_horizon+1));

            for(const auto& it : belief.probs) {
                auto qs = it.first->hybrid_state->quantum_state;
                auto is_target = this->target_vertices.find(it.first->id);
                if (is_target != this->target_vertices.end()) {
                    if (is_target->second) {
                        answer = answer + it.second;
                    }
                } else {
                    auto target_state = this->get_target_state(it.first->hidden_index, embedding, true);
                    auto temp = qs->apply_projector(target_state, embedding.at(0));
                    if (temp != nullptr && *qs == *temp) {
                        answer = answer + it.second;
                        this->target_vertices[it.first->id] =  true;
                        // cout << "target" << *qs  << " / " << *target_state << " / " << it.first->hidden_index << endl;
                    } else {
                        // cout << "not target" << *qs  << " / " << *target_state  << " " << it.first->hidden_index << endl;
                        this->target_vertices[it.first->id] =  false;
                    }
                }
            }

            return answer;
        }


        double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
                assert(false);
                double answer = 0;

                // for(const auto& it : belief.probs) {
                //     auto qs = it.first->hybrid_state->quantum_state;
                //     auto is_target = this->target_vertices.find(it.first->id);
                //     if (is_target != this->target_vertices.end()) {
                //         if (is_target->second) {
                //             answer = answer + it.second;
                //         }
                //     } else {
                //         auto target_state = this->get_target_state(it.first->hidden_index, embedding, true);
                //         auto temp = qs->apply_projector(target_state);
                //         if (temp != nullptr && *target_state == *temp) {
                //             answer = answer + it.second;
                //             this->target_vertices[it.first->id] =  true;
                //             cout << "target" << *qs  << " / " << *target_state << " / " << it.first->hidden_index << endl;
                //         } else {
                //             cout << "not target" << *qs  << " / " << *target_state  << " " << it.first->hidden_index << endl;
                //             this->target_vertices[it.first->id] =  false;
                //         }
                //     }
                // }

                return answer;
            }

        vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {
            assert(embedding.size() == 2);

            vector<shared_ptr<POMDPAction>> result;

            const auto H0 = make_shared<POMDPAction>("H0", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
                            embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::H, 0)}));
            result.push_back(H0);

            const auto H1 = make_shared<POMDPAction>("H1", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
                            embedding.at(1))), this->precision, vector<Instruction>({Instruction(GateName::H, 1)}));
            result.push_back(H1);

            const auto Z1 = make_shared<POMDPAction>("Z1", hardware_spec.to_basis_gates_impl(Instruction(GateName::Z,
                                embedding.at(1))), this->precision, vector<Instruction>({Instruction(GateName::Z, 1)}));
            result.push_back(Z1);

        auto P1 = make_shared<POMDPAction>("P1",
          vector<Instruction>({Instruction(GateName::Meas, embedding.at(1), 0)}),
          this->precision,
          vector<Instruction>({Instruction(GateName::Meas, 1, 0)}));
        result.push_back(P1);

       if (hardware_spec.does_coupler_exist(embedding.at(1), embedding.at(0))) {
           const auto CX10 = make_shared<POMDPAction>("CX10",
               hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(0)))
               , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({1}), 0)}));
           result.push_back(CX10);
       }

            if (hardware_spec.does_coupler_exist(embedding.at(0), embedding.at(1))) {
                const auto CX01 = make_shared<POMDPAction>("CX01",
            hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1)))
            , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({0}), 1)}));
                result.push_back(CX01);
            }

            return result;
        }

        [[nodiscard]] vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
            vector<unordered_map<int, int>> result;
            if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware) {
                unordered_map<int, int> d_temp;;
                d_temp[0] = 0;
                d_temp[1] = 1;
                return {d_temp};
            }

            vector<pair<pair<int, int>, double>> couplers = hardware_spec.get_sorted_qubit_couplers2();

            int total_embeddings = 3;
            if (couplers.size() < total_embeddings) {
                total_embeddings = couplers.size();
            }

            for (int i = 0; i < total_embeddings; i++) {
                const int q0 = couplers[i].first.first;
                const int q1 = couplers[i].first.second;
                unordered_map<int, int> d_temp;
                d_temp[0] = q0;
                d_temp[1] = q1;
                result.push_back(d_temp);
            }
            return result;
        }



    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, 1);
        shared_ptr<Algorithm> on1 = make_shared<Algorithm>(action_mappings["X0"], 0, 10, 1);
        shared_ptr<Algorithm> on0 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 10, 1);
        return normalize_algorithm(this->build_meas_sequence(horizon-1, 0, action_mappings["P0"], make_shared<ClassicalState>(), on0, on1));
    }

    string get_precondition(const MethodType &method) override {
        string state0 = "[1,0]";
        string state1 = "[0,1]";
        if (method == MethodType::SingleDistBellman) {
            return "P([q0] = "+ state0+" and [x0] = b0) = 0.5 and " + "P([q0] = "+ state1+" and [x0] = b0) = 0.5";
        }
        assert(method == MethodType::Convex);
        return "P([q0] = "+ state0+" and [x0] = b0) = 1 + " + "P([q0] = "+ state1+" and [x0] = b0) = 1";
    }

    string get_target_postcondition(const double &threshold) override {
        return "P( q0 = [1,0]) >= " + to_string(threshold);
    }
};
#endif