#ifndef GHZ_H
#define GHZ_H
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
        
    for (const auto& embedding : all_embeddings) {
        unordered_set<int> temp_s;
        for (const auto it : embedding)
            temp_s.insert(it.second);
        if (temp_s == current_set)
            return true;
    }
    return false;
}

// GHZ state preparation of 3 qubits
class GHZStatePrep : public Experiment {
    protected:
    void set_global_equality() override {
        QuantumState::use_global_eq = false;
    }

    void set_min_max_horizon(const MethodType& method) override {
        this->min_horizon = 3;
        this->max_horizon = 3;
    }

    void set_methods() override {
        this->method_types = {MethodType::SingleDistBellman};
    }

    void set_num_vars() override {
        this->nqvars = 3;
        this->ncvars = 1;
    }

    public:
    GHZStatePrep(const string &name, const set<QuantumHardware>& hw_list) : Experiment(name, hw_list){};

    [[nodiscard]] set<QuantumHardware> get_allowed_hardware() const override{
        set<QuantumHardware> result;
        for (int i = 0; i < QuantumHardware::HardwareCount; i++) { auto quantum_hardware = static_cast<QuantumHardware>(i);
            HardwareSpecification hs(quantum_hardware, false, false);
            if (hs.basis_gates_type != BasisGates::TYPE5 && hs.basis_gates_type != BasisGates::TYPE2) {
                result.insert(quantum_hardware);
            }
        }
        return result;
    }

        [[nodiscard]] virtual shared_ptr<QuantumState> get_target_state(const unordered_map<int, int> &embedding) const {
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
            result.emplace_back(make_shared<HybridState>(initial_state, classical_state), 1.0);

            return result;
        }

        MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
            MyFloat answer("0", this->precision*(this->max_horizon+1));

            auto local_target_state = this->get_target_state(embedding);
            for (const auto& it : belief.probs) {
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
            for (const auto& it : belief.probs) {
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

        [[nodiscard]] vector<unordered_map<int, int>> get_hardware_scenarios(HardwareSpecification const & hardware_spec) const override {
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
            result.reserve(embedding.size());
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

#endif
