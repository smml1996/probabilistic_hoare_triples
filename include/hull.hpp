//
// Created by Stefanie Muroya Lei on 23.04.26.
//

#ifndef PROBABILISTC_HOARE_TRIPLES_HULL_H
#define PROBABILISTC_HOARE_TRIPLES_HULL_H

#include <boost/multiprecision/cpp_int.hpp>
#include "utils.hpp"
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Convex_hull_3.h>
#include <CGAL/Epick_d.h>
#include <CGAL/convex_hull_d.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Gmpz.h>
#include <CGAL/Homogeneous_d.h>

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
    vector<MyFloat> values;
    MWP(const int &size);
    double get(const int &index);
    bool operator<=(const MWP &other) const {
        assert(this->values.size() == other.values.size());
        for (int i = 0; i < this->values.size(); i++) {
            if (this->values[i] > other.values[i]) {
                return false;
            }
        }
        return true;
    }

    shared_ptr<MWP> operator+(const MWP &other) {
        shared_ptr<MWP> result = make_shared<MWP>(this->values.size());
        assert(this->values.size() == other.values.size());

        for (int i = 0; i< this->values.size(); i++) {
            result->values[i] = this->values[i] + other.values[i];
        }
        return result;
    }

    int size() const {return this->values.size();}

    bool operator==(const MWP &other);

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

    bool update_pareto_front(const shared_ptr<MWP> &mwp);
    bool convex_add2(const shared_ptr<MWP> &mwp);
    bool convex_add3(const shared_ptr<MWP> &mwp);
    bool convex_add4(const shared_ptr<MWP> &mwp);

    bool is_upper_hull(const shared_ptr<MWP> &mwp);
public:
    set<shared_ptr<MWP>, MWPPtrComp> upper_hull;
    Hull(const int &dimension, const bool &convexify);
    bool add_point(const shared_ptr<MWP> &mwp);
    void clear(const int &dimension_, const bool &convexify_);
    int size() const;
};
#endif //PROBABILISTC_HOARE_TRIPLES_HULL_H