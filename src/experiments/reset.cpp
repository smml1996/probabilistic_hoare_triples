#ifndef RESET_H
#define RESET_H
#include <cassert>

#include "experiments.hpp"
using namespace std;

class ResetProblem : public Experiment {
protected:
    void set_uses_cnot() override {
        this->uses_cnot = false;
    }

    void set_global_equality() override {
        QuantumState::use_global_eq = true;
    }

    void set_num_vars() override {
        this->nqvars = 1;
        this->ncvars = 1;
    }

    void set_min_max_horizon(const MethodType &method) override {
        this->min_horizon = 2;
        if (method == MethodType::SingleDistBellman) {
            this->max_horizon = 9;
        } else {
            this->max_horizon = 7;
        }
    }

    void set_methods() override {
        this->method_types.insert(MethodType::SingleDistBellman);
        this->method_types.insert(MethodType::Convex);
        // this->method_types.insert( MethodType::Naive);
    }

public:
    ResetProblem(const string &name, const set<QuantumHardware> &hw_list) : Experiment(name, hw_list) {
        this->setup_params();
    };

    vector<pair<shared_ptr<HybridState>, double> >
    get_initial_distribution(const unordered_map<int, int> &embedding) const override {
        vector<pair<shared_ptr<HybridState>, double> > result;

        auto classical_state = make_shared<ClassicalState>();

        auto X0 = Instruction(GateName::X, embedding.at(0));

        // prepare first bell state
        auto state0 = make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
        result.emplace_back(new HybridState(state0, classical_state), 0.5);

        // prepare second bell state
        auto state1 = state0->apply_instruction(X0);
        assert(embedding.size() == 1);
        assert(!(*state1 == *state0));
        result.emplace_back(make_shared<HybridState>(state1, classical_state), 0.5);

        return result;
    }

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        auto state0 = make_shared<QuantumState>(vector<int>({embedding.at(0)}), this->precision);
        MyFloat answer("0", this->precision * (this->max_horizon + 1));

        for (const auto &it: belief.probs) {
            auto is_target = this->target_vertices.find(it.first->id);
            if (is_target != this->target_vertices.end()) {
                if (is_target->second) {
                    answer = answer + it.second;
                }
            } else if (*(it.first->hybrid_state->quantum_state) == *state0) {
                answer = answer + it.second;
                this->target_vertices[it.first->id] = true;
            } else {
                this->target_vertices[it.first->id] = false;
            }
        }

        return answer;
    }

    vector<shared_ptr<POMDPAction> > get_actions(HardwareSpecification &hardware_spec,
                                                 const unordered_map<int, int> &embedding) const override {
        assert(embedding.size() == 1);
        assert(embedding.find(0) != embedding.end());


        auto X0 = make_shared<POMDPAction>("X0", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                                               embedding.at(0))), this->precision,
                                           vector<Instruction>({Instruction(GateName::X, 0)}));

        auto P0 = make_shared<POMDPAction>("P0",
                                           vector<Instruction>({Instruction(GateName::Meas, embedding.at(0), 0)}),
                                           this->precision,
                                           vector<Instruction>({Instruction(GateName::Meas, 0, 0)}));

        return {X0, P0};
    }

    [[nodiscard]] vector<unordered_map<int, int> > get_hardware_scenarios(
        HardwareSpecification const &hardware_spec) const override {
        vector<unordered_map<int, int> > result;
        auto pivot_qubits = get_meas_pivot_qubits(hardware_spec, 0);
        for (auto target: pivot_qubits) {
            unordered_map<int, int> d_temp;
            d_temp[0] = target;
            result.push_back(d_temp);
        }
        return result;
    }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, 1);
        shared_ptr<Algorithm> on1 = make_shared<Algorithm>(action_mappings["X0"], 0, 10, 1);
        shared_ptr<Algorithm> on0 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 10, 1);
        return normalize_algorithm(
            this->build_meas_sequence(horizon - 1, 0, action_mappings["P0"], make_shared<ClassicalState>(), on0, on1));
    }
};
#endif
