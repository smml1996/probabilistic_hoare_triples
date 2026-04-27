#ifndef BELIEFS_H
#define BELIEFS_H
#include <cassert>
#include "pomdp.hpp"
#include "utils.hpp"
#include <map>



class Belief {
public:
    bool is_unreached = false;
    int obs = -1;
    unordered_map<shared_ptr<POMDPVertex>, MyFloat, POMDPVertexHash, POMDPVertexPtrEqual> probs;
    Belief() {
        is_unreached = true;
    }
    Belief(const shared_ptr<POMDPVertex> &v, const int& obs) {
        this->set_val(v, 1);
        this->obs = obs;
        this->is_unreached = false;
    }

    Belief(const int& obs) {
        this->obs = obs;
        this->is_unreached = false;
    }

    inline int get_obs() const {
        assert(obs > -1);
        return this->obs;
    }

    MyFloat get(const shared_ptr<POMDPVertex> &v);

    void set_val(const shared_ptr<POMDPVertex> &v, const MyFloat &prob);

    void add_val(const shared_ptr<POMDPVertex> &v, const MyFloat &val);

    bool operator==(const Belief& other) const;

    void print() const;
};


using multibelief_type = vector<shared_ptr<Belief>>;

class Multibelief {
    int obs;
public:
    multibelief_type beliefs;
    Multibelief(const multibelief_type &beliefs, int obs);
    bool check_multibelief() const;
    int get_obs() const;
};


#endif