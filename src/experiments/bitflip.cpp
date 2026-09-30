#ifndef BITFLIP_H
#define BITFLIP_H

#include <cassert>

#include "experiments.hpp"
#include "utils.hpp"
#include <complex>
#include <unordered_set>

using namespace std;

inline vector<pair<int, int>> get_selected_couplers(const HardwareSpecification &hardware_spec, int target) {
    if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware) {
        assert(target == 0);
        pair<int, int> first_pair = {1,2};
        return {first_pair, first_pair};

    }
    vector<pair<int, double>> couplers = hardware_spec.get_sorted_qubit_couplers(target);
    pair<int, int> first_pair = {couplers[0].first, couplers[1].first}; // most noisy pair of couplers for this target

    pair<int, int> second_pair = {couplers[couplers.size() -1].first, couplers[couplers.size() -2].first}; // least noisy pair of couplers for this target
    return {first_pair, second_pair};
}


inline bool does_result_contains_d(const vector<unordered_map<int, int>> &result, const unordered_map<int, int> &d) {
    for (auto d_ : result) {
        unordered_set<int> controls1({d.at(0), d.at(1)});
        unordered_set<int> controls2({d_.at(0), d_.at(1)});
        if(d_.at(2) == d.at(2) && controls1 == controls2) return true;
    }
    return false;
}
    

class IPMABitflip : public Experiment {

    bool is_even_parity_bell_state(const QuantumState &qs);

    public:
    vector<vector<complex<double>>> BELL0;
    vector<vector<complex<double>>> BELL1;
    vector<vector<complex<double>>> BELL2;
    vector<vector<complex<double>>> BELL3;
    IPMABitflip(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType>& method_types, const set<QuantumHardware>& hw_list, bool optimize) :
            Experiment(name, precision, with_thermalization, min_horizon, max_horizon,
                false, method_types, hw_list, optimize) {this->setup();};
        IPMABitflip() : Experiment() {
            this->name = "bitflip_ipma";
            this->precision = 8;
            this->with_thermalization = false;
            this->min_horizon = 4;
            this->max_horizon = 7;
            this->set_hidden_index = false;
            this->method_types.erase(MethodType::Convex);
            this->setup();
            this->nqvars = 3;
            this->ncvars = 1;

        }

