
#include "pomdp.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <queue>

#include "channels.hpp"
#include "utils.hpp"

int POMDPVertex::local_counter = 0;
int POMDPAction::local_counter = 0;

POMDPVertex::POMDPVertex() {
this->id = POMDPVertex::local_counter;
    POMDPVertex::local_counter += 1;
}

POMDPVertex::POMDPVertex(const int &id) {
    this->id = id;
}

bool POMDPVertex::operator==(const POMDPVertex &other) const{
    return this->id == other.id;
}

shared_ptr<POMDPVertex> POMDPVertex::get(const int &id) {
        return make_shared<POMDPVertex>(id);
}

QVertex::QVertex(const shared_ptr<HybridState> &hybrid_state) : POMDPVertex() {
    this->hybrid_state = hybrid_state;
}

bool QVertex::operator==(const QVertex &other) const {
    return this->hybrid_state == other.hybrid_state;
}

int QVertex::get_obs() const {
    return this->hybrid_state->classical_state->get_memory_val();
}

shared_ptr<QuantumState> QVertex::quantum_state() const {
    return this->hybrid_state->quantum_state;
}

shared_ptr<ClassicalState> QVertex::classical_state() const {
    return this->hybrid_state->classical_state;
}

int QVertex::hidden_index() const {
    return this->hybrid_state->hidden_index;
}

std::size_t POMDPVertexHash::operator()(const shared_ptr<POMDPVertex> &v) const {
    return v->id;
}


bool POMDPVertexPtrEqual::operator()(const shared_ptr<POMDPVertex> &a, const shared_ptr<POMDPVertex> &b) const {
    assert(a != nullptr);
    assert(b != nullptr);
    return *a == *b;
}

POMDPAction::POMDPAction(const string &name) {
    this->id = this->local_counter;
    this->local_counter++;
    this->name = name;
}

POMDPAction::POMDPAction(const int &id, const string &name) {
    this->id = id;
    this->name = name;
}

bool POMDPAction::operator==(const POMDPAction &other) const {
    return this->id == other.id;
}

QAction::QAction(HardwareSpecification &hw_spec, const vector<Instruction> &pseudo_instruction_seq, bool with_noise_) : POMDPAction() {
    this->with_noise = with_noise_;
    vector<string> inst_names;

    for (auto instruction : pseudo_instruction_seq) {
        inst_names.push_back(to_string(instruction));
        if (this->with_noise) {
            auto temp_seq = hw_spec.to_basis_gates_impl(instruction);

            for (auto real_ins : temp_seq) {
                this->instruction_sequence.push_back(real_ins);
            }
        } else {
            this->instruction_sequence.push_back(instruction);
        }
    }
    this->name = join(inst_names, "; ");
    this->name += ";";
}

void QAction::__handle_measure_instruction(const Instruction &instruction, const MeasurementChannel &channel,
    const shared_ptr<HybridState> &vertex, QEnsemble &result, bool is_meas1) const {
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

    auto curr_meas_outcome_data = get_sequence_probability(vertex->quantum_state, {instruction1});

    auto succ_qs = curr_meas_outcome_data.first;
    if (succ_qs == nullptr) {
        return;
    }
    auto meas_out_prob = curr_meas_outcome_data.second;

    if (meas_out_prob > zero) {
        assert(instruction.c_target > -1);
        auto classical_state0 = vertex->classical_state->apply_instruction(Instruction(GateName::Write0, instruction.c_target));

        auto classical_state1 = vertex->classical_state->apply_instruction(Instruction(GateName::Write1, instruction.c_target));

        shared_ptr<HybridState> new_vertex_correct; // the outcome stored in the classical state matches the meas. outcome
        shared_ptr<HybridState> new_vertex_incorrect; // outcome stored in the classical state doesnt match the meas. outcome
        if (is_meas1) {
            new_vertex_correct = make_shared<HybridState>(succ_qs, classical_state1, vertex->hidden_index); // we receive the correct outcome
            new_vertex_incorrect = make_shared<HybridState>(succ_qs, classical_state0, vertex->hidden_index);
        } else {
            new_vertex_correct = make_shared<HybridState>(succ_qs, classical_state0, vertex->hidden_index); // we receive the correct outcome
            new_vertex_incorrect = make_shared<HybridState>(succ_qs, classical_state1, vertex->hidden_index);
        }

        // probability of receiving the outcome that matches the measurement
        MyFloat prob = meas_out_prob;
        if (this->with_noise) {
            prob *= channel.get_ind_probability(is_meas1, is_meas1);
        }

        if (prob > 0) {
            result.add(new_vertex_correct, prob);
        }

        // probability of receiving the wrong outcome
        if (this->with_noise) {
            prob = meas_out_prob * channel.get_ind_probability( is_meas1, not is_meas1);
            if (prob > zero){
                result.add(new_vertex_incorrect, prob);
            }
        }
    }
}

void QAction::__handle_unitary_instruction(const Instruction &instruction, const QuantumChannel &channel,
    const shared_ptr<HybridState> &vertex, QEnsemble &result) const {

    for (int index = 0; index < channel.errors_to_probs.size(); index++) {
        auto err_seq = channel.errors_to_probs[index].first;
        auto prob = channel.errors_to_probs[index].second;

        assert (!err_seq.empty());

        auto new_qs = vertex->quantum_state->apply_instruction(instruction);

        if (this->with_noise) {
            auto temp = get_sequence_probability(new_qs, err_seq);
            auto errored_seq = temp.first;
            auto seq_prob = temp.second;
            if (seq_prob > 0.0) {
                auto new_vertex = make_shared<HybridState>(errored_seq, vertex->classical_state, vertex->hidden_index);
                result.add(new_vertex, seq_prob * prob);
            }
        } else {
            auto new_vertex = make_shared<HybridState>(new_qs, vertex->classical_state, vertex->hidden_index);
            result.add(new_vertex, MyFloat(1));
        }
    }
}

