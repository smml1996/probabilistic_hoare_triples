#ifndef BELIEFS_H
#define BELIEFS_H
#include <cassert>

#include "floats.hpp"
#include "pomdp.hpp"
#include <boost/multiprecision/cpp_int.hpp>

class Belief {
    cpp_int obs = -1;
public:
    [[nodiscard]] MyFloat get_sum(int precision) const;
    unordered_map<shared_ptr<POMDPVertex>, MyFloat, POMDPVertexHash, POMDPVertexPtrEqualID> probs;

    Belief();
    Belief(const shared_ptr<POMDPVertex> &v, int &precision) {
        MyFloat one = MyFloat(1, precision);
        this->set_val(v, one);
        this->obs = v->hybrid_state->classical_state->get_memory_val();
    }

    inline cpp_int get_obs() const {
        assert(obs > -1);
        return this->obs;
    }

    MyFloat get(const shared_ptr<POMDPVertex> &v, int precision);

    void set_val(const shared_ptr<POMDPVertex> &v, const MyFloat &prob);

    void add_val(const shared_ptr<POMDPVertex> &v, const MyFloat &val);

    bool is_normalized(int precision) const;

    bool operator==(const Belief& other) const;

    void print() const;
};

struct BeliefHash {
    std::size_t operator()(const Belief &) const;
};

MyFloat l1_norm(const Belief &b1, const Belief &b2, int precision);

Belief normalize_belief(const Belief &belief, int precision);

cpp_int get_belief_cs(const Belief &belief);

class VertexDict {
public:
    vertex_dict probs;

    double get(const shared_ptr<POMDPVertex> &v);

    void set_val(const shared_ptr<POMDPVertex> &v, const double &prob);

    void add_val(const shared_ptr<POMDPVertex> &v, const double &prob);
};



#endif