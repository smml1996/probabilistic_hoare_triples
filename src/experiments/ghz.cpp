#ifndef GHZ_H
#define GHZ_H
#include <absl/strings/internal/str_format/extension.h>
#include <queue>
#include "experiments.hpp"

inline bool are_adjacent_qubits(const map<int, set<int>> &graph, int qubit1,
                         const unordered_set<int> &qubits) {
    queue<int> q;
    unordered_set<int> visited;

    q.push(qubit1);
    visited.insert(qubit1);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        auto it = graph.find(current);
        if (it != graph.end()) {
            for (int succ : it->second) {
                if (visited.find(succ) == visited.end()) {
                    if (qubits.find(succ) != qubits.end()) {
                        visited.insert(succ);
                        q.push(succ);
                    }
                }
            }
        }
    }
    for (auto qubit : qubits) {
        if (visited.find(qubit) == visited.end()) {
            return false;
        }
    }
    return true;
}

inline bool is_repeated_embedding(const vector<unordered_map<int, int>> &all_embeddings, const unordered_map<int, int> &current) {
    unordered_set<int> current_set;
    for (auto it : current) {
        current_set.insert(it.second);
    }
        
    for (auto embedding : all_embeddings) {
        unordered_set<int> temp_s;
        for (const auto it : embedding)
            temp_s.insert(it.second);
        if (temp_s == current_set)
            return true;
    }
    return false;
}

// GHZ state preparation of 3 qubits
class GHZStatePreparation3 : public Experiment {
    public:
    GHZStatePreparation3(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
    const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) :
Experiment(name, precision, with_thermalization, min_horizon, max_horizon, false, method_types, hw_list, optimize){};
        GHZStatePreparation3() : Experiment() {
            this->name = "ghz_state_preparation3";
            this->precision = 8;
            this->with_thermalization = false;
            this->min_horizon = 3;
            this->max_horizon = 3;
            this->set_hidden_index = false;
            this->method_types.erase(MethodType::Convex); // there is only one initial state, is not worth it
            this->nqvars = 3;
            this->ncvars = 1;
        };

        set<QuantumHardware> get_allowed_hardware() const override{
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

        virtual shared_ptr<QuantumState> get_target_state(const unordered_map<int, int> &embedding) const {
            auto H0 = Instruction(GateName::H, embedding.at(0));
            auto CX01 = Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1));
            auto CX12 = Instruction(GateName::Cnot, vector<int>({embedding.at(1)}),embedding.at(2));
            auto qs_ = QuantumState({embedding.at(0), embedding.at(1), embedding.at(2)}, this->precision);
            auto qs0 = qs_.apply_instruction(H0);
            auto qs1 = qs0->apply_instruction(CX01);
            auto qs = qs1->apply_instruction(CX12);
            return qs;
        }


        vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
            vector<pair<shared_ptr<HybridState>, double>> result;

            auto classical_state = make_shared<ClassicalState>();

            // the zero state
            auto initial_state = make_shared<QuantumState>(get_qubits_used(embedding), this->precision);
            result.push_back(make_pair(make_shared<HybridState>(initial_state, classical_state), 1.0));

            return result;
        }

        MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
            MyFloat answer("0", this->precision*(this->max_horizon+1));

            auto local_target_state = this->get_target_state(embedding);
            for (auto it : belief.probs) {
                auto is_target = this->target_vertices.find(it.first->id);
                if (is_target != this->target_vertices.end()) {
                    if (is_target->second) {
                        answer = answer + it.second;
                    }
                } else {
                    if(*it.first->hybrid_state->quantum_state == *local_target_state) {
                        answer = answer + it.second;
                        this->target_vertices[it.first->id] =  true;
                    } else {
                        this->target_vertices[it.first->id] = false;
                    }
                }

            }
            return answer;
        }

        double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
            double answer = 0.0;

            auto local_target_state = this->get_target_state(embedding);
            for (auto it : belief.probs) {
                auto is_target = this->target_vertices.find(it.first->id);
                if (is_target != this->target_vertices.end()) {
                    if (is_target->second) {
                        answer = answer + it.second;
                    }
                } else {
                    if(*it.first->hybrid_state->quantum_state == *local_target_state) {
                        answer = answer + it.second;
                        this->target_vertices[it.first->id] =  true;
                    } else {
                        this->target_vertices[it.first->id] = false;
                    }
                }

            }
            return answer;
        }

        vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
            if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware) {
                unordered_map<int, int> embedding;
                embedding[0] = 0;
                embedding[1] = 1;
                embedding[2] = 2;
                return {embedding};
            }
            vector<unordered_map<int, int>> result;
            for (int qubit1 = 0; qubit1 < hardware_spec.num_qubits; qubit1++) {
                for (int qubit2 = 0; qubit2 < hardware_spec.num_qubits; qubit2++) {
                    for (int qubit3 = 0; qubit3 < hardware_spec.num_qubits; qubit3++) {
                        unordered_set<int >current_set({qubit1, qubit2, qubit3});
                        if (current_set.size() == 3) {
                            if (are_adjacent_qubits(hardware_spec.digraph, qubit1, {qubit1, qubit2, qubit3})) {
                                    unordered_map<int, int> d_temp;
                                    d_temp[0] = qubit1;
                                    d_temp[1] = qubit2;
                                    d_temp[2] = qubit3;
                                    if (!is_repeated_embedding(result, d_temp))
                                        result.push_back(d_temp);
                            }
                        }
                    }
                }
            }
            return result;
        }

        vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {
            vector<shared_ptr<POMDPAction>> result;
            for (auto it : embedding) {
                result.push_back(
                    make_shared<POMDPAction>("H" + to_string(it.first),
                        hardware_spec.to_basis_gates_impl(Instruction(GateName::H, it.second)),
                        this->precision,
                        vector<Instruction>({Instruction(GateName::H, it.first)})
                        )
                    );
            }

            for (auto it1 : embedding) {
                int v_control = it1.first;
                int control = it1.second;
                for (auto it2 : embedding) {
                    int v_target = it2.first;
                    int target = it2.second;
                    if (control != target) {
                        auto instruction = make_shared<Instruction>(GateName::Cnot, vector<int>({control}), target);
                        if (hardware_spec.get_hardware() == PerfectHardware || hardware_spec.instructions_to_channels.find(instruction) != hardware_spec.instructions_to_channels.end() ) {
                            result.push_back(
                                make_shared<POMDPAction>(
                                    "CX" + to_string(v_control)+"-"+to_string(v_target),
                                    vector<Instruction>({*instruction}),
                                    this->precision,
                                    vector<Instruction>({Instruction(GateName::Cnot, vector<int>({v_control}), v_target)})
                                    )
                                );
                        }
                    }
                }
            }
            return result;
        }

        string get_precondition(const MethodType &method) override {
            assert(method == MethodType::SingleDistBellman);
            string state000 = "[1,0,0,0,0,0,0,0]";
            return "P([q0,q1,q2]="+state000+" and [x0] = b0) = 1";
        }

        string get_target_postcondition(const double &threshold) override {
            string  ghzstate = "[0.70710678,0,0,0,0,0,0,0.70710678]";
            return "P( q0,q1,q2 ="+ ghzstate +") >= " + to_string(threshold);
        }
};