void QAction::__handle_reset_instruction(const Instruction &instruction, const QuantumChannel &channel,
                                         const shared_ptr<HybridState> &vertex, QEnsemble &result, bool is_meas1) const {
    assert (instruction.gate_name == GateName::Reset);
    Instruction projector;

    if (is_meas1) {
        projector = Instruction(GateName::P1, instruction.target);
    } else {
        projector = Instruction(GateName::P0, instruction.target);
    }

    auto succ_data = get_sequence_probability(vertex->quantum_state, {projector});
    auto proj_qs = succ_data.first;
    auto proj_prob = succ_data.second;

    if (proj_prob == zero) {
        return;
    }

    if (is_meas1){
        auto x_instruction = Instruction(GateName::X, instruction.target);
        proj_qs = proj_qs->apply_instruction(x_instruction);
    }

    if (!this->with_noise) {
        auto new_vertex = make_shared<HybridState>(proj_qs, vertex->classical_state, vertex->hidden_index);
        result.add(new_vertex, proj_prob);
    }

    for (int index = 0; index < channel.errors_to_probs.size(); index++) {
        auto err_seq = channel.errors_to_probs[index].first;
        auto prob = channel.errors_to_probs[index].second;
        if (proj_prob > 0) {
            auto new_qs = proj_qs;
            auto temp2 = get_sequence_probability(new_qs, err_seq);
            auto errored_seq = temp2.first;
            auto seq_prob = temp2.second;
            seq_prob = proj_prob * seq_prob;
            if (seq_prob > 0.0) {
                auto new_vertex = make_shared<HybridState>(errored_seq, vertex->classical_state, vertex->hidden_index);
                result.add(new_vertex, seq_prob * prob);
            }
        }
    }
}

QEnsemble QAction::__dfs(HardwareSpecification &hardware_specification, const shared_ptr<HybridState> &current_vertex,
                         const int &index_ins) const {
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
        QEnsemble result;
        result.insert_new(current_vertex, MyFloat(1));
        return result;
    }

    assert(index_ins < instruction_sequence.size());

    auto current_instruction = instruction_sequence[index_ins];
    QEnsemble temp_result;
    if (current_instruction.instruction_type == InstructionType::Classical) {
        auto new_classical_state = current_vertex->classical_state->apply_instruction(current_instruction);
        auto new_vertex= make_shared<HybridState>(current_vertex->quantum_state, new_classical_state, current_vertex->hidden_index);
        temp_result.insert_new(new_vertex, MyFloat(1));
    } else {
        auto instruction_channel = hardware_specification.get_channel(make_shared<Instruction>(current_instruction));
        if (current_instruction.instruction_type == InstructionType::Measurement) {
            // get successors for 0-measurements
            this->__handle_measure_instruction(current_instruction, *static_pointer_cast<MeasurementChannel>(instruction_channel), current_vertex, temp_result, false );

            // get successors for 1-measurements
            this->__handle_measure_instruction(current_instruction, *static_pointer_cast<MeasurementChannel>(instruction_channel), current_vertex, temp_result, true);
        } else if (current_instruction.gate_name == GateName::Reset){
            // WARNING: use of reset not known in all models, check when using real hardware specifications
            this->__handle_reset_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), current_vertex, temp_result, false);

            this->__handle_reset_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), current_vertex, temp_result, true);
        } else {
            this->__handle_unitary_instruction(current_instruction, *static_pointer_cast<QuantumChannel>(instruction_channel), current_vertex, temp_result);
        }
    }
    QEnsemble result;
    for (auto it : temp_result.values) {
        auto successor = it.first;
        auto prob = it.second;
        auto successors2 = this->__dfs(hardware_specification, successor, index_ins+1);
        for (auto it2 : successors2.values) {
            auto succ2 = it2.first;
            auto prob2 = it2.second;
            result.add(succ2, prob*prob2);
        }
    }

    assert (!result.empty());
    result.normalize();
    return result;
}

QEnsemble QAction::get_successor_states(HardwareSpecification &hardware_specification,
                                       const shared_ptr<QVertex> &current_vertex) const {
    return this->__dfs(hardware_specification, current_vertex->hybrid_state, 0);
}

std::size_t POMDPActionHash::operator()(const shared_ptr<POMDPAction> &action) const {
    return action->id;
}


bool POMDPActionPtrEqual::operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const {
    return *a == *b;
}

void POMDP::parse_transitions(const vector<string> &lines) {
    bool found = false;

    for (int index = 0; index < lines.size();) {
        string line = lines[index];
        if (line == "# Transition function (s,a,s -> p)") {
             assert(!found);
            found = true;
        } else if ((file_format == POMDPFormat::ABHSVI && found) || (file_format == POMDPFormat::F1 && line.size() >0 && line[0] == 'T')) {
            if (line.size() == 0) {
                if (this->file_format == ABHSVI) {
                    return;
                }
                continue;
            }
            bool added = false;
            int bot_from = 0;
            int top_from = this->states.size()-1;
            int bot_to = 0;
            int top_to = this->states.size()-1;

            vector<shared_ptr<POMDPAction>> current_actions;
            double probability;

            if (file_format == POMDPFormat::ABHSVI) {
                vector<string> temp;
                split_str(line, "->", temp);
                assert(temp.size() == 2);
                vector<string> temp2;
                split_str(temp[0], ',', temp2);
                assert(temp2.size() == 3);

                probability = stod(temp[1]);

                if (temp2[0] != "_") {
                    bot_from = stoi(temp2[0]);
                    top_from = bot_from;
                }

                if (temp2[1] != "_") {
                    current_actions.push_back(this->get_action(temp2[1]));
                } else {
                    current_actions = this->actions;
                }

                if (temp2[2] != "_") {
                    bot_to = stoi(temp2[2]);
                    top_to = bot_to;
                }

            } else {
                vector<string> tokens;
                split_str(line, ' ', tokens);
                if (tokens[1] == "*") {
                    current_actions = this->actions;
                } else {
                    current_actions.push_back(this->get_action(tokens[1]));
                }
                if (tokens.size() == 7) {
                        if (tokens[3] != "*") {
                            bot_from = stoi(tokens[3]);
                            top_from = bot_from;
                        }

                        if (tokens[5] != "*") {
                            bot_to = stoi(tokens[5]);
                            top_to = bot_to;
                        }
                        probability = stod(tokens[6]);
                    } else if (tokens.size() == 4) {
                        if (tokens[3] != "*") {
                            bot_from = stoi(tokens[3]);
                            top_from = bot_from;
                        }

                        index++;
                        line = lines[index];
                        vector<string> probs_toks;
                        split_str(line, ' ', probs_toks);

                        assert(probs_toks.size() == this->states.size());

                        for (const auto& action : actions) {
                            for (int from_v = bot_from; from_v <= top_from; from_v++) {
                                for (int to_v = bot_to; to_v <= top_to; to_v++) {
                                    double probability  = stod(probs_toks[to_v]);
                                    this->add_transition(action, from_v, to_v, probability);
                                }
                            }
                        }
                        added = true;
                    } else if (tokens.size() == 2) {
                        unordered_map<int, unordered_map<int, double>> temp_transitions;

                        for (int from_v = bot_from; from_v <= top_from; from_v++) {
                            index+=1;
                            line = lines[index];
                            vector<string> probs_toks;
                            split_str(line, ' ', probs_toks);
                            assert(probs_toks.size() == this->states.size());
                            for (int to_v = bot_to; to_v <= top_to; to_v++) {
                                double probability  = stod(probs_toks[to_v]);
                                temp_transitions[from_v][to_v] = probability;
                            }
                        }

                        for (const auto& action : actions) {
                            for (int from_v = bot_from; from_v <= top_from; from_v++) {
                                for (int to_v = bot_to; to_v <= top_to; to_v++) {
                                    double probability  = temp_transitions[from_v][to_v];
                                    this->add_transition(action, from_v, to_v, probability);
                                }
                            }
                        }
                        added = true;

                    } else {
                        assert(false);
                    }

            }


            if (!added) {
                for (const auto& action : current_actions) {
                    for (int from_v = bot_from; from_v <= top_from; from_v++) {
                        for (int to_v = bot_to; to_v <= top_to; to_v++) {
                            this->add_transition(action, from_v, to_v, probability);
                        }
                    }
                }
            }
            

        }
        index++;
    }
}

