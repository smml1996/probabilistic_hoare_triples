
#include "pomdp.hpp"

#include <cassert>
#include <iostream>

#include "utils.hpp"
#include<queue>

#include "algorithm.hpp"

int POMDPVertex::local_counter = 1;

POMDPVertex::~POMDPVertex() = default;

POMDPVertex::POMDPVertex(const shared_ptr<HybridState> &hybrid_state, int hidden_index) {
    this->id = POMDPVertex::local_counter;
    POMDPVertex::local_counter += 1;
    this->hybrid_state = hybrid_state;
    this->hidden_index = hidden_index;
}

bool POMDPVertex::operator==(const POMDPVertex &other) const{
    assert(this->hybrid_state != nullptr);
    assert(other.hybrid_state != nullptr);
    return this->hidden_index == other.hidden_index && *this->hybrid_state == *other.hybrid_state;
}

shared_ptr<ClassicalState> POMDPVertex::get_obs() const {
    return this->hybrid_state->classical_state;
}

std::size_t POMDPVertexHash::operator()(const shared_ptr<POMDPVertex> &v) const {
    return v->id;
}


bool POMDPVertexPtrEqual::operator()(const shared_ptr<POMDPVertex> &a, const shared_ptr<POMDPVertex> &b) const {
    assert(a != nullptr);
    assert(b != nullptr);
    return *a == *b;
}

bool POMDPVertexPtrEqualID::operator()(const shared_ptr<POMDPVertex> &a, const shared_ptr<POMDPVertex> &b) const {
    return a->id == b->id;
}



void POMDPAction::_handle_measure_instruction(const Instruction &instruction, const MeasurementChannel &channel, const POMDPVertex &vertex, vertex_dict &result, bool is_meas1) const {
    /*
    applies a measurement instruction to a given hybrid state (POMDP vertex)

        Args:
            instruction (Instruction): a measurement instruction
            channel (MeasChannel): measurement channel for the instruction
            vertex (POMDPVertex): current vertex for which we want to know the successors when we apply the instruction and the channel
            is_meas1 (bool, optional): _description_. is this a measurement 1 or 0?
            result (Dict[POMDPVertex, float], optional): _description_. This is a dictionary that maps a pomdp vertex and the probability of reaching it from the current vertex. We accumulate the result in this dictionary, this is why it is a parameter.
    */

    Instruction instruction1;
    if (!is_meas1) {
        instruction1 = Instruction(GateName::P0, instruction.target);
    } else {
        instruction1 = Instruction(GateName::P1, instruction.target);
    }

    auto temp = get_sequence_probability(vertex.hybrid_state->quantum_state, {instruction1}, this->precision);

    auto q = temp.first;
    if (q == nullptr) {
        return;
    }
    auto meas_prob = temp.second;

    if (meas_prob > 0.0) {
        auto hidden_index = vertex.hidden_index;
        assert(instruction.c_target > -1);
        auto classical_state0 = vertex.hybrid_state->classical_state->apply_instruction(Instruction(GateName::Write0, instruction.c_target));

        auto classical_state1 = vertex.hybrid_state->classical_state->apply_instruction(Instruction(GateName::Write1, instruction.c_target));

        shared_ptr<POMDPVertex> new_vertex_correct;
        shared_ptr<POMDPVertex> new_vertex_incorrect;
        if (is_meas1) {
            new_vertex_correct = make_shared<POMDPVertex>(make_shared<HybridState>(q, classical_state1), hidden_index); // we receive the correct outcome
            new_vertex_incorrect = make_shared<POMDPVertex>(make_shared<HybridState>(q, classical_state0), hidden_index);
        } else {
            new_vertex_correct = make_shared<POMDPVertex>(make_shared<HybridState>(q, classical_state0), hidden_index); // we receive the correct outcome
            new_vertex_incorrect = make_shared<POMDPVertex>(make_shared<HybridState>(q, classical_state1), hidden_index);
        }
        auto prob = round_to(meas_prob * channel.get_ind_probability(is_meas1, is_meas1),  this->precision);
        if (prob > 0) {
            if (result.find(new_vertex_correct) != result.end()) {
                result[new_vertex_correct] = 0.0;
            }
            result[new_vertex_correct] += prob;
        }

        prob = round_to(meas_prob * channel.get_ind_probability( is_meas1, not is_meas1), this->precision);
        if (prob > 0){
            if (result.find(new_vertex_incorrect) != result.end()) {
                result[new_vertex_incorrect] = 0.0;
            }
            result[new_vertex_incorrect] += prob;
        }
        assert(is_close(channel.get_ind_probability(is_meas1, is_meas1) + channel.get_ind_probability(is_meas1, not is_meas1), 1, this->precision));
    }
}