        set<QuantumHardware> get_allowed_hardware() const override {
            set<QuantumHardware> result;
            for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
                QuantumHardware quantum_hardware = static_cast<QuantumHardware>(i);
                HardwareSpecification hs(quantum_hardware, false, false);
                if (hs.basis_gates_type != BasisGates::TYPE5 && hs.basis_gates_type != BasisGates::TYPE2) {
                    result.insert(quantum_hardware);
                }
            }
            return result;
        }

        void setup() {
            this->BELL0 = vector<vector<complex<double>>>(
                4, vector<complex<double>>(4)
            );
            this->BELL0[0][0] = complex<double>(0.5, 0.0);
            this->BELL0[3][0] = complex<double>(0.5, 0.0);
            this->BELL0[0][3] = complex<double>(0.5, 0.0);
            this->BELL0[3][3] = complex<double>(0.5, 0.0);


            this->BELL1 = vector<vector<complex<double>>>(
                4, vector<complex<double>>(4)
            );
            this->BELL1[0][0] = complex<double>(0.5, 0.0);
            this->BELL1[3][0] = complex<double>(-0.5, 0.0);
            this->BELL1[0][3] = complex<double>(-0.5, 0.0);
            this->BELL1[3][3] = complex<double>(0.5, 0.0);

            this->BELL2 = vector<vector<complex<double>>>(
                4, vector<complex<double>>(4)
            );
            this->BELL2[1][1] = complex<double>(0.5, 0.0);
            this->BELL2[1][2] = complex<double>(0.5, 0.0);
            this->BELL2[2][1] = complex<double>(0.5, 0.0);
            this->BELL2[2][2] = complex<double>(0.5, 0.0);

            this->BELL3 = vector<vector<complex<double>>>(
                4, vector<complex<double>>(4)
            );
            this->BELL3[1][1] = complex<double>(0.5, 0.0);
            this->BELL3[1][2] = complex<double>(-0.5, 0.0);
            this->BELL3[2][1] = complex<double>(-0.5, 0.0);
            this->BELL3[2][2] = complex<double>(0.5, 0.0);
        }

        [[nodiscard]] bool guard(const shared_ptr<POMDPVertex>& vertex, const unordered_map<int, int>& embedding, const shared_ptr<POMDPAction>& action) const override {
            if (*action == HALT_ACTION) return false;
            if (action->instruction_sequence[0].gate_name != GateName::Meas) return true;
            auto qs = vertex->hybrid_state->quantum_state;
            auto P0 = Instruction(GateName::P0, embedding.at(2));
            auto P1 = Instruction(GateName::P1, embedding.at(2));

            auto qs0 = qs->apply_instruction(P0);
            auto qs1 = qs->apply_instruction(P1);

            if (qs0 != nullptr) {
                auto pt0 = qs0->multi_partial_trace(vector<int>({embedding.at(2)}));
                if (!is_matrix_in_list(pt0, {BELL0, BELL1, BELL2, BELL3}, this->precision)) return false;
            }
                
            if (qs1 != nullptr) {
                auto pt1 = qs1->multi_partial_trace(vector<int>({embedding.at(2)}));
                return is_matrix_in_list(pt1, {BELL0, BELL1, BELL2, BELL3}, this->precision);
            }
                
            return true;

        }

        vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
            vector<pair<shared_ptr<HybridState>, double>> result;

            shared_ptr<ClassicalState> classical_state = make_shared<ClassicalState>();


            auto H0 = Instruction(GateName::H, embedding.at(0));
            auto CX01 = Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1));
            auto X0 = Instruction(GateName::X, embedding.at(0));
            auto Z0 = Instruction(GateName::Z, embedding.at(0));

            // prepare first bell state
            auto  temp0 =  make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
            auto  temp1 = temp0->apply_instruction(H0);
            auto bell0 = temp1->apply_instruction(CX01);
            result.emplace_back(make_shared<HybridState>(bell0, classical_state), 0.25);

            // prepare second bell state
            auto bell1 = bell0->apply_instruction(X0);
            result.push_back(make_pair(make_shared<HybridState>(bell1, classical_state), 0.25));
        
            // prepare third bell state
            auto bell2 = bell0->apply_instruction(Z0);
            result.push_back(make_pair(make_shared<HybridState>(bell2, classical_state), 0.25));

            // preapre fourth bell state
            auto bell3 = bell2->apply_instruction(X0);
            result.push_back(make_pair(make_shared<HybridState>(bell3, classical_state), 0.25));

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

    virtual vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

            assert(embedding.size() == 3);
            assert(embedding.find(0) != embedding.end());
            assert(embedding.find(1) != embedding.end());
            assert(embedding.find(2) != embedding.end());

            
            auto X0 = make_shared<POMDPAction>("X0", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::X, 0)}));

            auto P2 = make_shared<POMDPAction>("P2",
                vector<Instruction>({Instruction(GateName::Meas, embedding.at(2), 2)}),
                this->precision, 
                vector<Instruction>({Instruction(GateName::Meas, 2, 2)}));

            auto CX02 = make_shared<POMDPAction>("CX02",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(2)))
                , this->precision, vector<Instruction>({Instruction(GateName::Cnot, vector<int>({0}), 2)}));

            auto CX12 = make_shared<POMDPAction>("CX12",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(2))), 
                this->precision, 
                vector<Instruction>({Instruction(GateName::Cnot, vector<int>({1}), 2)}));

            return {X0, P2, CX02, CX12};
        }

        vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
            vector<unordered_map<int, int>> result;
            set<int> pivot_qubits;
            if (hardware_spec.get_hardware() != QuantumHardware::PerfectHardware && hardware_spec.num_qubits < 14) {
                
                for(int qubit = 0; qubit < hardware_spec.num_qubits; qubit++) {
                    if (hardware_spec.get_qubit_indegree(qubit) > 1) {
                        pivot_qubits.insert(qubit);
                    }
                        
                }
            } else {
                pivot_qubits = get_meas_pivot_qubits(hardware_spec, 2);
            }

            for (auto target : pivot_qubits) {
                for (auto p : get_selected_couplers(hardware_spec, target)) {
                    unordered_map<int, int> d_temp;
                    d_temp[0] = p.first;
                    d_temp[1] = p.second;
                    d_temp[2] = target;
                    if (!does_result_contains_d(result, d_temp)) result.push_back(d_temp);
                }
            }
            return result; 
        }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
            assert (method == MethodType::SingleDistBellman);
            auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
            auto action_mappings = this->get_actions_dictionary(hardware_spec, 3);
            shared_ptr<Algorithm> first_cx = make_shared<Algorithm>(action_mappings["CX02"], 0, 10, 1);
            shared_ptr<Algorithm> second_cx = make_shared<Algorithm>(action_mappings["CX12"], 0, 10, 1);
            shared_ptr<Algorithm> on1 = make_shared<Algorithm>(action_mappings["X0"], 0, 10, 1);
            shared_ptr<Algorithm> on0 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 10, 1);
            first_cx->children.push_back(second_cx);
            second_cx->children.push_back(this->build_meas_sequence(horizon-3, 2, action_mappings["P2"], make_shared<ClassicalState>(), on0, on1));
            return normalize_algorithm(first_cx);
    }

    string get_precondition(const MethodType &method) override {
        assert (this->precision == 8);
        string bell0_str = "[0.70710, 0, 0, 0.70710, 0, 0, 0, 0]";
        // string bell0_str = "[0, 0.70710, 0, 0, 0, 0, 0, 0.70710]";

        string bell1_str = "[0.70710, 0, 0, -0.70710, 0, 0, 0, 0]";
        // string bell1_str = "[0, -0.70710, 0, 0, 0, 0, 0, 0.70710]";

        string bell2_str = "[0, 0.70710, 0.70710, 0, 0, 0, 0, 0]";
        // string bell2_str = "[0, 0, 0,  0.70710, 0,  0.70710, 0, 0]";

        string bell3_str = "[0, 0.70710, -0.70710, 0, 0, 0, 0, 0]";
        // string bell3_str = "[0, 0, 0,  -0.70710, 0,  0.70710, 0, 0]";
        assert (method == MethodType::SingleDistBellman);
        return string("P([q0,q1,q2] = "+ bell0_str +" and [x2] = b0 ) = 0.25 ") + // |00> + |11>
            "and P([q0,q1,q2] = "+ bell1_str + " and [x2] = b0) = 0.25 and " + // |00> + |11>
            "P([q0,q1,q2] = " + bell2_str + " and [x2] = b0) = 0.25 and " + // |01> + |10>
            "P([q0,q1,q2] = " + bell3_str + " and [x2] = b0) = 0.25"  // |01> - |10>
            ;
    }

    string get_target_postcondition(const double &threshold) override {
        string bell0_str = "[0.70710,0,0,0.70710]";
        string bell1_str = "[0.70710,0,0,-0.70710]";
        return "P( q0,q1 = "+ bell0_str + " or q0,q1 = " + bell1_str  + ") >= " + to_string(threshold);
    }
};