void POMDP::parse_reward_function(const vector<string> &lines) {

    bool found = false;

    for (int index=0; index < lines.size();) {
        string line = lines[index];
        if (line == "# Reward function (s,a -> r)") {
            assert(!found);
            found = true;
        } else if ( (found && this->file_format == ABHSVI) || (this->file_format == F1 && line[0] == 'R')) {
            if (line.size() == 0) {
                if (this->file_format == ABHSVI) {
                    return;
                }
                continue;
            }

            int bot_from = 0;
            int top_from = this->states.size()-1;

            vector<shared_ptr<POMDPAction>> current_actions;
            double reward;
            if (this->file_format == ABHSVI) {
                vector<string> temp;
                split_str(line, "->", temp);
                vector<string> temp2;
                split_str(temp[0], ',', temp2);

                reward = stod(temp[1]);

                if (temp2[0] != "_") {
                    bot_from = stoi(temp2[0]);
                    top_from = bot_from;
                }

                if (temp2[1] != "_") {
                    current_actions.push_back(this->get_action(temp2[1]));
                } else {
                    current_actions = this->actions;
                }

            } else {
                vector<string> tokens;
                split_str(line, ' ', tokens);
                if (tokens[1] == "*") {
                    current_actions = this->actions;
                } else {
                    current_actions.push_back(this->get_action(tokens[1]));
                }

                if (tokens[3] != "*") {
                    bot_from = stoi(tokens[3]);
                    top_from = bot_from;
                }

                if (tokens.size() == 9) {
                    reward = stod(tokens[8]);
                } else {
                    index+=1;
                    line = lines[index];
                    vector<string> temp;
                    split_str(line, ' ', temp);
                    reward = stod(temp[0]);
                }

            }



            for (const auto& action : current_actions) {
                for (int from_v = bot_from; from_v <= top_from; from_v++) {
                    this->add_reward(action, from_v, reward);
                }
            }

        }
        index+=1;
    }
}

void POMDP::parse_observation_function(const vector<string> &lines) {

    bool found = false;

    for (int index = 0; index < lines.size();) {
        string line = lines[index];
        if (line == "# Observation function (a,s,o -> p)") {
            assert(!found);
            found = true;
        } else if ( (found && this->file_format == ABHSVI) || (this->file_format == F1 && line[0] == 'O')) {
            if (line.size() == 0) {
                if (this->file_format == ABHSVI) {
                    return;
                }
                continue;
            }
            bool added = false;
            int bot_from = 0;
            int top_from = this->states.size()-1;

            int bot_obs = 0;
            int top_obs = this->observations.size()-1;

            double probability;
            vector<shared_ptr<POMDPAction>> current_actions;


            if (this->file_format == ABHSVI) {
                vector<string> temp;
                split_str(line, "->", temp);
                vector<string> temp2;
                split_str(temp[0], ',', temp2);

                probability = stod(temp[1]);

                if (temp2[0] != "_") {
                    current_actions.push_back(this->get_action(temp2[0]));
                } else {
                    for (auto a : this->actions) {
                        current_actions.push_back(a);
                    }

                    current_actions.push_back(HALT_ACTION);
                }

                if (temp2[1] != "_") {
                    bot_from = stoi(temp2[1]);
                    top_from = bot_from;
                }

                if (temp2[2] != "_") {
                    bot_obs = stoi(temp2[2]);
                    top_obs = bot_obs;
                }

            } else {
                vector<string> tokens;
                split_str(line, ' ', tokens);
                if (tokens[1] == "*") {
                    current_actions = this->actions;
                } else {
                    current_actions.push_back(this->get_action(tokens[1]));
                }
                if (tokens.size() == 7) {
                        if (tokens[3] != "*") {
                            bot_from = stoi(tokens[3]);
                            top_from = bot_from;
                        }

                        if (tokens[5] != "*") {
                            bot_obs = stoi(tokens[5]);
                            top_obs = bot_obs;
                        }
                        probability = stod(tokens[6]);
                    } else if (tokens.size() == 4) {
                        if (tokens[3] != "*") {
                            bot_from = stoi(tokens[3]);
                            top_from = bot_from;
                        }

                        index +=1;
                        line = lines[index];

                        vector<string> probs_toks;
                        split_str(line, ' ', probs_toks);

                        assert(probs_toks.size() == this->observations.size());

                        for (const auto& action : actions) {
                            for (int to_v = bot_from; to_v <= top_from; to_v++) {
                                for (int obs = bot_obs; obs <= top_obs; obs++) {
                                    probability  = stod(probs_toks[obs]);
                                    this->add_obs_transition(action, to_v, obs, probability);
                                }
                            }
                        }
                        added = true;
                    } else if (tokens.size() == 2) {
                        unordered_map<int, unordered_map<int, double>> temp_transitions;

                        for (int from_v = bot_from; from_v <= top_from; from_v++) {
                            index+=1;
                            line = lines[index];
                            vector<string> probs_toks;
                            split_str(line, ' ', probs_toks);
                            assert(probs_toks.size() == this->observations.size());
                            for (int obs = bot_obs; obs <= top_obs; obs++) {
                                probability  = stod(probs_toks[obs]);
                                temp_transitions[from_v][obs] = probability;
                            }
                        }

                        for (const auto& action : actions) {
                            for (int from_v = bot_from; from_v <= top_from; from_v++) {
                                for (int obs = bot_obs; obs <= top_obs; obs++) {
                                    probability  = temp_transitions[from_v][obs];
                                    this->add_obs_transition(action, from_v, obs, probability);
                                }
                            }
                        }
                        added = true;
                    } else {
                        assert(false);
                    }

            }


            if (!added){
                for (auto obs= bot_obs; obs <= top_obs; obs++) {
                    for (const auto& action : current_actions) {
                        for (int from_v = bot_from; from_v <= top_from; from_v++) {
                            this->add_obs_transition(action, from_v, obs, probability);
                        }
                    }
                }
            }

        }
        index+=1;
    }

    assert(found || file_format == F1);
}

