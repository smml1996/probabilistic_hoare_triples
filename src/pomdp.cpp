
#include "pomdp.hpp"

#include <cassert>
#include <fstream>
#include <iostream>

#include "utils.hpp"

int POMDPVertex::local_counter = 0;
int POMDPAction::local_counter = 0;

POMDPVertex::~POMDPVertex() {
    // delete this->hybrid_state;
}

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

std::size_t POMDPActionHash::operator()(const shared_ptr<POMDPAction> &action) const {
    return action->id;
}


bool POMDPActionPtrEqual::operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const {
    return *a == *b;
}

void POMDP::parse_transitions(const vector<string> &lines) {
    bool found = false;
    assert(this->file_format == ABHSVI);

    for (auto line : lines) {
        if (line == "# Transition function (s,a,s -> p)") {
             assert(!found);
            found = true;
        } else if (found) {
            if (line.size() == 0) {
                return;
            }

            vector<string> temp;
            split_str(line, "->", temp);
            assert(temp.size() == 2);
            vector<string> temp2;
            split_str(temp[0], ',', temp2);
            assert(temp2.size() == 3);

            double probability = stod(temp[1]);

            int bot_from = 0;
            int top_from = this->states.size()-1;
            int bot_to = 0;
            int top_to = this->states.size()-1;

            vector<shared_ptr<POMDPAction>> current_actions;

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


            for (const auto& action : current_actions) {
                for (int from_v = bot_from; from_v <= top_from; from_v++) {
                    for (int to_v = bot_to; to_v <= top_to; to_v++) {
                        this->add_transition(action, from_v, to_v, probability);
                    }
                }
            }

        }
    }

    assert(found);
}

void POMDP::parse_reward_function(const vector<string> &lines) {

    bool found = false;
    assert(this->file_format == ABHSVI);

    for (auto line : lines) {
        if (line == "# Reward function (s,a -> r)") {
            assert(!found);
            found = true;
        } else if (found) {
            if (line.size() == 0) {
                return;
            }

            vector<string> temp;
            split_str(line, "->", temp);
            vector<string> temp2;
            split_str(temp[0], ',', temp2);

            double reward = stod(temp[1]);

            int bot_from = 0;
            int top_from = this->states.size()-1;

            vector<shared_ptr<POMDPAction>> current_actions;

            if (temp2[0] != "_") {
                bot_from = stoi(temp2[0]);
                top_from = bot_from;
            }

            if (temp2[1] != "_") {
                current_actions.push_back(this->get_action(temp2[1]));
            } else {
                current_actions = this->actions;
            }


            for (const auto& action : current_actions) {
                for (int from_v = bot_from; from_v <= top_from; from_v++) {
                    this->add_reward(action, from_v, reward);
                }
            }

        }
    }

    assert(found);
}

void POMDP::parse_observation_function(const vector<string> &lines) {

    bool found = false;
    assert(this->file_format == ABHSVI);

    for (auto line : lines) {
        if (line == "# Observation function (a,s,o -> p)") {
            assert(!found);
            found = true;
        } else if (found) {
            if (line.size() == 0) {
                return;
            }

            vector<string> temp;
            split_str(line, "->", temp);
            vector<string> temp2;
            split_str(temp[0], ',', temp2);

            double probability = stod(temp[1]);

            int bot_from = 0;
            int top_from = this->states.size()-1;

            int bot_obs = 0;
            int top_obs = this->observations.size()-1;

            vector<shared_ptr<POMDPAction>> current_actions;

            if (temp2[0] != "_") {
                current_actions.push_back(this->get_action(temp2[0]));
            } else {
                for (auto a : this->actions) {
                    current_actions.push_back(a);
                }

                current_actions.push_back(halt_action);
            }

            if (temp2[1] != "_") {
                bot_from = stoi(temp2[1]);
                top_from = bot_from;
            }

            if (temp2[2] != "_") {
                bot_obs = stoi(temp2[2]);
                top_obs = bot_obs;
            }


            for (auto obs= bot_obs; obs <= top_obs; obs++) {
                for (const auto& action : current_actions) {
                    for (int from_v = bot_from; from_v <= top_from; from_v++) {
                        this->add_obs_transition(action, from_v, obs, probability);
                    }
                }
            }


        }
    }

    assert(found);
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

void POMDP::set_rewards_halt() {

    for (auto v : this->states) {
        if (this->f_reward.find(v) != this->f_reward.end()) {
            if (this->f_reward.find(v)->second.find(halt_action) == this->f_reward.find(v)->second.end()) {
                this->f_reward[v][halt_action] = MyFloat(0);
            }
        }
    }
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
    cout << file_path << endl;
    this->file_format = file_format;
    assert(file_format == POMDPFormat::ABHSVI);
    auto pomdp_path = benchmarks_path / file_path;

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

    int num_states = get_basic_abhsvi(lines[0]);
    int num_actions = get_basic_abhsvi(lines[2]);
    int num_observation = get_basic_abhsvi(lines[3]);

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
    this->parse_initial_tuples(lines);
    this->set_rewards_halt();
}

POMDP::~POMDP() {
    // for (auto state : this->states) {
    //     delete state;
    // }
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
    } else {
        assert(this->obs_transitions[p_action][p_v_to].find(obs) == this->obs_transitions[p_action][p_v_to].end());
    }
    this->obs_transitions[p_action][p_v_to][obs] = MyFloat(prob_);

}

void POMDP::add_reward(const shared_ptr<POMDPAction> &p_action, const int &v_, const double &r) {
    assert(v_ < this->states.size());
    shared_ptr<POMDPVertex> v = this->states[v_];

    auto v_d = this->f_reward.find(v);

    if (v_d == this->f_reward.end()) {
        this->f_reward.emplace(v, unordered_map<shared_ptr<POMDPAction>, MyFloat, POMDPActionHash, POMDPActionPtrEqual>());
    }

    auto action_d = this->f_reward[v].find(p_action);


    assert( action_d == this->f_reward[v].end());

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
    auto it_v = this->f_reward.find(v);

    if (it_v == this->f_reward.end() || it_v->second.find(action) == it_v->second.end()) {
        return MyFloat(0.0);
    }

    return it_v->second.find(action)->second;
}
