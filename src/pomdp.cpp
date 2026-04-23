
#include "pomdp.hpp"

#include <cassert>
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

bool POMDPAction::operator==(const POMDPAction &other) const {
    return this->id == other.id;
}

std::size_t POMDPActionHash::operator()(const shared_ptr<POMDPAction> &action) const {
    return action->id;
}


bool POMDPActionPtrEqual::operator()(const shared_ptr<POMDPAction> &a, const shared_ptr<POMDPAction> &b) const {
    return *a == *b;
}

shared_ptr<POMDPAction> POMDP::get_action(const string &str_a) const {
    if (str_a[0] >= '0' and str_a[0] <= '9') {
        return this->actions[stoi(str_a)];
    } else {
        for (auto action : this->actions) {
            if (action->name == str_a) {
                return action;
            }
        }
    }
    assert(false);
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
    shared_ptr<POMDPVertex> p_v_from = this->states[from_vertex];
    shared_ptr<POMDPVertex> p_v_to = this->states[to_vertex];

    auto from_vertex_d = this->transition_matrix.find(p_v_from);
    if (from_vertex_d == this->transition_matrix.end()) {
        this->transition_matrix.emplace(p_v_from, unordered_map<shared_ptr<POMDPAction>, unordered_map<shared_ptr<POMDPVertex>,
        MyFloat, POMDPVertexHash, POMDPVertexPtrEqual>, POMDPActionHash, POMDPActionPtrEqual>());
    }
    auto action_d = this->transition_matrix.find(p_v_from)->second.find(p_action);

    if (action_d == from_vertex_d->second.end()) {
        from_vertex_d->second.emplace(p_action,  unordered_map<shared_ptr<POMDPVertex>,
        MyFloat, POMDPVertexHash, POMDPVertexPtrEqual>());
    } else {
        auto temp = action_d->second.find(p_v_to);
        assert(temp != action_d->second.end());
    }
    this->transition_matrix[p_v_from][p_action][p_v_to] = MyFloat(prob_);
}

void POMDP::add_obs_transition(const shared_ptr<POMDPAction> &p_action, const int &to_vertex, const int &obs,
    const double &prob_) {
    assert(this->observations.find(obs) != this->observations.end());
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

MyFloat POMDP::get_obs_prob(const shared_ptr<POMDPAction> &action, const shared_ptr<POMDPVertex> &to_vertex,
    const int &obs) {
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