void POMDP::parse_initial_tuples(const vector<string> &lines) {

    unordered_set<int> initial_states_;
    bool found = false;

    assert(this->file_format == ABHSVI);

    for (auto line : lines) {
        if (line == "# Initial tuples (n,s)") {
            assert(!found);
            found = true;
        } else if (found) {
            if (line.size() == 0) {
                return;
            }
            vector<string> temp;
            split_str(line, ',', temp);
            int v_id= stoi(temp[1]);
            assert(initial_states_.find(v_id) == initial_states_.end());
            this->initial_states.push_back(this->states[v_id]);
        }
    }

    assert(found);
}

shared_ptr<POMDPAction> POMDP::get_action(const string &str_a) const {
    if (str_a[0] >= '0' && str_a[0] <= '9') {
        int temp_id = stoi(str_a);
        assert(temp_id < this->actions.size());
        return this->actions[temp_id];
    } else {
        for (auto action : this->actions) {
            if (action->name == str_a) {
                return action;
            }
        }
    }
    assert(false);
}

POMDP::POMDP(const string &file_path, const POMDPFormat &file_format) {

    this->file_name = file_path;
    this->file_format = file_format;

    filesystem::path pomdp_path;
    if (file_format == ABHSVI) {
        pomdp_path = abhsvi_benchmarks_path / file_path;
    } else {
        pomdp_path = f1_benchmarks_path / file_path;
    }
    cout << "pomdp path: " << pomdp_path << endl;

    ifstream pomdp_file(pomdp_path);
    if (!pomdp_file.is_open()) {
        std::cerr << "Failed to open stats file: " << pomdp_path << "\n";
        assert(false);
    }

    // collect all lines
    string line;

    vector<string> lines;

    while (getline(pomdp_file, line)) {
        trim(line);
        lines.push_back(line);
    }


    int num_states = pf_get_num_states(lines, file_format);
    int num_actions = pf_get_num_actions(lines, file_format);
    int num_observation = pf_get_num_observations(lines, file_format);

    for (int i = 0; i < num_states; i++) {
        this->states.push_back(make_shared<POMDPVertex>());
    }

    for (int i = 0; i < num_actions; i++) {
            this->actions.push_back(make_shared<POMDPAction>());
    }

    for (int i = 0; i < num_observation; i++) {
        this->observations.insert(i);
    }


    this->parse_transitions(lines);
    this->parse_reward_function(lines);
    this->parse_observation_function(lines);
    if (this->file_format == ABHSVI) {
        this->parse_initial_tuples(lines);
    }


}

void POMDP::print_pomdp() const {
    cout << "states: " << endl;

    for (auto s : states) {
        cout <<  s->id << endl;
    }
    cout << "transitions: " << endl;
    for (auto it : this->transition_matrix) {
        for (const auto it_action : it.second) {
            for (const auto it_successor: it_action.second) {
                cout << it.first->id << " ----- " << it_action.first->id << " " << round_to(it_successor.second.value, 3) << " " << it_successor.first->id << endl;
            }
        }

    }
}

void POMDP::add_transition(const shared_ptr<POMDPAction> &p_action, const int &from_vertex, const int &to_vertex, const double &prob_) {
    assert(prob_ >= 0);
    assert(from_vertex < this->states.size());
    assert(to_vertex < this->states.size());
    shared_ptr<POMDPVertex> p_v_from = this->states[from_vertex];
    shared_ptr<POMDPVertex> p_v_to = this->states[to_vertex];
    this->transition_matrix[p_v_from][p_action][p_v_to] = MyFloat(prob_);
}

void POMDP::add_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &from_vertex,
    const shared_ptr<POMDPVertex> &to_vertex, const MyFloat &prob_) {
    if (is_close(prob_.value, 0)) {
        return;
    }

    assert(prob_ > zero);
    this->transition_matrix[from_vertex][p_action][to_vertex] = MyFloat(prob_);
}

void POMDP::add_obs_transition(const shared_ptr<POMDPAction> &p_action, const int &to_vertex, const int &obs,
    const double &prob_) {
    assert(this->observations.find(obs) != this->observations.end());
    assert(prob_ >= 0);
    assert(to_vertex < this->states.size());
    shared_ptr<POMDPVertex> p_v_to = this->states[to_vertex];

    auto action_d = this->obs_transitions.find(p_action);

    if (action_d == this->obs_transitions.end()) {
        this->obs_transitions.emplace(p_action, unordered_map<shared_ptr<POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>());
    }

    auto to_vertex_d = this->obs_transitions[p_action].find(p_v_to);

    if (to_vertex_d  == this->obs_transitions[p_action].end()) {
        this->obs_transitions[p_action].emplace(p_v_to, unordered_map<int, MyFloat>());
    }
    // } else {
    //     assert(this->obs_transitions[p_action][p_v_to].find(obs) == this->obs_transitions[p_action][p_v_to].end());
    // }
    this->obs_transitions[p_action][p_v_to][obs] = MyFloat(prob_);

}

