//
// Created by Stefanie Muroya Lei on 01.10.25.
//

#include <cassert>
#include <queue>

#include "experiments.hpp"
#include "utils.hpp"
#include <unordered_set>
#include <absl/strings/str_format.h>

#include "bitflip.cpp"

using namespace std;

class BellStateReach : public IPMABitflip {
    public:
    BellStateReach(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
    const set<MethodType>& method_types, const set<QuantumHardware>& hw_list, bool optimize) : IPMABitflip(name, precision, with_thermalization, min_horizon, max_horizon,
                method_types, hw_list, optimize) {
        this->method_types.insert(MethodType::ConvexDist);
        this->nqvars = 3;
        this->ncvars = 1;
    }

    BellStateReach()  : IPMABitflip(){ this->name = "BellStateReach"; };

    vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
        vector<pair<shared_ptr<HybridState>, double>> result;
        shared_ptr<ClassicalState> classical_state0 = make_shared<ClassicalState>();

        auto H0 = Instruction(GateName::H, embedding.at(0));

        auto X0 = Instruction(GateName::X, embedding.at(0));
        auto Z0 = Instruction(GateName::Z, embedding.at(0));

        auto X2 = Instruction(GateName::X, embedding.at(2));

        auto  state0 =  make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
        result.push_back(make_pair(make_shared<HybridState>(state0, classical_state0), 0.2)); // |00>

        auto state1 = state0->apply_instruction(X0); // |10>
        result.push_back(make_pair(make_shared<HybridState>(state1, classical_state0), 0.2));

        auto state_plus = state0->apply_instruction(H0);
        state_plus = state_plus->apply_instruction(X2); // |+1>
        result.push_back(make_pair(make_shared<HybridState>(state_plus, classical_state0), 0.3));

        auto state_minus = state_plus->apply_instruction(Z0);
        result.push_back(make_pair(make_shared<HybridState>(state_minus, classical_state0), 0.3));