class IPMA2Bitflip : public IPMABitflip {
public:
    IPMA2Bitflip(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) :
    IPMABitflip(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize){};
    IPMA2Bitflip() : IPMABitflip() {
        this->name = "bitflip_ipma2";
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

            assert(embedding.size() == 3);
            assert(embedding.find(0) != embedding.end());
            assert(embedding.find(1) != embedding.end());
            assert(embedding.find(2) != embedding.end());

            
            auto X0 = make_shared<POMDPAction>("X0", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::X, 0)}));

            auto P2 = make_shared<POMDPAction>("P2",
                vector<Instruction>({Instruction(GateName::Meas, embedding.at(2), 2)}),
                this->precision, 
                vector<Instruction>({Instruction(GateName::Meas, 2, 2)}));
            
            auto vCX02_instructions = hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(2)));

            auto vCX12_instructions = hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(2)));

            for (auto ins : vCX12_instructions) {
                vCX02_instructions.push_back(ins);
            }

            vector<Instruction> pseudo_instruction_CX = {Instruction(GateName::Cnot, vector<int>({0}), 2), Instruction(GateName::Cnot, vector<int>({1}), 2)};

            auto CX = make_shared<POMDPAction>("CX",
                vCX02_instructions, this->precision, pseudo_instruction_CX);

            return {X0, P2, CX};
        }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        assert (method == MethodType::SingleDistBellman);
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, 3);
        shared_ptr<Algorithm> cx = make_shared<Algorithm>(action_mappings["CX"], 0, 10, 1);
        shared_ptr<Algorithm> on1 = make_shared<Algorithm>(action_mappings["X0"], 0, 10, 1);
        shared_ptr<Algorithm> on0 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 10, 1);
        cx->children.push_back(this->build_meas_sequence(horizon-2, 2, action_mappings["P2"], make_shared<ClassicalState>(), on0, on1));
        return normalize_algorithm(cx);
    }
};