void POMDP::add_obs_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &p_v_to, const int &obs,
    const double &prob_) {
    assert(this->observations.find(obs) != this->observations.end());
    assert(prob_ >= 0);

    auto action_d = this->obs_transitions.find(p_action);

    if (action_d == this->obs_transitions.end()) {
        this->obs_transitions.emplace(p_action, unordered_map<shared_ptr<POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>());
    }

    auto to_vertex_d = this->obs_transitions[p_action].find(p_v_to);

    if (to_vertex_d  == this->obs_transitions[p_action].end()) {
        this->obs_transitions[p_action].emplace(p_v_to, unordered_map<int, MyFloat>());
    }
    // } else {
    //     assert(this->obs_transitions[p_action][p_v_to].find(obs) == this->obs_transitions[p_action][p_v_to].end());
    // }
    this->obs_transitions[p_action][p_v_to][obs] = MyFloat(prob_);

}


void POMDP::add_obs_transition(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &p_v_to, const int &obs,
    const MyFloat &prob_) {
    assert(this->observations.find(obs) != this->observations.end());
    assert(prob_ > zero);

    auto action_d = this->obs_transitions.find(p_action);

    if (action_d == this->obs_transitions.end()) {
        this->obs_transitions.emplace(p_action, unordered_map<shared_ptr<POMDPVertex>, unordered_map<int, MyFloat>, POMDPVertexHash, POMDPVertexPtrEqual>());
    }

    auto to_vertex_d = this->obs_transitions[p_action].find(p_v_to);

    if (to_vertex_d  == this->obs_transitions[p_action].end()) {
        this->obs_transitions[p_action].emplace(p_v_to, unordered_map<int, MyFloat>());
    }

    this->obs_transitions[p_action][p_v_to][obs] = prob_;

}

void POMDP::add_reward(const shared_ptr<POMDPAction> &p_action, const int &v_, const double &r) {

    assert(v_ < this->states.size());
    shared_ptr<POMDPVertex> v = this->states[v_];

    auto v_d = this->f_reward.find(v);

    if (v_d == this->f_reward.end()) {
        this->f_reward.emplace(v, unordered_map<shared_ptr<POMDPAction>, MyFloat, POMDPActionHash, POMDPActionPtrEqual>());
    }

    auto action_d = this->f_reward[v].find(p_action);


    if( action_d != this->f_reward[v].end()) {
        // only add the first reward seen in the file
        return;
    }

    this->f_reward[v][p_action] = MyFloat(r);

}

void POMDP::add_reward(const shared_ptr<POMDPAction> &p_action, const shared_ptr<POMDPVertex> &v, const MyFloat &r) {
    auto v_d = this->f_reward.find(v);

    if (v_d == this->f_reward.end()) {
        this->f_reward.emplace(v, unordered_map<shared_ptr<POMDPAction>, MyFloat, POMDPActionHash, POMDPActionPtrEqual>());
    }

    auto action_d = this->f_reward[v].find(p_action);

    if( action_d != this->f_reward[v].end()) {
        // only add the first reward seen in the file
        return;
    }
    this->f_reward[v][p_action] = MyFloat(r);
}

MyFloat POMDP::get_obs_prob(const shared_ptr<POMDPAction> &action, const shared_ptr<POMDPVertex> &to_vertex,
                            const int &obs) {
    assert(obs < this->observations.size());
    if (this->obs_transitions.find(action) == this->obs_transitions.end()) {
        return 0.0;
    }

    if (this->obs_transitions.find(action)->second.find(to_vertex) == this->obs_transitions.find(action)->second.end()) {
        return 0.0;
    }

    if (this->obs_transitions.find(action)->second.find(to_vertex)->second.find(obs) == this->obs_transitions.find(action)->second.find(to_vertex)->second.end()) {
        return  0.0;
    }

    return obs_transitions[action][to_vertex][obs];
}

MyFloat POMDP::get_reward(const shared_ptr<POMDPVertex> &v, const shared_ptr<POMDPAction> &action) const {
    if (*action == *HALT_ACTION) return 0.0;
    auto it_v = this->f_reward.find(v);

    if (it_v == this->f_reward.end() || it_v->second.find(action) == it_v->second.end()) {
        return MyFloat(0.0);
    }

    return it_v->second.find(action)->second;
}

vector<shared_ptr<POMDPVertex>> POMDP::get_goal_states() const {
    vector<shared_ptr<POMDPVertex>> result;
    unordered_set<int> visited;
    for (auto action : this->actions) {
        for (auto state : this->states) {
            auto reward = this->get_reward(state, action);
            if (reward.value > 0) {
                if(visited.find(state->id) == visited.end()) {
                    result.push_back(state);
                    visited.insert(state->id);
                }

            }
        }
    }
    cout << "results size: " << result.size() << endl;
    return result;
}

vector<shared_ptr<POMDPVertex>> POMDP::get_states_with_depth(vector<shared_ptr<POMDPVertex>> &start_states, const int &horizon) {

    vector<shared_ptr<POMDPVertex>> result;
    unordered_set<int> visited;

    queue<pair<shared_ptr<POMDPVertex>, int>> q;
    cout << "start states size: " << start_states.size() << endl;
    for (auto state: start_states) {
        q.emplace(state, 0);
        visited.insert(state->id);
    }
    while (!q.empty()) {
        auto current_state = q.front().first;
        auto current_horizon = q.front().second;
        cout << "current horizon: " << current_horizon << endl;
        q.pop();
        assert(current_horizon <= horizon);
        if (current_horizon  == horizon) continue;

        for (auto action : this->actions) {
            for (auto succ : this->states) {
                if (this->transition_matrix[succ][action][current_state] > zero) {
                        if (visited.find(succ->id) == visited.end()) {
                            q.emplace(succ, current_horizon + 1);
                            visited.insert(succ->id);
                            if (current_horizon + 1 == horizon) {
                                result.push_back(succ);
                            }
                        }
                }
            }
        }
    }

    return result;
}

