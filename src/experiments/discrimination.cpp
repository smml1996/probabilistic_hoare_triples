#include "experiments.hpp"
#include "reset.cpp"

//
// Created by Stefanie Muroya Lei on 06.09.25.
//
class BasisStatesDiscrimination : public ResetProblem {
public:
    BasisStatesDiscrimination(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) : ResetProblem(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize) {
        this->set_hidden_index = true;
        this->nqvars = 2;
        this->ncvars = 1;
    };

    BasisStatesDiscrimination() {
        this->set_hidden_index = true;
        this->name = "basic_zero_plus";
        this->precision = 8;
        this->with_thermalization = false;
        this->min_horizon = 1;
        this->max_horizon = 3;
        this->nqvars = 2;
        this->ncvars = 1;
    };

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        MyFloat result("0", this->precision*(this->max_horizon+1));
        for (auto it : belief.probs) {
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            assert (it.first->hidden_index == 0 || it.first->hidden_index == 1);
            if (it.first->hidden_index == cs->read(0)) {
                result = result + it.second;
            }
        }
        return result;
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        double result = 0;
        for (auto it : belief.probs) {
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            assert (it.first->hidden_index == 0 || it.first->hidden_index == 1);
            if (it.first->hidden_index == cs->read(0)) {
                result = result + it.second;
            }
        }
        return result;
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {
        assert(embedding.size() == 1);
        assert(embedding.find(0) != embedding.end());


        vector<Instruction> H_seq = hardware_spec.to_basis_gates_impl(Instruction(GateName::H,
            embedding.at(0)));
        // H_seq.emplace_back(GateName::Write1, 2);
        auto H0 = make_shared<POMDPAction>("H0", H_seq, this->precision, vector<Instruction>({Instruction(GateName::H, 0)}));

        auto P0 = make_shared<POMDPAction>("P0",
            vector<Instruction>({Instruction(GateName::Meas, embedding.at(0), 0),
                // Instruction(GateName::Write1, 3)
                }),
            this->precision,
            vector<Instruction>({Instruction(GateName::Meas, 0, 0)}));

        auto determine0 = make_shared<POMDPAction>("Is0",
            vector<Instruction>({Instruction(GateName::Write0, 0), Instruction(GateName::Write1, 1)}),
            this->precision,
            vector<Instruction>({Instruction(GateName::Write0, 0)}));

        auto determinePlus = make_shared<POMDPAction>("IsPlus",
            vector<Instruction>({Instruction(GateName::Write1, 0), Instruction(GateName::Write1, 1)}),
            this->precision,
            vector<Instruction>({Instruction(GateName::Write1, 0)}));
        return {H0, P0, determine0, determinePlus};
    }
};

class CBasisStatesDiscrimination : public BasisStatesDiscrimination {
public:
    CBasisStatesDiscrimination(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) : BasisStatesDiscrimination(name, precision, with_thermalization, min_horizon, max_horizon, method_types, hw_list, optimize) {
        this->set_hidden_index = true;
    };

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        MyFloat result("0", this->precision*(this->max_horizon+1));
        for (auto it : belief.probs) {
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            assert (it.first->hidden_index == 0 || it.first->hidden_index == 1);
            if (it.first->hidden_index == cs->read(0)) {
                result = result + it.second;
            }
        }
        return result;
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        double result = 0.0;
        for (auto it : belief.probs) {
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            assert (it.first->hidden_index == 0 || it.first->hidden_index == 1);
            if (it.first->hidden_index == cs->read(0)) {
                result = result + it.second;
            }
        }
        return result;
    }

    vector<shared_ptr<POMDPAction>> get_actions(HardwareSpecification &hardware_spec, const unordered_map<int, int> &embedding) const override {

        assert(embedding.size() == 1);
        assert(embedding.find(0) != embedding.end());


        auto X0 = make_shared<POMDPAction>("X0", hardware_spec.to_basis_gates_impl(Instruction(GateName::X,
            embedding.at(0))), this->precision, vector<Instruction>({Instruction(GateName::X, 0)}));

        auto P0 = make_shared<POMDPAction>("P0",
            vector<Instruction>({Instruction(GateName::Meas, embedding.at(0), 0)}),
            this->precision,
            vector<Instruction>({Instruction(GateName::Meas, 0, 0)}));

        auto determine0 = make_shared<POMDPAction>("Is0",
            vector<Instruction>({Instruction(GateName::Write0, 0)}),
            this->precision,
            vector<Instruction>({Instruction(GateName::Write0, 0)}));

        auto determine1 = make_shared<POMDPAction>("Is1",
            vector<Instruction>({Instruction(GateName::Write1, 0)}),
            this->precision,
            vector<Instruction>({Instruction(GateName::Write1, 0)}));
        return {X0, P0, determine0, determine1};
    }

};

class BasicZeroPlusDiscrimination : public BasisStatesDiscrimination {
public:
    BasicZeroPlusDiscrimination(const string &name, int precision, bool with_thermalization, int min_horizon, int max_horizon,
        const set<MethodType> &method_types, const set<QuantumHardware>& hw_list, bool optimize) :
    BasisStatesDiscrimination(name, precision, with_thermalization, min_horizon, max_horizon,
    method_types, hw_list, optimize) {
    };

