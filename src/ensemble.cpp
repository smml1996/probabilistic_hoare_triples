//
// Created by Stefanie Muroya Lei on 02.01.26.
//
#include "ensemble.hpp"
#include "boost_clean.hpp"

Ensemble<z3::expr> get_symbolic_ensemble(const std::vector<shared_ptr<Ensemble<MyFloat>>> &ensembles, const z3::expr_vector &weights, z3::context &ctx, const int &mc_precision) {
    Ensemble<z3::expr> result(mc_precision);

    assert( weights.size() == ensembles.size());
    for (int i = 0; i <ensembles.size(); i++) {
        for (auto e : ensembles[i]->probs) {
            result.add_prob(e.first, weights[i] * ctx.real_val(to_string(e.second).c_str()));
        }
    }
    return result;
}

shared_ptr<Ensemble<MyFloat>> to_myfloat(const shared_ptr<Ensemble<double>> &ensemble, const int &mc_precision, const int &max_depth) {
    shared_ptr<Ensemble<MyFloat>> result = make_shared<Ensemble<MyFloat>>(mc_precision * (max_depth + 1));

    for (auto p : ensemble->probs) {
        result->add_prob(p.first, MyFloat(to_string(p.second), mc_precision* (max_depth + 1)));
    }

    return result;
}

shared_ptr<Ensemble<double>> to_double(const shared_ptr<Ensemble<MyFloat>> &ensemble, const int &mc_precision) {
    shared_ptr<Ensemble<double>> result = make_shared<Ensemble<double>>(mc_precision);

    for (auto p : ensemble->probs) {
        result->add_prob(p.first, round_to(to_double(p.second), mc_precision));
    }

    return result;
}

template <typename FloatT>
int Ensemble<FloatT>::does_hybrid_state_exists(shared_ptr<HybridState> &state) const{
    for (int i = 0; i < this->probs.size(); i++) {
        auto hs = this->probs[i];
        if (*hs.first == *state) {
            return i;
        }
    }
    return -1;
}

template <>
bool Ensemble<MyFloat>::operator==(const Ensemble<MyFloat> &other) const {
    if (other.probs.size() != this->probs.size()) return false;
    for (auto e : other.probs) {
        auto index = this->does_hybrid_state_exists(e.first);
        if (index == -1) return false;
        if (e.second != other.probs[index].second) return false;
    }
    return true;
}

template <>
bool Ensemble<double>::operator==(const Ensemble<double> &other) const {
    if (other.probs.size() != this->probs.size()) return false;
    for (auto e : other.probs) {
        auto index = this->does_hybrid_state_exists(e.first);
        if (index == -1) return false;
        if (!is_close(e.second,other.probs[index].second, this->precision)) return false;
    }
    return true;
}

template <>
bool Ensemble<z3::expr>::operator==(const Ensemble<z3::expr> &other) const {
    if (other.probs.size() != this->probs.size()) return false;
    for (auto e : other.probs) {
        auto index = this->does_hybrid_state_exists(e.first);
        if (index == -1) return false;
        if (!is_close(e.second,other.probs[index].second, this->precision)) return false;
    }
    return true;
}

template <typename FloatT>
void Ensemble<FloatT>::add_prob(shared_ptr<HybridState> &hs, FloatT value) {
    assert(this->precision != -1);
    int index = this->does_hybrid_state_exists(hs);
    if (index == -1) {
        this->probs.push_back(make_pair(hs, value));
        return;
    }
    auto temp = this->probs[index];
    this->probs[index] = make_pair(hs, value + temp.second) ;
}

template <>
inline double Ensemble<double>::get_weight() {
    double result = 0;

    for (const auto& e : this->probs) {
        result += e.second;
    }
    return result;
}

template <>
z3::expr Ensemble<z3::expr>::get_weight() {
    throw runtime_error("get_weight not implemented for Ensemble<MyFloat>");
}

template <>
MyFloat Ensemble<MyFloat>::get_weight() {
    MyFloat result("0", this->precision);

    for (const auto& e : this->probs) {
        result = result + e.second;
    }

    return result;
}

template<>
void Ensemble<double>::normalize() {
    auto w = get_weight();
    for (auto & prob : this->probs) {
        prob.second = round_to(prob.second/w, this->precision);
    }
}

template<>
void Ensemble<MyFloat>::normalize() {
    throw runtime_error("normalize not implemented for Ensemble<MyFloat>");
}

template<>
void Ensemble<z3::expr>::normalize() {
    throw runtime_error("normalize not implemented for Ensemble<z3::expr>");
}