        return result;
    }

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        MyFloat result("0", this->precision*(this->max_horizon+1));
        for (auto it : belief.probs) {
            auto is_target = this->target_vertices.find(it.first->id);
            if (is_target != this->target_vertices.end()) {
                if (is_target->second) {
                    result = result + it.second;
                }
            } else {
                auto hybrid_state = it.first->hybrid_state;
                auto qs = hybrid_state->quantum_state;
                auto current_rho = qs->multi_partial_trace(vector<int>({embedding.at(2)}));
                assert (current_rho.size() == 4);
                if (are_matrices_equal(current_rho, this->BELL0, this->precision) || are_matrices_equal(current_rho, this->BELL1, this->precision)) { // equality up to a threshold because there might be floating point overflow
                    result = result + it.second;
                    this->target_vertices[it.first->id] =  true;
                }  else {
                    this->target_vertices[it.first->id] =  false;
                }
            }

        }
        return result;
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        double result = 0.0;
        for (auto it : belief.probs) {
            auto is_target = this->target_vertices.find(it.first->id);
            if (is_target != this->target_vertices.end()) {
                if (is_target->second) {
                    result = result + it.second;
                }
            } else {
                auto hybrid_state = it.first->hybrid_state;
                auto qs = hybrid_state->quantum_state;
                auto current_rho = qs->multi_partial_trace(vector<int>({embedding.at(2)}));
                assert (current_rho.size() == 4);
                if (are_matrices_equal(current_rho, this->BELL0, this->precision) || are_matrices_equal(current_rho, this->BELL1, this->precision)) { // equality up to a threshold because there might be floating point overflow
                    result = result + it.second;
                    this->target_vertices[it.first->id] =  true;
                    }  else {
                        this->target_vertices[it.first->id] =  false;
                    }
            }

        }
        return result;
    }
    [[nodiscard]] bool guard(const shared_ptr<POMDPVertex>& vertex, const unordered_map<int, int>& embedding, const shared_ptr<POMDPAction>& action) const override {
        return true;
    }


    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

        assert(embedding.size() == 3);
        assert(embedding.find(0) != embedding.end());
        assert(embedding.find(1) != embedding.end());
        assert(embedding.find(2) != embedding.end());


        vector<Instruction> meas_data_seq({Instruction(GateName::Meas, embedding.at(2), 2)});
        vector<Instruction> v_meas_data_seq({Instruction(GateName::Meas, 2, 2)});
        auto MEASData = make_shared<POMDPAction>("MEASData",meas_data_seq, this->precision, v_meas_data_seq);

        vector<Instruction> seqH0;
        for (auto it : hardware_spec.to_basis_gates_impl(Instruction(GateName::H, embedding.at(0)))) {
            seqH0.push_back(it);
        }

        auto H0 = make_shared<POMDPAction>("H0", seqH0, this->precision, vector<Instruction>({
                Instruction(GateName::H, 0)
                    }));


        auto CX01 = make_shared<POMDPAction>("CX01",vector<Instruction>({Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1))}), this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({0}), 1)}));
        return {H0, CX01, MEASData};
    }

    static set<int> get_fourth(const HardwareSpecification &hardware_spec, unordered_set<int> invalid_qubits) {
        auto vals = get_meas_pivot_qubits(hardware_spec, 0);
        for (auto it : invalid_qubits) {
            vals.erase(it);
        }
        return vals;
    }

    vector<int> get_shortest_path(const HardwareSpecification &hardware_spec, const int &source, const int &target) const {
        queue<pair<int, int>> q;
        unordered_set<int> visited;

        q.push(make_pair(source, 0));
        visited.insert(source);
        unordered_map<int, int> paths;
        while (!q.empty()) {
            auto current = q.front();
            auto qubit = current.first;
            auto distance = current.second;
            assert(qubit != target);
            q.pop();


            auto it = hardware_spec.digraph.find(qubit);
            if (it != hardware_spec.digraph.end()) {
                for (auto qubit2 : hardware_spec.digraph.at(qubit)) {
                    if (paths.find(qubit2)  == paths.end()) {
                        paths[qubit2] = qubit;
                    }
                    if (qubit2 == target) {
                        vector<int> answer;
                        int last_seen = target;
                        answer.push_back(target);
                        while (last_seen != source) {
                            last_seen = paths[last_seen];
                            answer.push_back(last_seen);
                        }
                        reverse(answer.begin(), answer.end());
                        return answer;
                    }

                    if (visited.find(qubit2) == visited.end()) {
                        q.push(make_pair(qubit2, distance + 1));
                        visited.insert(qubit2);
                    }
                }
            }
        }
        return {};
    }

    vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
        if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware ) {
            unordered_map<int, int> m;
            m[0] = 0;
            m[1] = 1;
            m[2] = 2;
            return {m};
        }
        vector<unordered_map<int, int>> result;
        vector<pair<pair<int, int>, double>> couplers = hardware_spec.get_sorted_qubit_couplers2();
        pair<int, int> first_pair = {couplers[0].first.first, couplers[0].first.second}; // most noisy pair of couplers for this target
        vector<pair<int, int>> selected_couplers;
        selected_couplers.push_back(first_pair);
        if (couplers.size() > 0) {
            pair<int, int> second_pair = {couplers[couplers.size() -1].first.first, couplers[couplers.size() -1].first.second}; // least noisy pair of couplers for this target
            selected_couplers.push_back(second_pair);
        }

        for (auto it_source : selected_couplers) {
            auto qubit0 = it_source.first;
            auto qubit1 = it_source.second;
            assert(qubit0 != qubit1);
            auto possible_fourth = get_fourth(hardware_spec, {qubit0, qubit1});
            for (auto qubit2 : possible_fourth) {
                unordered_map<int, int> m;
                m[0] = qubit0;
                m[1] = qubit1;
                m[2] = qubit2;
                result.push_back(m);
            }

        }
        return result;
    }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        map<string, shared_ptr<POMDPAction>> action_mappings;
        unordered_map<int, int> embedding;
        embedding[0] = 0;
        embedding[1] = 1;
        embedding[2] = 2;

        auto actions = this->get_actions(hardware_spec, embedding);

        for (auto action : actions) {
            action_mappings[action->name] = action;
        }

        auto on0_algorithm = make_shared<Algorithm>(action_mappings["H0"], 0, 10, 1);
        auto on1_algorithm = make_shared<Algorithm>(action_mappings["CX01"], 1, 10, 1);
        auto meas_action = action_mappings["MEASData"];
        if (horizon == 1 || (horizon == 2 && method == MethodType::SingleDistBellman)) {
            on1_algorithm = normalize_algorithm(on1_algorithm);
            return on1_algorithm;

        }
        if (horizon == 2) {
            assert(MethodType::ConvexDistHull);
            on0_algorithm = normalize_algorithm(on0_algorithm);
            auto new_head = make_shared<Algorithm>(make_shared<POMDPAction>(random_branch), 0, 5, -1); // we are not going to use precisio
            new_head->children.push_back(on0_algorithm);
            new_head->children.push_back(on1_algorithm);
            new_head->children_probs.insert({0, 0.5});
            new_head->children_probs.insert({1, 0.5});
            return new_head;
        }
        on0_algorithm->children.push_back(make_shared<Algorithm>(action_mappings["CX01"], 0, 10, 1));
        return normalize_algorithm(this->build_meas_sequence(horizon-2, 2, meas_action,
            make_shared<ClassicalState>(), on0_algorithm, on1_algorithm));
    }

    string get_precondition(const MethodType &method) override {
        assert (this->precision == 8);
        string state000 = "[1,0,0,0,0,0,0,0]";
        string state100 = "[0,1,0,0,0,0,0,0]";
        string statePlus = "[0,0,0,0, 0.70710678, 0.70710678, 0,0]";
        string stateMinus = "[0,0,0,0, 0.70710678, -0.70710678, 0,0]";
        if (method == MethodType::SingleDistBellman) {
            return string("P([q0,q1,q2] = "+ state000 +" and [x2] = b0 ) = 0.2 and ") + // |000>
            "P([q0,q1,q2] = "+ state100 + " and [x2] = b0) = 0.2 and " + // |100>
            "P([q0,q1,q2] = " + statePlus + " and [x2] = b0) = 0.3 and " + // |+01>
            "P([q0,q1,q2] = " + stateMinus + " and [x2] = b0) = 0.3"  // |-01>
            ;
        }

        assert (method == MethodType::ConvexDist);
        return string("P([q0,q1,q2] = "+ state000 +" and [x2] = b0 ) = 1 + ") + // |00> + |11>
            "P([q0,q1,q2] = "+ state100 + " and [x2] = b0) = 1 + " + // |00> + |11>
            "P([q0,q1,q2] = " + statePlus + " and [x2] = b0) = 1 + " + // |01> + |10>
            "P([q0,q1,q2] = " + stateMinus + " and [x2] = b0) = 1"  // |01> - |10>
            ;
    }
};