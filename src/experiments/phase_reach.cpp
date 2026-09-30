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
    void set_num_vars() override {
        this->nqvars = 2;
        this->ncvars = 1;
    }

    void set_hidden_index_to() override {
        this->set_hidden_index = true;
    }

    void set_min_max_horizon(const MethodType& method) override {
        this->min_horizon = 2;
        this->max_horizon = 8;
    }

    void set_methods() override {
        this->method_types.insert(MethodType::SingleDistBellman);
        this->method_types.insert(MethodType::Convex);
        this->method_types.insert( MethodType::Naive);
    }

    public:
    PhaseReach(const string &name, const set<QuantumHardware> &hw_list) : IPMA(name, hw_list) {
        this->setup_params();
    };

        vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
            assert (embedding.size() == 2);
            vector<pair<shared_ptr<HybridState>, double>> result;

            auto classical_state = make_shared<ClassicalState>();

            auto X0 = Instruction(GateName::X, embedding.at(0));
            auto X1 = Instruction(GateName::X, embedding.at(0));

            // prepare first bell state
            auto state0 = make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
            result.emplace_back(new HybridState(state0, classical_state), 0.25);


            // prepare second bell state
            auto state1 = state0->apply_instruction(X0);
            result.emplace_back(make_shared<HybridState>(state1, classical_state), 0.25);

            auto state2 = state0->apply_instruction(X1);
            result.emplace_back(make_shared<HybridState>(state2, classical_state), 0.25);

            auto state3 = state2->apply_instruction(X0);
            result.emplace_back(make_shared<HybridState>(state3, classical_state), 0.25);

            return result;
        }

        MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
            auto state0 = make_shared<QuantumState>(vector<int>({embedding.at(0)}), this->precision);
            MyFloat answer("0", this->precision*(this->max_horizon+1));

            for(const auto& it : belief.probs) {
                auto qs = it.first->hybrid_state->quantum_state;
                auto is_target = this->target_vertices.find(it.first->id);
                if (is_target != this->target_vertices.end()) {
                    if (is_target->second) {
                        answer = answer + it.second;
                    }
                } else {
                    auto amplitude = qs->get_amplitude(it.first->hidden_index);
                    if (it.first->hidden_index % 2 == 0) {
                        if (is_close(amplitude, complex<double>(1,0), this->precision)) {
                            answer = answer + it.second;
                            this->target_vertices[it.first->id] =  true;
                        } else {
                            this->target_vertices[it.first->id] =  false;
                        }
                    } else {
                        if (is_close(amplitude, complex<double>(-1,0), this->precision)) {
                            answer = answer + it.second;
                            this->target_vertices[it.first->id] =  true;
                        } else {
                            this->target_vertices[it.first->id] =  false;
                        }
                    }
                }
            }

            return answer;
        }


        double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
                auto state0 = make_shared<QuantumState>(vector<int>({embedding.at(0)}), this->precision);
                double answer = 0;

                for(const auto& it : belief.probs) {
                    auto qs = it.first->hybrid_state->quantum_state;
                    auto is_target = this->target_vertices.find(it.first->id);
                    if (is_target != this->target_vertices.end()) {
                        if (is_target->second) {
                            answer = answer + it.second;
                        }
                    } else {
                        auto amplitude = qs->get_amplitude(it.first->hidden_index);
                        if (it.first->hidden_index % 2 == 0) {
                            if (is_close(amplitude, complex<double>(1,0), this->precision)) {
                                answer = answer + it.second;
                                this->target_vertices[it.first->id] =  true;
                            } else {
                                this->target_vertices[it.first->id] =  false;
                            }
                        } else {
                            if (is_close(amplitude, complex<double>(-1,0), this->precision)) {
                                answer = answer + it.second;
                                this->target_vertices[it.first->id] =  true;
                            } else {
                                this->target_vertices[it.first->id] =  false;
                            }
                        }
                    }
                }

                return answer;
            }

        vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

            assert(embedding.size() == 2);

            vector<shared_ptr<POMDPAction>> result;

            auto Z0 = make_shared<POMDPAction>("Z0", hardware_spec.to_basis_gates_impl(Instruction(GateName::Z,
                embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::Z, 0)}));
            result.push_back(Z0);

            auto Z1 = make_shared<POMDPAction>("Z1", hardware_spec.to_basis_gates_impl(Instruction(GateName::Z,
                embedding.at(1))), this->precision, vector<Instruction>({Instruction(GateName::Z, 1)}));
            result.push_back(Z1);

            if (hardware_spec.does_coupler_exist(embedding.at(0), embedding.at(1))) {
                auto CX01 = make_shared<POMDPAction>("CX01",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1)))
                , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({0}), 1)}));
                result.push_back(CX01);
            }

            if (hardware_spec.does_coupler_exist(embedding.at(1), embedding.at(0))) {
                auto CX10 = make_shared<POMDPAction>("CX10",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(0)))
                , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({1}), 0)}));
                result.push_back(CX10);
            }

            return result;
        }

        [[nodiscard]] vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
            vector<unordered_map<int, int>> result;

            for (auto it : hardware_spec.digraph) {
                for (auto it2 : it.second) {
                    assert (it2 != it.first);
                    if (it2 > it.first) {
                        unordered_map<int, int> d_temp;;
                        d_temp[0] = it.first;
                        d_temp[1] = it2;
                        result.push_back(d_temp);
                    }
                }
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

class PhaseReach2 : public PhaseReach {
public:
    PhaseReach2(const string &name, const set<QuantumHardware> &hw_list) : PhaseReach(name, hw_list){};

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

        assert(embedding.size() == 2);

        vector<shared_ptr<POMDPAction>> result;

        auto H0 = make_shared<POMDPAction>("H0", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
            embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::H, 0)}));
        result.push_back(H0);

        auto H1 = make_shared<POMDPAction>("H1", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
            embedding.at(1))), this->precision, vector<Instruction>({Instruction(GateName::H, 1)}));
        result.push_back(H1);

        if (hardware_spec.does_coupler_exist(embedding.at(0), embedding.at(1))) {
            auto CX01 = make_shared<POMDPAction>("CX01",
            hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1)))
            , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({0}), 1)}));
            result.push_back(CX01);
        }

        if (hardware_spec.does_coupler_exist(embedding.at(1), embedding.at(0))) {
            auto CX10 = make_shared<POMDPAction>("CX10",
            hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(0)))
            , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({1}), 0)}));
            result.push_back(CX10);
        }

        return result;
    }
};
#endif