    BasicZeroPlusDiscrimination() : BasisStatesDiscrimination() {};

    vector<pair<shared_ptr<HybridState>, double>> get_initial_distribution(unordered_map<int, int> &embedding) const override {
        assert (embedding.size() == 1);
        vector<pair<shared_ptr<HybridState>, double>> result;

        auto classical_state = make_shared<ClassicalState>();

        auto H0 = Instruction(GateName::H, embedding.at(0));

        auto state0 = make_shared<QuantumState>(vector<int>({embedding.at(0)}), this->precision);
        result.push_back(make_pair(make_shared<HybridState>(state0, classical_state), 0.5));

        // prepare first bell state
        auto state1 = state0->apply_instruction(H0);

        result.push_back(make_pair(make_shared<HybridState>(state1, classical_state), 0.5));

        return result;
    }

    [[nodiscard]] bool guard(const shared_ptr<POMDPVertex>& vertex, const unordered_map<int, int>& embedding, const shared_ptr<POMDPAction>& action) const override {
        return true;
    }

    MyFloat postcondition(const Belief &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        auto state0 = QuantumState({embedding.at(0)}, this->precision);
        auto H = Instruction(GateName::H, embedding.at(0));
        auto state1 = state0.apply_instruction(H);
        MyFloat result("0", this->precision*(this->max_horizon+1));
        for (auto it : belief.probs) {
            assert(it.first->hidden_index == 0 || it.first->hidden_index == 1);
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            if (cs->read(0) == it.first->hidden_index) {
                result = result + it.second;
            }
        }
        return result;
    }

    double postcondition_double(const VertexDict &belief, const unordered_map<int, int> &embedding) override {
        assert (embedding.size() == 1);
        auto state0 = QuantumState({embedding.at(0)}, this->precision);
        auto H = Instruction(GateName::H, embedding.at(0));
        auto state1 = state0.apply_instruction(H);
        double result = 0.0;
        for (auto it : belief.probs) {
            assert(it.first->hidden_index == 0 || it.first->hidden_index == 1);
            auto hybrid_state = it.first->hybrid_state;
            auto cs = hybrid_state->classical_state;
            if (cs->read(0) == it.first->hidden_index) {
                result = result + it.second;
            }
        }
        return result;
    }

    shared_ptr<Algorithm> get_textbook_algorithm(MethodType &method, const int &horizon) override {
        auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, false, false);
        auto action_mappings = this->get_actions_dictionary(hardware_spec, 1);
        if (horizon == 1 || horizon == 2) {
            auto new_head = make_shared<Algorithm>(make_shared<POMDPAction>(random_branch), 0, 5, -1); // we are not going to use precision
            new_head->children.push_back(make_shared<Algorithm>(action_mappings["P0"], 0, 10, -1));
            new_head->children.push_back(normalize_algorithm(make_shared<Algorithm>(action_mappings["IsPlus"], 0, 10, -1)));
            new_head->children_probs.insert({0, 2.0/3.0});
            new_head->children_probs.insert({1, 1.0/3.0});
            auto meas_node = new_head->children.at(0);
            new_head->children[0] = normalize_algorithm(meas_node);
            return new_head;
        }
        auto new_head = make_shared<Algorithm>(make_shared<POMDPAction>(random_branch), 0, 5, -1); // we are not going to use precision
        new_head->children_probs.insert({0, 0.5});
        new_head->children_probs.insert({1, 0.5});

        // simple measurements
        shared_ptr<Algorithm> on1 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 1, 10, 1);
        shared_ptr<Algorithm> on0 = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 10, 1);
        auto simple_meas_sequence = this->build_meas_sequence(horizon, 0, action_mappings["P0"], make_shared<ClassicalState>(), on0, on1);
        new_head->children.push_back(normalize_algorithm(simple_meas_sequence));

        // -------------
        // apply h0 and then measurement sequences
        auto h0_node = make_shared<Algorithm>(action_mappings["H0"], 1, 10, -1);
        shared_ptr<Algorithm> onH1 = make_shared<Algorithm>(action_mappings["Is0"], 1, 10, -1);
        shared_ptr<Algorithm> onH0 = make_shared<Algorithm>(action_mappings["IsPlus"], 0, 10, -1);
        auto other_meas_node = this->build_meas_sequence(horizon-2, 0, action_mappings["P0"], make_shared<ClassicalState>(), onH0, onH1);
        h0_node->children.push_back(other_meas_node);

        new_head->children.push_back(normalize_algorithm(h0_node));

        return new_head;
    }

    string get_precondition(const MethodType &method) override {
        assert (method == MethodType::ConvexDist);
        string state00 = "[1,0,0,0]";
        string statePP = "[0.5,0.5,0.5,0.5]"; // |++>

        return string("P([q0,q1] = "+ state00 +" and [x0] = b0 ) = 1 + ") +
            "P([q0,q1] = "+ statePP + " and [x0] = b0) = 1";
    }

    string get_target_postcondition(const double &threshold) override {
        return "P( q1 = [1,0] and x0 = b0 or q1 = [0.70710, 0.70710] and x0 = b1 ) >= " + to_string(threshold);
    }
};
