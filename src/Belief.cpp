#include "Belief.hpp"

#include <csignal>
#include <iostream>

#include "utils.hpp"



MyFloat Belief::get(const shared_ptr<const POMDPVertex> &v) {
    assert(!this->is_unreached);
    if(this->probs.find(v) == this->probs.end()){
        return MyFloat(0.0);
    }
    return this->probs[v];
}

void Belief::set_val(const shared_ptr<const POMDPVertex> &v, const MyFloat &prob) {
    this->is_unreached = false;
    if (prob == zero) return;
    this->probs.insert_or_assign(v, MyFloat(prob));
}

void Belief::add_val(const shared_ptr<const POMDPVertex> &v, const MyFloat &val) {
    this->is_unreached = false;
    assert(v != nullptr);
    auto final_val =  this->get(v) + val;
    this->probs.insert_or_assign(v, final_val);
    if (this->probs.at(v) == zero) {
        this->probs.erase(v);
    }
}

bool Belief::operator==(const Belief& other) const {
    if(this->is_unreached != other.is_unreached) return false;
    if(this->probs.size() != other.probs.size()) return false;

    for (auto it : this->probs) {
        auto it2 = other.probs.find(it.first);
        if (it2 != other.probs.end()) {
            if (it.second != it2->second) {
                return false;
            }
        }

    }

    return true;
}

void Belief::print() const {
    if (this->is_unreached) {
        cout << "unreach." << endl;
    }
    for (auto it : this->probs) {
        cout << it.first->id << "--" << it.second << endl;
    }
}

int Multibelief::get_belief_index(const shared_ptr<Belief> &belief, const unordered_set<int> &available_indices) const {
    for (auto i : available_indices) {
        if (this->beliefs[i] == belief) {
            return i;
        }
    }

    return -1;
}

Multibelief::Multibelief(const multibelief_type &beliefs, int obs) {
    this->beliefs = beliefs;
    this->obs = obs;
}

bool Multibelief::check_multibelief() const {

    for (auto belief : beliefs) {
        assert(belief->get_obs() == this->obs);
    }
    return true;
}

int Multibelief::get_obs() const {
    return this->obs;
}

bool Multibelief::operator==(const Multibelief &other) const {
    assert(this->beliefs.size() == other.beliefs.size());

    unordered_set<int> unused_ids;

    for (int i = 0; i < this->beliefs.size(); i++) {
        unused_ids.insert(i);
    }
    for (int i = 0; i < this->beliefs.size(); i++) {
        int index = other.get_belief_index(this->beliefs[i], unused_ids);
        if (index == -1) {return false;}
        unused_ids.erase(index);
    }

    return true;
}

bool is_multibelief_in_list(const vector<shared_ptr<Multibelief>> &multibeliefs,
    const shared_ptr<Multibelief> &multibelief) {

    for (auto m : multibeliefs) {
        if (m == multibelief) {
            return true;
        }
    }

    return false;
}
