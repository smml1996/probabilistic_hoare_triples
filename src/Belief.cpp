#include "Belief.hpp"

#include <csignal>
#include <iostream>

#include "utils.hpp"

MyFloat Belief::get_sum(int precision) const {
    MyFloat result("0", precision);

    for (auto & prob : this->probs) {
        result = result + prob.second;
    }

    return result;
}

Belief::Belief(const bool &) {
    this->is_unreached = true;
}

Belief::Belief() {
    this->is_unreached = false;
}

MyFloat Belief::get(const shared_ptr<POMDPVertex> &v, int precision) {
    if(this->probs.find(v) == this->probs.end()){
        return MyFloat("0", precision);
    }
    return this->probs[v];
}

void Belief::set_val(const shared_ptr<POMDPVertex> &v, const MyFloat &prob) {
    if (prob == MyFloat("0", prob.precision)) return;
    this->probs.insert_or_assign(v, MyFloat(prob));
}

void Belief::add_val(const shared_ptr<POMDPVertex> &v, const MyFloat &val) {
    assert(v != nullptr);
    assert(v->hybrid_state != nullptr);
    auto final_val =  this->get(v, val.precision) + val;
    this->probs.insert_or_assign(v, MyFloat(final_val));
    if (this->probs.at(v) == MyFloat("0", val.precision)) {
        this->probs.erase(v);
    }
}

bool Belief::is_normalized(int precision) const {
    auto my_sum = to_double(this->get_sum(precision));
    if (is_close(my_sum, 1.0, precision)) {
        return true;
    }

    return false;
}

bool Belief::operator==(const Belief& other) const {
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
    for (auto it : this->probs) {
        cout << *it.first << "--" << it.second << endl;
    }
}


std::size_t BeliefHash::operator()(const shared_ptr<Belief> &belief_) const {
    auto belief = *belief_;
    std::size_t seed = 0;

    std::vector<std::pair<shared_ptr<POMDPVertex>, MyFloat>> items(belief.probs.begin(), belief.probs.end());

    std::sort(items.begin(), items.end(),
      [](auto &a, auto &b) {
          if (a.second == b.second) {
              return a.first->id < b.first->id;
          }
          return a.second < b.second;  // ascending by MyFloat
          // or b.second < a.second for descending
      });


    for (const auto &kv : items) {
        std::hash<std::string> float_hasher;
        POMDPVertexHash vertex_hasher;
        std::size_t h1 = vertex_hasher(kv.first);
        std::size_t h2 = float_hasher(to_string(kv.second) + to_string(kv.second));

        // combine (boost::hash_combine style)
        seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

MyFloat l1_norm(const Belief &b1, const Belief &b2, int precision) {
    MyFloat result("0", precision);
    for (auto it = b1.probs.begin(); it != b1.probs.end(); ++it) {
        auto it2 = b2.probs.find(it->first);
        if (it2 != b2.probs.end()) {
            auto temp = MyFloat("-1", precision) * it2->second ;
            result = result + abs(it->second + temp);
        } else {
            result = result + it->second;
        }
    }
    return result;
}

Belief normalize_belief(const Belief &belief, int precision) {
    Belief result;
    MyFloat total("0", precision);
    for (auto it : belief.probs) {
        total = total + it.second;
    }

    for (auto it : belief.probs) {
        // MyFloat value = it.second / total;
        // result.set_val(it.first, value);
    }

    return result;
}

cpp_int get_belief_cs(const Belief &belief) {
    cpp_int current_classical_state = -1;
    for(auto & prob : belief.probs) {
        if (current_classical_state == -1) {
            current_classical_state = prob.first->hybrid_state->classical_state->get_memory_val();
        } else {
            assert(prob.first->hybrid_state->classical_state->get_memory_val() == current_classical_state);
        }
    }

    return current_classical_state;
}

Multibelief::Multibelief(const multibelief_type &beliefs, cpp_int obs) {
    this->beliefs = beliefs;
    this->obs = obs;
}

bool Multibelief::check_multibelief() {

    for (auto belief : beliefs) {
        assert(belief->get_obs() == this->obs);
    }
    return true;
}

cpp_int Multibelief::get_obs() const {
    return this->obs;
}


double VertexDict::get(const shared_ptr<POMDPVertex> &v) {
    if (this->probs.find(v) == this->probs.end()) {
        return 0.0;
    }
    return this->probs.at(v);
}

void VertexDict::set_val(const shared_ptr<POMDPVertex> &v, const double &prob) {
    if (is_close(prob, 0, 10)) {
        return;
    }
    this->probs[v] = prob;
}


void VertexDict::add_val(const shared_ptr<POMDPVertex> &v, const double &val) {
    assert(v != nullptr);
    auto final_val =  this->get(v) + val;
    this->probs.insert_or_assign(v, final_val);
    if (is_close(this->probs.at(v), 0, 10)) {
        this->probs.erase(v);
    }
}

bool Strategy::insert(const shared_ptr<Strategy> &strategy) {
    auto obs = strategy->obs;
    assert (this->obs_to_strategies.find(obs) == this->obs_to_strategies.end());
    this->obs_to_strategies[obs] = strategy;
    return true;

}

shared_ptr<Algorithm> Strategy::to_algorithm() {
    shared_ptr<Algorithm> result = make_shared<Algorithm>(this->action, this->obs, -1, this->horizon);

    for (auto obs_strat : obs_to_strategies) {
        result->children.push_back(obs_strat.second->to_algorithm());
    }

    return result;
}

Strategy::Strategy(const int &horizon, const shared_ptr<POMDPAction> &action, const cpp_int &obs) {
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
        if(!is_close(prob, 0.0, 6)) {
            auto strat = mapping.at(i);
            this->value.push_back(make_pair(strat, prob));
        }
    }
}

shared_ptr<Algorithm> MixedStrategy::to_algorithm() {

    auto new_head = make_shared<Algorithm>(make_shared<POMDPAction>(random_branch), -1, 5, -1); // we are not going to use precision
    assert(new_head->children.size() == 0);
    int count = 0;
    for(auto element : this->value) {
        auto prob = element.second;
        new_head->children.push_back(element.first->to_algorithm());
        new_head->children_probs.insert({count, prob});
        count += 1;

    }
    return new_head;
}