void POMDPAction::_handle_unitary_instruction(const Instruction &instruction, const QuantumChannel &channel, const POMDPVertex &vertex, vertex_dict &result) const {
    for (const auto & errors_to_prob : channel.errors_to_probs) {
        auto err_seq = errors_to_prob.first;
        auto prob = errors_to_prob.second;

        assert (!err_seq.empty());
        auto new_qs = vertex.hybrid_state->quantum_state->apply_instruction(instruction);
        auto temp = get_sequence_probability(new_qs, err_seq, this->precision);
        auto errored_seq = temp.first;
        auto seq_prob = temp.second;
        if (seq_prob > 0.0) {
            auto new_vertex = make_shared<POMDPVertex>(make_shared<HybridState>(errored_seq, vertex.hybrid_state->classical_state), vertex.hidden_index);
            if (result.find(new_vertex) != result.end()) {
                result[new_vertex] = 0.0;
            }
            result[new_vertex] += round_to(seq_prob * prob, this->precision);
        }
    }
}

void POMDPAction::_handle_reset_instruction(const Instruction &instruction, const QuantumChannel &channel, const POMDPVertex &vertex, vertex_dict &result, bool is_meas1) const {
    assert (instruction.gate_name == GateName::Reset);
    Instruction projector;

    if (is_meas1) {
        projector = Instruction(GateName::P1, instruction.target);
    } else {
        projector = Instruction(GateName::P0, instruction.target);
    }

    for (const auto & errors_to_prob : channel.errors_to_probs) {
        auto err_seq = errors_to_prob.first;
        auto prob = errors_to_prob.second;
        auto temp = get_sequence_probability(vertex.hybrid_state->quantum_state, {projector}, this->precision);
        auto new_qs = temp.first;
        auto prob_new_qs = temp.second;
        if (prob_new_qs > 0) {
            if (is_meas1){
                auto x_instruction = Instruction(GateName::X, instruction.target);
                new_qs = new_qs->apply_instruction(x_instruction);
            }
            auto temp2 = get_sequence_probability(new_qs, err_seq, this->precision);
            auto errored_seq = temp2.first;
            auto seq_prob = temp2.second;
            seq_prob = round_to(prob_new_qs * seq_prob, this->precision);
            if (seq_prob > 0.0) {
                auto new_vertex = make_shared<POMDPVertex>(make_shared<HybridState>(errored_seq, vertex.hybrid_state->classical_state), vertex.hidden_index);
                if (result.find(new_vertex) != result.end()) {
                    result[new_vertex] = 0.0;
                }
                result[new_vertex] += round_to(seq_prob * prob, this->precision);
            }
        }
    }
}

vertex_dict POMDPAction::_dfs(HardwareSpecification &hardware_specification, const shared_ptr<POMDPVertex>& current_vertex, int index_ins) const {
    /* perform a dfs to compute successors states of the sequence of instructions.
        It applies the instruction at index self.instructions_seq[index_ins] along with errors recursively

        Args:
            noise_model (NoiseModel): hardware noise model
            current_vertex (POMDPVertex):
            index_ins (int): should be less than or equal (base case that returns empty dictionary)

        Returns:
            Dict[POMDPVertex, float]: returns a dictionary where the key is a successors POMDPVertex and the corresponding probability of reaching it from current_vertex
    */
    if (index_ins == this->instruction_sequence.size()) {
        vertex_dict result;
        result[current_vertex] = 1.0;
        return result;
    }

    assert(index_ins < this->instruction_sequence.size());

    auto current_instruction = this->instruction_sequence[index_ins];
    vertex_dict temp_result;
    if (current_instruction.instruction_type == InstructionType::Classical) {
        auto new_classical_state = current_vertex->hybrid_state->classical_state->apply_instruction(current_instruction);
        auto new_vertex = make_shared<POMDPVertex>(make_shared<HybridState>(current_vertex->hybrid_state->quantum_state, new_classical_state), current_vertex->hidden_index);
        temp_result[new_vertex] = 1.0;
    } else {
        auto instruction_channel = hardware_specification.get_channel(make_shared<Instruction>(current_instruction));
        if (current_instruction.instruction_type == InstructionType::Measurement) {
            // get successors for 0-measurements
            this->_handle_measure_instruction(current_instruction, *static_pointer_cast<MeasurementChannel>(instruction_channel), *current_vertex, temp_result, false );

            // get successors for 1-measurements
            this->_handle_measure_instruction(current_instruction, *static_pointer_cast<MeasurementChannel>(instruction_channel), *current_vertex, temp_result, true);
        } else if (current_instruction.gate_name == GateName::Reset){
            // WARNING: use of reset not known in all models, check when using real hardware specifications
            this->_handle_reset_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), *current_vertex, temp_result, false);

            this->_handle_reset_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), *current_vertex, temp_result, true);
        } else {
            this->_handle_unitary_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), *current_vertex, temp_result);
        }
    }
    vertex_dict result;
    for (const auto& it : temp_result) {
        auto successor = it.first;
        assert (successor != nullptr);
        auto prob = it.second;
        auto successors2 = this->_dfs(hardware_specification, successor, index_ins+1);
        for (const auto& it2 : successors2) {
            auto succ2 = it2.first;
            auto prob2 = it2.second;
            if ( result.find(succ2) != result.end()) result.at(succ2) = 0.0;
            result[succ2] += prob*prob2;
        }
    }
    for (const auto& it : result) {
        auto s = it.first;
        auto prob = it.second;
        result[s] = round_to(prob, this->precision);
    }
    assert (!result.empty());
    normalize(result);
    return result;
}

