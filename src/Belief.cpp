#include "Belief.hpp"

#include <csignal>
#include <iostream>

#include "utils.hpp"



MyFloat Belief::get(const shared_ptr<POMDPVertex> &v) {
    assert(!this->is_unreached);
    if(this->probs.find(v) == this->probs.end()){
        return 0.0;
    }
    return this->probs[v];
}

void Belief::set_val(const shared_ptr<POMDPVertex> &v, const MyFloat &prob) {
    this->is_unreached = false;
    if (prob == 0.0) return;
    this->probs.insert_or_assign(v, MyFloat(prob));
}

void Belief::add_val(const shared_ptr<POMDPVertex> &v, const MyFloat &val) {
    this->is_unreached = false;
    assert(v != nullptr);
    auto final_val =  this->get(v) + val;
    this->probs.insert_or_assign(v, final_val);
    if (this->probs.at(v) == zero) {
        this->probs.erase(v);
    }
}

bool Belief::operator==(const Belief& other) const {
    assert(!this->is_unreached);
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

bool Strategy::insert(const shared_ptr<Strategy> &strategy) {
    auto obs_ = strategy->obs;
    assert (this->obs_to_strategies.find(obs_) == this->obs_to_strategies.end());
    this->obs_to_strategies[obs_] = strategy;
    return true;

}


Strategy::Strategy(const int &horizon, const shared_ptr<POMDPAction> &action, const int &obs) {
    this->horizon = horizon;
    this->action = action;
    this->obs = obs;
}

Strategy::Strategy(const Strategy &strategy) {
    this->horizon = strategy.horizon;
    this->action = strategy.action;
    this->obs = strategy.obs;

    for (auto p : strategy.obs_to_strategies) {
        this->obs_to_strategies.insert({p.first, p.second});
    }
}

MixedStrategy::MixedStrategy(const vector<double> &probs, const unordered_map<int, shared_ptr<Strategy>> &mapping) {
    for (int i = 0; i < probs.size(); ++i) {
        auto prob = probs[i];
        if(!is_close(prob, 0.0)) {
            auto strat = mapping.at(i);
            this->value.push_back(make_pair(strat, prob));
        }
    }
}