// GHZ state preparation of 4 qubits
class GHZStatePreparation4 : public GHZStatePreparation3 {
    public:
        GHZStatePreparation4(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon, const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) : GHZStatePreparation3(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize){};
        GHZStatePreparation4() : GHZStatePreparation3() {
            this->name = "ghz_state_preparation4";
            this->precision = 8;
            this->with_thermalization = false;
            this->min_horizon = 4;
            this->max_horizon = 4;
            this->set_hidden_index = false;
            this->method_types.erase(MethodType::Convex); // there is only one initial state, is not worth it
        };

        shared_ptr<QuantumState> get_target_state(const unordered_map<int, int> &embedding) const override{
            auto H0 = Instruction(GateName::H, embedding.at(0));
            auto CX01 = Instruction(GateName::Cnot, vector<int>({embedding.at(0)}), embedding.at(1));
            auto CX12 = Instruction(GateName::Cnot, vector<int>({embedding.at(1)}),embedding.at(2));
            auto CX23 = Instruction(GateName::Cnot, vector<int>({embedding.at(2)}),embedding.at(3));
            auto qs_ = QuantumState({embedding.at(0), embedding.at(1), embedding.at(2)}, this->precision);
            auto qs0 = qs_.apply_instruction(H0);
            auto qs1 = qs0->apply_instruction(CX01);
            auto qs2 = qs1->apply_instruction(CX12);
            auto qs = qs2->apply_instruction(CX23);
            return qs;
        }

        vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
            if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware) {
                unordered_map<int, int> embedding;
                embedding[0] = 0;
                embedding[1] = 1;
                embedding[2] = 2;
                embedding[3] = 3;
                return {embedding};
            }
            vector<unordered_map<int, int>> result;
            for (int qubit1 = 0; qubit1 < hardware_spec.num_qubits; qubit1++) {
                for (int qubit2 = 0; qubit2 < hardware_spec.num_qubits; qubit2++) {
                    for (int qubit3 = 0; qubit3 < hardware_spec.num_qubits; qubit3++) {
                        for (int qubit4 = 0; qubit4 < hardware_spec.num_qubits; qubit4++) {
                            unordered_set<int >current_set({qubit1, qubit2, qubit3, qubit4});
                            if (current_set.size() == 4) {
                                if (are_adjacent_qubits(hardware_spec.digraph, qubit1, {qubit1, qubit2, qubit3, qubit4})) {
                                        unordered_map<int, int> d_temp;
                                        d_temp[0] = qubit1;
                                        d_temp[1] = qubit2;
                                        d_temp[2] = qubit3;
                                        d_temp[3] = qubit4;
                                        if (!is_repeated_embedding(result, d_temp))
                                            result.push_back(d_temp);
                                }
                                cout << result.size() << endl;
                            }
                        }
                    }
                }
            }
            return result;
        }

    string get_precondition(const MethodType &method) override {
        throw runtime_error("Not implemented");
    }
    string get_target_postcondition(const double &threshold) override {
        throw runtime_error("Not implemented");
    }
};
#endif