void normalize(vertex_dict &v) {
    int total_entries = v.size();
    double total_sum = 0.0;
    for (const auto& it : v) {
        total_sum += it.second;
    }

    for (auto &it : v) {
        it.second /= total_sum;
    }

    int current_index = 1;
    double total_prob = 0.0;
    for (auto &it : v) {
        if (current_index == total_entries) {
            it.second = 1.0 - total_prob;
        }
        total_prob += it.second;
        current_index++;
    }

    unordered_set<shared_ptr<POMDPVertex>, POMDPVertexHash, POMDPVertexPtrEqual> to_remove;
    for (const auto& it : v) {
        if (it.second == 0) {
            to_remove.insert(it.first);
        }
    }
    for (const auto& it : to_remove) {
        v.erase(it);
    }
}


POMDPAction::POMDPAction(const string &name, const vector<Instruction> &instruction_sequence, int precision, const vector<Instruction> &pseudo_instruction_sequence) {
    assert (!name.empty());
    this->name = name;
    this->instruction_sequence = instruction_sequence;
    this->precision = precision;
    this->pseudo_instruction_sequence = pseudo_instruction_sequence;
}

POMDPAction::POMDPAction(json &data) {
    this->name = data["name"].get<string>();
    this->precision = -1;
    for (const auto& j_ins : data["seq"])
        this->pseudo_instruction_sequence.emplace_back(j_ins, -1);
}

vertex_dict POMDPAction::get_successor_states(HardwareSpecification &hardware_specification, const shared_ptr<POMDPVertex> &current_vertex) const {
    return this->_dfs(hardware_specification, current_vertex, 0);
}

bool POMDPAction::operator==(const POMDPAction &other) const {
    return this->name == other.name;
}

shared_ptr<POMDPAction> POMDPAction::at_embedding(const unordered_map<int, int> &embedding) const {
    shared_ptr<POMDPAction> result = make_shared<POMDPAction>(this->name, vector<Instruction>({}), this->precision, this->pseudo_instruction_sequence);

    for (auto ins : this->instruction_sequence) {
        result->instruction_sequence.push_back(ins.rename(embedding));
    }

    return result;
}

string to_string(const POMDPAction &action) {
    assert(!action.name.empty());
    string result;
    if (action.name == HALT_ACTION.name) {
        return "HALT";
    }

    for (const auto& instruction : action.instruction_sequence) {
        result += to_string(instruction);
    }
    return result;
}

string v_to_string(const POMDPAction &action) {
    assert(!action.name.empty());
    string result;
    if (action.name == HALT_ACTION.name) {
        return "HALT";
    }

    for (const auto& instruction : action.pseudo_instruction_sequence) {
        result += to_string(instruction);
    }
    return result;
}

string to_string(const shared_ptr<POMDPAction> &action) {
    return to_string(*action);
}

string v_to_string(const shared_ptr<POMDPAction> &action) {
    assert(action != nullptr);
    if (action->name == HALT_ACTION.name) {
        return "skip";
    }
    return to_string(*action);
}

std::size_t POMDPActionHash::operator()(const shared_ptr<POMDPAction> &action) const {
    std::hash<std::string> str_hasher;
    return str_hasher(action->name);
}


bool POMDPActionPtrEqual::operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const {
    return *a == *b;
}