class IPMA3Bitflip : public IPMABitflip {
    public:
    IPMA3Bitflip(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) :
    IPMABitflip(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize){};
    IPMA3Bitflip() : IPMABitflip() {
        this->name = "bitflip_ipma3";
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

            assert(embedding.size() == 3);
            assert(embedding.find(0) != embedding.end());
            assert(embedding.find(1) != embedding.end());
            assert(embedding.find(2) != embedding.end());

            
            auto X0 = make_shared<POMDPAction>("X0", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::X, embedding.at(0))}));

            auto P2 = make_shared<POMDPAction>("P2",
                vector<Instruction>({Instruction(GateName::Meas, embedding.at(2), 2)}),
                this->precision, 
                vector<Instruction>({Instruction(GateName::Meas, embedding.at(2), 2)}));
            
            auto vCX02_instructions = hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(2)));

            auto vCX12_instructions = hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(1)}), embedding.at(2)));

            for (auto ins : vCX12_instructions) {
                vCX02_instructions.push_back(ins);
            }

            vector<Instruction> pseudo_instruction_CX = {Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(2)), Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1))};

            auto CX = make_shared<POMDPAction>("CX",
                vCX02_instructions, this->precision, pseudo_instruction_CX);

            
            auto X2 = make_shared<POMDPAction>("X2", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
                embedding.at(2))), this->precision, vector<Instruction>({Instruction(GateName::X, embedding.at(2))}));
            return {X0, P2, CX, X2};
        }
};

class CXHBitflip : public IPMABitflip {
    public:
    CXHBitflip(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) :
    IPMABitflip(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize){};
    CXHBitflip() : IPMABitflip() {
            this->name = "bitflip_cxh";
            this->min_horizon = 4;
            this->max_horizon = 7;
    };

    vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
        vector<unordered_map<int, int>> result;
        set<int> pivot_qubits;
        if (hardware_spec.get_hardware() != QuantumHardware::PerfectHardware && hardware_spec.num_qubits < 14) {

            for(int qubit = 0; qubit < hardware_spec.num_qubits; qubit++) {
                if (hardware_spec.get_qubit_indegree(qubit) > 1) {
                    pivot_qubits.insert(qubit);
                }

            }
        } else {
            pivot_qubits = get_meas_pivot_qubits(hardware_spec, 2);
        }

        for (auto target : pivot_qubits) {
            for (auto p : get_selected_couplers(hardware_spec, target)) {
                unordered_map<int, int> d_temp;
                d_temp[0] = p.first;
                d_temp[1] = target;
                d_temp[2] = p.second;
                if (!does_result_contains_d(result, d_temp)) result.push_back(d_temp);
            }
        }
        return result;
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

