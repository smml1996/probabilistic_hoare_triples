//
// Created by Stefanie Muroya Lei on 23.04.26.
//

#ifndef PROBABILISTC_HOARE_TRIPLES_HULL_H
#define PROBABILISTC_HOARE_TRIPLES_HULL_H

#include "utils.hpp"
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Convex_hull_3.h>
#include <CGAL/Epick_d.h>
#include <CGAL/convex_hull_d.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>




using K3 = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point3 = K3::Point_3;
using Polyhedron3 = CGAL::Polyhedron_3<K3>;

using KernelD = CGAL::Epick_d<CGAL::Dynamic_dimension_tag>;
using PointD = KernelD::Point_d;


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
    set<shared_ptr<MWP>, MWPPtrComp> upper_hull;
    vector<Point3> points3;
    Polyhedron3 poly3;

    bool update_pareto_front(const shared_ptr<MWP> &mwp);
    bool convex_add2(const shared_ptr<MWP> &mwp);
    bool convex_add3(const shared_ptr<MWP> &mwp);
    bool convex_add4(const shared_ptr<MWP> &mwp);
public:
    Hull() = default;
    Hull(const int &dimension, const bool &convexify);
    bool add_point(const shared_ptr<MWP> &mwp);
};
#endif //PROBABILISTC_HOARE_TRIPLES_HULL_H