shared_ptr<POMDPVertex> POMDP::get_vertex(const shared_ptr<HybridState> &new_hs, const int &hidden_index) const
{
    for (const auto & state : this->states) {
        if (state->hidden_index == hidden_index) {
            if (*new_hs == *state->hybrid_state) {
                return state;
            }
        }
    }
    return nullptr;
}

shared_ptr<POMDPVertex> POMDP::create_new_vertex(const shared_ptr<HybridState> &hybrid_state, int hidden_index) {
    auto v = this->get_vertex(hybrid_state, hidden_index);
    if (v == nullptr) {
        auto new_vertex = make_shared<POMDPVertex>(hybrid_state, hidden_index);
        this->states.push_back(new_vertex);
        return new_vertex;
    }
    auto copy_v = make_shared<POMDPVertex>(v->hybrid_state, hidden_index);
    copy_v->id = v->id;
    return copy_v;
}

POMDP::~POMDP() = default;

POMDP::POMDP(int precision) {
    this->initial_state = nullptr;
    this->precision = precision;
}

POMDP::POMDP(const shared_ptr<POMDPVertex> &initial_state, const vector<shared_ptr<POMDPVertex>> &states, const vector<shared_ptr<POMDPAction>> &actions, const unordered_map<shared_ptr<POMDPVertex>, unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, double,POMDPVertexHash, POMDPVertexPtrEqualID>,POMDPActionHash, POMDPActionPtrEqual>, POMDPVertexHash, POMDPVertexPtrEqualID>  &transition_matrix_) {
    this->initial_state = initial_state;
    this->states = states;
    this->actions = actions;
    this->transition_matrix_ = transition_matrix_;
    this->precision = -1;
}