            assert(embedding.size() == 3);
            assert(embedding.find(0) != embedding.end());
            assert(embedding.find(1) != embedding.end());
            assert(embedding.find(2) != embedding.end());

            
            auto H1 = make_shared<POMDPAction>("H1", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
                embedding.at(1))), this->precision, vector<Instruction>({Instruction(GateName::H, 1)}));

            auto H2 = make_shared<POMDPAction>("H2", hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
                embedding.at(2))), this->precision, vector<Instruction>({Instruction(GateName::H, 2)}));

            auto P2 = make_shared<POMDPAction>("P2",
                vector<Instruction>({Instruction(GateName::Meas, embedding.at(2), 2)}),
                this->precision, 
                vector<Instruction>{Instruction(GateName::Meas, 2, 2)});

            auto CX21 = make_shared<POMDPAction>("CX21",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(2)}), embedding.at(1)))
                , this->precision, vector<Instruction>{Instruction(GateName::Cnot, vector<int>({2}), 1)});

            auto CX01 = make_shared<POMDPAction>("CX01",
                hardware_spec.to_basis_gates_impl(Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1))), 
                this->precision, 
                vector<Instruction>{Instruction(GateName::Cnot, vector<int>({0}), 1)});

        return {H2, H1, CX21, CX01, P2};
    }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        assert(horizon == 7);
        // H([q2]);
        // CNOT([q2,q1]);
        // H([q2]);
        // H([q1]);
        // CNOT([q2,q1]);
        // CNOT([q0,q1]);
        // x2 := measure(q2);

        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, 3);

        shared_ptr<Algorithm> head = make_shared<Algorithm>(action_mappings["H2"], 0, 10, 1);
        head->children.push_back(
            make_shared<Algorithm>(action_mappings["CX21"], 0, 10, 2)
            );

        auto second_ins = head->children.at(0);
        auto third_ins = make_shared<Algorithm>(action_mappings["H2"], 0, 10, 3);
        second_ins->children.push_back(third_ins);

        auto fourth_ins = make_shared<Algorithm>(action_mappings["H1"], 0, 10, 4);
        third_ins->children.push_back(fourth_ins);

        auto fifth_ins = make_shared<Algorithm>(action_mappings["CX21"], 0, 10, 5);
        fourth_ins->children.push_back(fifth_ins);
        auto sixth_ins = make_shared<Algorithm>(action_mappings["CX01"], 0, 10, 6);
        fifth_ins->children.push_back(sixth_ins);
        auto seventh_ins = make_shared<Algorithm>(action_mappings["P2"], 0, 10, 7);
        sixth_ins->children.push_back(seventh_ins);
        return normalize_algorithm(head);
    }
};

// CONVEX experiments
class BellStateDiscrimination2: public IPMA2Bitflip {
    map<int, vector<vector<complex<double>>>> indices_to_matrix;
public:
    BellStateDiscrimination2(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) : IPMA2Bitflip(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize) {

        this->set_hidden_index = true;
        this->indices_to_matrix[0] = this->BELL0;
        this->indices_to_matrix[1] = this->BELL1;
        this->indices_to_matrix[2] = this->BELL2;
        this->indices_to_matrix[3] = this->BELL3;
    };

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
                auto cs = hybrid_state->classical_state;

                if (cs->get_memory_val() == it.first->hidden_index) {
                    result = result + it.second;
                    this->target_vertices[it.first->id] = true;
                } else {
                    this->target_vertices[it.first->id] = false;
                }
            }
        }
        return result;
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        double result = 0;
        for (auto it : belief.probs) {
            auto is_target = this->target_vertices.find(it.first->id);
            if (is_target != this->target_vertices.end()) {
                if (is_target->second) {
                    result = result + it.second;
                }
            } else {
                auto hybrid_state = it.first->hybrid_state;
                auto cs = hybrid_state->classical_state;

                if (cs->get_memory_val() == it.first->hidden_index) {
                    result = result + it.second;
                    this->target_vertices[it.first->id] = true;
                } else {
                    this->target_vertices[it.first->id] = false;
                }
            }
        }
        return result;
    }

};

class BellStateDiscrimination3: public BellStateDiscrimination2, IPMA3Bitflip {
    map<int, vector<vector<complex<double>>>> indices_to_matrix;
public:
    BellStateDiscrimination3(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) : BellStateDiscrimination2(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize) {
    };

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        return BellStateDiscrimination2::postcondition(belief, embedding);
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        return BellStateDiscrimination2::postcondition_double(belief, embedding);
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {
        return IPMA3Bitflip::get_actions(hardware_spec, embedding);
    }

    void run() override{
        return BellStateDiscrimination2::run();
    }

};
#endif