vector<shared_ptr<POMDPVertex>> POMDP::get_random_initial_states(const int &n_states, const int &horizon) {
    if (this->states.size() < n_states) return {};
    auto goal_states = this->get_goal_states();
    auto candidate_states = this->get_states_with_depth(goal_states, horizon);
    if (candidate_states.size() < n_states) return {};

    if (this->states.size() == n_states) return this->states;
    std::mt19937 gen(n_states + this->states.size() + horizon); // fix seed


    vector<shared_ptr<POMDPVertex>> result;
    for (int i = 0; i < n_states; i++) {
        std::uniform_int_distribution<> dist(0, candidate_states.size() - 1);
        int idx = dist(gen);
        result.push_back(candidate_states[idx]);
        candidate_states.erase(candidate_states.begin() + idx);
    }

    return result;
}

void POMDP::normalize_transitions() {
    for (auto action : this->actions) {
        for (auto from_v : this->states) {
            MyFloat total_prob;
            for (auto to_v : this->states) {
                total_prob += this->transition_matrix[from_v][action][to_v].value;
            }
            for (auto to_v : this->states) {
                this->transition_matrix[from_v][action][to_v] /= total_prob;
            }
        }
    }
}

void POMDP::normalize_obs_function() {
    for (auto action : this->actions) {
        for (auto to_v : this->states) {
            MyFloat total_prob;
            for (auto obs : this->observations) {
                total_prob += this->get_obs_prob(action, to_v, obs);
            }
            for (auto obs :this->observations) {
                this->add_obs_transition(action, to_v, obs, (this->get_obs_prob(action, to_v, obs)/total_prob).value);
            }

        }
    }
}

unordered_map<int, unordered_set<int>> POMDP::get_bfs_distances(const shared_ptr<POMDPVertex> &initial_state) {
    queue<pair<shared_ptr<POMDPVertex>, int>> q;
    unordered_set<int> visited;
    q.push(make_pair(initial_state, 0));
    visited.insert(initial_state->id);

    unordered_map<int, unordered_set<int>> bfs_distances;
    bfs_distances[0] = {initial_state->id};

    while (!q.empty()) {
        auto current_state = q.front().first;
        auto current_depth = q.front().second;
        int next_depth = current_depth + 1;
        q.pop();

        for (const auto& succ :this->states) {
            if (visited.find(succ->id) == visited.end()) {
                for (const auto& action : this->actions) {
                    auto prob = this->transition_matrix[current_state][action][succ];
                    if (prob > zero) {
                         if (bfs_distances.find(next_depth) == bfs_distances.end()) {
                             bfs_distances[next_depth] = {};
                         }
                        bfs_distances[next_depth].insert(succ->id);
                        visited.insert(succ->id);
                        q.push(make_pair(succ, next_depth + 1));
                    }
                }
            }
        }
    }

    return bfs_distances;

}

shared_ptr<POMDPVertex> POMDP::get_vertex_by_id(const int &id) const {
    for (auto state :this->states) {
        if (state->id == id) return state;
    }

    assert(false);
}

void POMDP::normalize() {
    this->normalize_obs_function();
    this->normalize_transitions();
    this->check();
}

void POMDP::to_abhsvi_format(vector<int> initial_states, const int &distance, const bool &is_robot) {
    this->normalize();
    unordered_map<int, int> friends;
    friends[54] = 4;
    friends[56] = 6;
    friends[58] = 8;

    friends[59] = 9;
    friends[61] = 11;
    friends[63] = 13;



    assert(this->file_format == POMDPFormat::F1);

    auto pomdp_path = abhsvi_benchmarks_path / (this->file_name + "_" + to_string(initial_states[0]) + "_" + to_string(initial_states[1]) + "_" + to_string(distance));
    cout << "\"" <<(this->file_name + "_" + to_string(initial_states[0]) + "_" + to_string(initial_states[1]) + "_" + to_string(distance)) << "\"" << endl;
    std::ofstream pomdp_file(pomdp_path);

    if (!pomdp_file.is_open()) {
        std::cerr << "Failed to open results file: " << pomdp_path << "\n";
        return;
    }

    // compute states names
    vector<string> v_state_names;
    unordered_map<shared_ptr<POMDPVertex>, int, POMDPVertexHash, POMDPVertexPtrEqual> v_to_id;
    for (int i = 0; i < this->states.size(); i++) {
        v_state_names.push_back("s" + to_string(i));
        v_to_id[this->states[i]] = i;
    }
    string states_names = join(v_state_names, ", ");

    // action names
    vector<string> v_action_names;
    unordered_map<shared_ptr<POMDPAction>, int, POMDPActionHash, POMDPActionPtrEqual> action_to_id;
    for (int i = 0; i < this->actions.size(); i++) {
        v_action_names.push_back("a" + to_string(i));
        action_to_id[this->actions[i]] = i;
    }
    string action_names = join(v_action_names, ", ");

    // observation names
    vector<string> v_obs_names;
    for (int i = 0; i < this->observations.size(); i++) {
        v_obs_names.push_back("o" + to_string(i));
        assert(this->observations.find(i) != this->observations.end());
    }
    string obs_names = join(v_obs_names, ", ");

    // writing in file
    pomdp_file << std::fixed;
    pomdp_file << this->states.size() << ", [" << states_names << "]\n"; // states
    pomdp_file << "1, [e1]\n"; // environments
    pomdp_file << this->actions.size() << ", [" << action_names << "]\n"; // actions
    pomdp_file << this->observations.size() << ", [" << obs_names << "]\n";

    pomdp_file << endl;
    pomdp_file << "0, []" << endl;
    pomdp_file << endl;

    pomdp_file << "# Transition function (s,a,s -> p)" << endl;
    for (auto v_from : this->states) {
        auto id_from = v_to_id.find(v_from)->second;
        for (auto action : this->actions) {
            auto id_action = action_to_id.find(action)->second;
            for (auto v_to : this->states) {
                auto id_to = v_to_id.find(v_to)->second;

                auto prob = this->transition_matrix[v_from][action][v_to].value;
                if (prob > 0) {
                    pomdp_file << id_from << "," << id_action << "," << id_to << " -> " << prob << endl;
                }
            }
        }
    }
    pomdp_file << endl;


    pomdp_file << "# Observation function (a,s,o -> p)" << endl;
    for (auto action : this->actions) {
        auto id_action = action_to_id.find(action)->second;
        for (auto v : this->states) {
            auto id_v = v_to_id.find(v)->second;
            MyFloat total_prob(0);
            for (auto obs : this->observations) {
                double prob = (this->get_obs_prob(action, v, obs) + zero).value;
                if (prob > 0) {
                    total_prob += MyFloat(prob);
                    pomdp_file << id_action << "," << id_v << "," << obs << " -> " << prob << endl;
                }
            }
        }
    }
    pomdp_file << endl;

    pomdp_file << "# Reward function (s,a -> r)" << endl;
    for (auto v : this->states) {
        int v_id = v_to_id.find(v)->second;
        for (auto action : this->actions) {
            int id_action = action_to_id.find(action)->second;
            double reward = this->get_reward(v, action).value;
            if (reward != 0) {
                pomdp_file << v_id << "," << id_action << " -> " << reward << endl;
            }
        }
    }
    pomdp_file << endl;
    pomdp_file << "# Initial tuples (n,s)" << endl;
    for (auto s : initial_states) {
        pomdp_file << "0," << s << endl;
        if (!is_robot) {
            pomdp_file << "0," << friends[s] << endl;
        }
    }
    pomdp_file.close();
}

