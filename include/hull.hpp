#ifndef PROBABILISTC_HOARE_TRIPLES_HULL_H
#define PROBABILISTC_HOARE_TRIPLES_HULL_H

#include <boost/multiprecision/cpp_int.hpp>
#include "utils.hpp"
#include <CGAL/Polyhedron_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Epick_d.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Homogeneous_d.h>

#include "pomdp.hpp"

typedef CGAL::Gmpz RT;

using K3 = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point3 = K3::Point_3;
using Polyhedron3 = CGAL::Polyhedron_3<K3>;

using KernelD = CGAL::Homogeneous_d<RT>;
using PointD  = KernelD::Point_d;
typedef CGAL::Convex_hull_d<KernelD> Convex_hull_d;

struct Flags {
    bool has_upper = false;
    bool has_lower = false;
};

class MWP {
public:
    shared_ptr<Strategy> strategy;
    vector<MyFloat> values;
    MWP(const int &size, const shared_ptr<Strategy> &strategy);
    MWP(const int &size, const Strategy &strategy);
    double get(const int &index) const;
    bool operator<=(const MWP &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] > other.values[i]) {
                return false;
            }
        }
        return true;
    }

    shared_ptr<MWP> add_mwp(const shared_ptr<MWP> &right, const bool &use_child=false) const {
        // returns a new mwp without modifying any of the operands
        // adds right operand as child

        shared_ptr<MWP> result;

        // the strategy of the right operand is the initial action
        result = make_shared<MWP>(this->values.size(), make_shared<Strategy>(*this->strategy));

        // add values of operands and store in new result mwp
        assert(this->values.size() == right->values.size());
        for (int i = 0; i< this->values.size(); i++) {
            result->values[i] = this->values[i] + right->values[i];
        }

        if (use_child) {
            assert(*right->strategy->action == *Strategy::TEMP_STRATEGY->action);
            assert(this->strategy->obs_to_strategies.size() == 0); // assumption: called on strategies without children.
            for (auto e_child : right->strategy->obs_to_strategies) {
                result->strategy->insert(e_child.second);
            }
        } else {
            result->strategy->insert(right->strategy);
        }

        return result;
    }



    int size() const {return this->values.size();}

    bool operator==(const MWP &other) const;

};

struct MWPPtrComp {
    bool operator()(const shared_ptr<MWP>& a, const shared_ptr<MWP>& b) const {
        assert(a->size() == b->size());

        for (int i = 0; i < a->size(); i++) {
            if (a->values[i] != b->values[i]) {
                return a->values[i] < b->values[i];
            }
        }

        return false;
    }
};

class Hull {
    int dimension;
    bool convexify;

    vector<Point3> points3;
    vector<PointD> pointsD;

    Polyhedron3 poly3;
    Convex_hull_d poly4;

    bool should_convexify();
    bool update_pareto_front(const shared_ptr<MWP> &mwp);
    bool convex_add2(const shared_ptr<MWP> &mwp);
    bool convex_add3(const shared_ptr<MWP> &mwp);
    bool convex_add4(const shared_ptr<MWP> &mwp);
    bool is_upper_hull(const shared_ptr<MWP> &mwp);
    int last_size; // used when convexify=true. Records the size of upper_hull after the last convexification (also updated when upper_hull decreases size).
public:
    static int size_to_convexify; // every time upper_hull last_size increases by this amount we convexify again.
    set<shared_ptr<MWP>, MWPPtrComp> upper_hull;
    Hull(const int &dimension, const bool &convexify);
    bool add_point(const shared_ptr<MWP> &mwp);
    void clear(const int &dimension_, const bool &convexify_);
    int size() const;
};
#endif //PROBABILISTC_HOARE_TRIPLES_HULL_H