void POMDP::build_pomdp(const vector<shared_ptr<POMDPAction>> &actions_,
    HardwareSpecification &hardware_specification,
    int horizon,
    unordered_map<int, int> embedding,
    shared_ptr<HybridState> initial_state_hs,
    const vector<pair<shared_ptr<HybridState>, double>> &initial_distribution,
    vector<int> &qubits_used, const guard_type& guard, bool set_hidden_index) {

    this->actions = actions_;
    assert(this->states.empty());

    queue<pair<shared_ptr<POMDPVertex>, int>> q;
    if (initial_state_hs == nullptr) {
        initial_state_hs = make_shared<HybridState>(make_shared<QuantumState>(qubits_used, this->precision), make_shared<ClassicalState>());
    }

    auto initial_v = this->create_new_vertex(initial_state_hs, -1);
    this->initial_state = initial_v;

    if (initial_distribution.empty()) {
        assert(false);
        q.emplace(initial_v, 0);
    } else {
        double total = 0.0;

        for (auto& p : initial_distribution) {
            total += p.second;
        }
        if (!is_close(total, 1.0, this->precision)){
            throw invalid_argument("Initial distribution must sum to 1. Instead it is " + to_string(total) + "\n");
        }

        this->transition_matrix_[initial_v] = unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, double, POMDPVertexHash, POMDPVertexPtrEqualID>, POMDPActionHash, POMDPActionPtrEqual>();
        assert(this->transition_matrix_.find(initial_v) != this->transition_matrix_.end());
        auto INIT_CHANNEL = make_shared<POMDPAction>("INIT__", vector<Instruction>(), this->precision, vector<Instruction>());
        this->transition_matrix_[initial_v][INIT_CHANNEL] = unordered_map<shared_ptr<POMDPVertex>, double, POMDPVertexHash, POMDPVertexPtrEqualID>();
        assert(this->transition_matrix_.at(initial_v).find(INIT_CHANNEL) != this->transition_matrix_.at(initial_v).end());
        int hidden_index;
        for (int index = 0; index < initial_distribution.size(); index ++) {
            auto hybrid_state = initial_distribution[index].first;
            auto prob = initial_distribution[index].second;
            if (set_hidden_index){
                hidden_index = index;
            } else{
                hidden_index = -1;
            }
            auto v = this->create_new_vertex(hybrid_state, hidden_index); //new POMDPVertex(new HybridState(*hybrid_state), hidden_index);
            this->transition_matrix_[initial_v][INIT_CHANNEL].insert_or_assign(v, prob);
            this->transition_matrix[initial_v][INIT_CHANNEL].insert_or_assign(v, MyFloat(to_string(prob), this->precision * (horizon+1)));
            assert (this->transition_matrix_[initial_v][INIT_CHANNEL].find(v) != this->transition_matrix_[initial_v][INIT_CHANNEL].end());
            assert (v->id > 0);
            q.emplace(v, 0); // second element denotes that this vertex is at horizon 0
        }
    }

    unordered_set<shared_ptr<POMDPVertex>, POMDPVertexHash, POMDPVertexPtrEqualID> visited;

    while (!q.empty()) {
        pair<shared_ptr<POMDPVertex>, int> temp = q.front();
        q.pop();
        auto current_v = temp.first;
        auto current_horizon = temp.second;
        if (horizon != -1) {
            if (current_horizon == horizon) {
                continue;
            }
        }

        if (visited.find(current_v) != visited.end()) {
            continue;
        }

        visited.insert(current_v);

        if (this->transition_matrix_.find(current_v) == this->transition_matrix_.end()) {
            this->transition_matrix_[current_v] = unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, double, POMDPVertexHash, POMDPVertexPtrEqualID>, POMDPActionHash, POMDPActionPtrEqual>();
            this->transition_matrix[current_v] = unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>, MyFloat, POMDPVertexHash, POMDPVertexPtrEqualID>, POMDPActionHash, POMDPActionPtrEqual>();
        }
        for (auto action : actions) {
            if (guard(current_v, embedding, action)) {
                assert(this->transition_matrix_[current_v].find(action) == this->transition_matrix_[current_v].end());


                this->transition_matrix_[current_v][action] = unordered_map<shared_ptr<POMDPVertex>, double, POMDPVertexHash, POMDPVertexPtrEqualID>();
                this->transition_matrix[current_v][action] = unordered_map<shared_ptr<POMDPVertex>, MyFloat, POMDPVertexHash, POMDPVertexPtrEqualID>();
                auto successors = action->get_successor_states(hardware_specification, current_v);
                assert(!successors.empty());
                for (const auto& it : successors ) {
                    auto succ = it.first;
                    auto prob = it.second;
                    assert(succ->hybrid_state != nullptr);
                    assert(succ->hybrid_state->quantum_state != nullptr);
                    assert(succ->hybrid_state->classical_state != nullptr);
                    auto new_vertex = this->create_new_vertex(succ->hybrid_state, succ->hidden_index);
                    assert(new_vertex != nullptr);
                    assert(new_vertex->hybrid_state != nullptr);
                    assert(new_vertex->hybrid_state != nullptr);
                    if (this->transition_matrix_[current_v][action].find(new_vertex) == this->transition_matrix_[current_v][action].end()) {
                        this->transition_matrix_[current_v][action].insert_or_assign(new_vertex, 0);
                    }

                    this->transition_matrix_[current_v][action][new_vertex] = this->transition_matrix_[current_v][action][new_vertex] + prob;
                    if (visited.find(new_vertex) == visited.end()) {
                        if (horizon == -1 || (current_horizon + 1 < horizon)) {
                            q.emplace(make_shared<POMDPVertex>(*new_vertex), current_horizon);
                        }
                    }
                }
            }
        }
    }

    for (const auto& it : this->transition_matrix_) {
        for (const auto& it_action : it.second) {
            for (const auto& it_successor: it_action.second) {
                this->transition_matrix[it.first][it_action.first][it_successor.first] = MyFloat(to_string(round_to(it_successor.second, 8)), this->precision * (horizon+1));
            }
        }

    }
}

ostream& operator<<(ostream& os, const POMDPVertex& v) {
    os << v.id << " " << v.hidden_index << " -- ";
    os << *v.hybrid_state;

    return os;
}

void POMDP::print_pomdp() const {
    cout << "states: " << endl;

    for (const auto& s : states) {
        cout <<  *s << endl;
    }
    cout << "transitions: " << endl;
    for (const auto& it : this->transition_matrix_) {
        for (const auto& it_action : it.second) {
            for (const auto& it_successor: it_action.second) {
                cout << it.first->id << " ----- " << it_action.first->name << " " << round_to(it_successor.second, this->precision) << " " << it_successor.first->id << endl;
            }
        }

    }
}

void POMDP::check_pomdp() const {
    for (const auto& it : this->transition_matrix) {
        assert(it.first->hybrid_state != nullptr);
        assert(it.first->hybrid_state->quantum_state != nullptr);
        assert(it.first->hybrid_state->classical_state != nullptr);
        for (const auto& it_action : it.second) {
            assert(it_action.first != nullptr);
            assert (!it_action.first->name.empty());
            for (const auto& it_successor: it_action.second) {
                assert(it_successor.first->hybrid_state != nullptr);
                assert(it_successor.first->hybrid_state->quantum_state != nullptr);
                assert(it_successor.first->hybrid_state->classical_state != nullptr);
            }
        }
    }
}