void POMDP::to_python_code(const string &pomdp_path) {
    std::ofstream pomdp_file(pomdp_path);

    if (!pomdp_file.is_open()) {
        std::cerr << "Failed to open results file: " << pomdp_path << "\n";
        return;
    }

    // compute states names
    unordered_map<shared_ptr<POMDPVertex>, int, POMDPVertexHash, POMDPVertexPtrEqual> v_to_id;
    for (int i = 0; i < this->states.size(); i++) {
        v_to_id[this->states[i]] = i;
    }

    // action names
    unordered_map<shared_ptr<POMDPAction>, int, POMDPActionHash, POMDPActionPtrEqual> action_to_id;
    for (int i = 0; i < this->actions.size(); i++) {
        action_to_id[this->actions[i]] = i;
    }

    // writing in file
    pomdp_file << std::fixed;
    pomdp_file << this->states.size() << endl; // states
    pomdp_file << this->actions.size() << endl; // actions
    pomdp_file << this->observations.size() << endl;

    for (auto v_from : this->states) {
        auto id_from = v_to_id.find(v_from)->second;
        for (auto action : this->actions) {
            auto id_action = action_to_id.find(action)->second;
            for (auto v_to : this->states) {
                auto id_to = v_to_id.find(v_to)->second;

                auto prob = this->transition_matrix[v_from][action][v_to].value;
                if (prob > 0) {
                    pomdp_file << "T," << id_from << "," << id_action << "," << id_to << "," << prob << endl;
                }
            }
        }
    }
    for (auto action : this->actions) {
        auto id_action = action_to_id.find(action)->second;
        for (auto v : this->states) {
            auto id_v = v_to_id.find(v)->second;
            MyFloat total_prob(0);
            for (auto obs : this->observations) {
                double prob = (this->get_obs_prob(action, v, obs) + zero).value;
                if (prob > 0) {
                    total_prob += MyFloat(prob);
                    pomdp_file << "O," << id_action << "," << id_v << "," << obs << "," << prob << endl;
                }
            }
        }
    }
    pomdp_file << endl;

    for (auto v : this->states) {
        int v_id = v_to_id.find(v)->second;
        for (auto action : this->actions) {
            int id_action = action_to_id.find(action)->second;
            double reward = this->get_reward(v, action).value;
            if (reward != 0) {
                pomdp_file << "R," << v_id << "," << id_action << "," << reward << endl;
            }
        }
    }
    pomdp_file.close();

}

void POMDP::check_transitions() {
    for (auto action : this->actions) {
        for (auto from_v : this->states) {
            MyFloat total_prob;
            for (auto to_v : this->states) {
                total_prob += this->transition_matrix[from_v][action][to_v].value;
            }
            assert(is_close(total_prob.value, 1));
        }
    }
}

void POMDP::check_obs_function() {
    for (auto action : this->actions) {
        for (auto to_v : this->states) {
            MyFloat total_prob;
            for (auto obs : this->observations) {
                total_prob += this->get_obs_prob(action, to_v, obs);
            }
            assert(is_close(total_prob.value,1));

        }
    }
}

void POMDP::check() {
    this->check_transitions();
    this->check_obs_function();
}

int POMDP::get_reachable(const int &horizon) {

    unordered_set<int> reachable_states;

    for (const auto& state : this->initial_states) {
        auto bfs_dict = this->get_bfs_distances(state);
        for (int d = 0; d <= horizon; d++) {
            if (bfs_dict.find(d) !=  bfs_dict.end()) {
                for (auto s : bfs_dict[d]) {
                    reachable_states.insert(s);
                }
            }
        }
    }

    return reachable_states.size();
}

shared_ptr<QVertex> QPOMDP::get_vertex(const shared_ptr<HybridState> &new_hs) {
    for (int i = 0; i < this->states.size(); i++) {
        auto old_qvertex = dynamic_pointer_cast<QVertex>(this->states.at(i));
        if (*new_hs == *old_qvertex->hybrid_state) {
            return old_qvertex;
        }

    }
    return nullptr;
}

shared_ptr<QVertex> QPOMDP::create_new_vertex(const shared_ptr<HybridState> &hybrid_state) {
    auto v = this->get_vertex(hybrid_state);
    if (v == nullptr) {
        auto new_vertex = make_shared<QVertex>(hybrid_state);
        this->states.push_back(new_vertex);
        return new_vertex;
    }

    return v;
}


bool Strategy::insert(const shared_ptr<Strategy> &strategy) {
    auto obs = strategy->obs;
    assert (this->obs_to_strategies.find(obs) == this->obs_to_strategies.end());
    this->obs_to_strategies[obs] = strategy;
    return true;

}

Strategy::Strategy(const shared_ptr<POMDPAction> &action, const int &obs) {
    this->action = action;
    this->obs = obs;
}

Strategy::Strategy(const Strategy &strategy) {
    this->action = strategy.action;
    this->obs = strategy.obs;

    for (auto p : strategy.obs_to_strategies) {
        this->obs_to_strategies.insert({p.first, p.second});
    }
}

bool Strategy::operator==(Strategy &other) {
    if (!(*other.action == *this->action)) {
        return false;
    }
    if (this->obs != other.obs) {
        return false;
    }

    if (this->obs_to_strategies.size() != other.obs_to_strategies.size()) {
        return false;
    }

    // check children are the same
    for (auto element : this->obs_to_strategies) {
        auto current_obs = element.first;
        auto left_child = element.second;

        if (other.obs_to_strategies.find(current_obs) == other.obs_to_strategies.end()) {
            return false;
        }

        auto right_child = other.obs_to_strategies[current_obs];
        if (!(*left_child == *right_child)) {
            return false;
        }
    }
    return true;
}

void Strategy::normalize() {
    if (*this->action == *HALT_ACTION) return;
    if (this->obs_to_strategies.empty()) {
        this->insert(make_shared<Strategy>(HALT_ACTION, this->obs));
        return;
    }
    for (auto element : obs_to_strategies) {
        element.second->normalize();
    }
}

void MixedStrategy::normalize() {
    double total = 0;

    for (auto element : this->value) {
        assert(element.second > 0 && (element.second < 1 || is_close(element.second, 1)));
        total += element.second;
    }

    total = round_to(total);

    if (!is_close(total, 1)) {
        cerr << "WARNING MIXED STRATEGY PROBABILITIES: " << total << endl;
    }

    vector<int> indices_to_remove;
    for (int i = 0; i < this->value.size(); i++) {
        this->value[i].second = round(this->value[i].second/total);
        this->value[i].first->normalize();
        if (is_close(this->value[i].second, 0)) {
            indices_to_remove.push_back(i);
        }
    }

    if (indices_to_remove.size() > 0) {
        cerr << "WARNING MIXED STRATEGY REMOVES ZERO PROB. STRATEGIES: " << indices_to_remove.size() << endl;
    }
    std::sort(indices_to_remove.begin(), indices_to_remove.end(), std::greater<>());
    for (auto idx : indices_to_remove) {
        this->value.erase(this->value.begin() + idx);
    }

    std::sort(this->value.begin(), this->value.end(),
      [](const auto& a, const auto& b) {
          return a.second < b.second;
      });
}

MixedStrategy::MixedStrategy(const vector<pair<shared_ptr<Strategy>, double>> &values) {
    this->value = values;
    this->normalize();
}

bool MixedStrategy::operator==(MixedStrategy &other) {

    if (this->value.size() != other.value.size()) {
        return false;
    }

    unordered_set<int> used_indices;
    for (auto element : this->value) {
        auto index = other.find_strategy(element.first, element.second);
        if (index == -1 || used_indices.find(index) != used_indices.end()) {
            return false;
        } else {
            used_indices.insert(index);
        }

    }
    return true;
}

int MixedStrategy::find_strategy(const shared_ptr<Strategy> &strategy, const double &prob) {
    int result = 0;
    for (auto element : this->value) {
        if (is_close(element.second, prob) && *element.first == *strategy) {
            return result;
        }
        result+=1;
    }
    return -1;
}

string to_string(shared_ptr<Strategy> &algorithm, string tabs="") {
    if (algorithm == nullptr) return "";

    string result = tabs + algorithm->action->name + "\n";
    for(auto element : algorithm->obs_to_strategies) {
        auto child = element.second;
        string child_alg;
        {
            if (algorithm->obs_to_strategies.size() > 1) {
                result += tabs + "if c = " + to_string(element.first) + ":\n" ;
                child_alg = to_string(child, tabs+"\t");
            } else {
                child_alg = to_string(child, tabs);
            }
        }
        result += child_alg;
    }

    return result;
}


string to_string(MixedStrategy &algorithm, string tabs = "") {

    if (algorithm.value.size() == 1) {
        return to_string(algorithm.value.at(0).first);
    }

    string result = "";
    result += tabs + "{\n";
    string current_tabs = tabs + "\t";

    auto temp_algorithm = make_shared<MixedStrategy>(vector<pair<shared_ptr<Strategy>, double>>{});
    vector<shared_ptr<MixedStrategy>> new_children;
    for (size_t i = 1; i < algorithm.value.size(); ++i) {
        temp_algorithm->value.push_back(algorithm.value.at(i));
    }

    double condition_prob = round_to(1 - algorithm.value.at(0).second);
    result += to_string(algorithm.value.at(0).first, tabs + "\t");
    result += "\n } ⊕_" + to_string(condition_prob) + " {\n";
    result += to_string(*temp_algorithm, tabs+ "\t");
    result += tabs + "\n}\n";
    return result;
}

bool MixedStrategy::dump(filesystem::path path) {
    // Open file for writing
    std::ofstream out(path);  // creates the file or overwrites if it exists
    if (!out) {
        std::cerr << "Failed to open file: " << path << "\n";
        return false;
    }
    out << to_string(*this);

    out.close();
    return true;
}



json to_json(Strategy &algorithm) {
    vector<json> j_children;

    for (auto element : algorithm.obs_to_strategies) {
        j_children.push_back(to_json(*element.second));
    }

    return json{
        {"action", algorithm.action->name},
        {"children", j_children},
        {"classical_state", algorithm.obs},
    };
}


json to_json(MixedStrategy &algorithm) {
    vector<json> j_children;
    vector<double> children_probs;
    for (const auto& child : algorithm.value) {
        j_children.push_back(to_json(*child.first));
        children_probs.push_back(child.second);
    }

    return json{
        {"action", RANDOM_BRANCH->name},
        {"children", j_children},
        {"children_probs", children_probs}
    };
}


bool MixedStrategy::dump_raw(filesystem::path path) {
    // Dump into a file
    std::ofstream file(path);
    if (!file) {
        std::cerr << "Error opening file for writing\n";
        return false;
    }
    auto j = to_json(*this);
    file << j.dump(4);  // "4" = pretty print with indentation
    file.close();

    return true;
}
