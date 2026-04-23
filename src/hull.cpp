//
// Created by Stefanie Muroya Lei on 23.04.26.
//

#include "hull.hpp"

MWP::MWP(const int &size) {
    for (int i = 0; i <size; i++) {
        this->values.push_back(zero);
    }
}

double MWP::get(const int &index) {
    if (index >= values.size()) {
        throw std::out_of_range("index out of range");
    }

    return this->values[index].value;

}

bool MWP::operator==(const MWP &other) {
    assert(this->size() == other.size());
    for (int i = 0; i < this->size(); i++) {
        if (this->values[i] != other.values[i]) {
            return false;
        }
    }
    return true;
}

bool Hull::update_pareto_front(const shared_ptr<MWP> &mwp) {

    unordered_set<shared_ptr<MWP>> to_remove;
    for (auto p : this->upper_hull) {
        if (*mwp <= *p) {
            assert(to_remove.empty());
            return false;
        }

        if (*p <= *mwp) {
            to_remove.insert(p);
        }
    }

    this->upper_hull.insert(mwp);
    for (auto r_strat : to_remove) {
        this->upper_hull.erase(r_strat);
    }
    return true;

}

bool Hull::convex_add2(const shared_ptr<MWP> &mwp) {
    if (this->update_pareto_front(mwp)) {
        // convexify pareto front
        auto it = this->upper_hull.find(mwp);
        while (true) {
            if (it == this->upper_hull.begin()) break;

            auto b = it;
            auto a = std::prev(b);
            if (a == this->upper_hull.begin()) break;

            auto a2 = std::prev(a);

            auto ax = (*a)->values[0];
            auto ay = (*a)->values[1];
            auto a2x = (*a2)->values[0];
            auto a2y = (*a2)->values[1];

            auto bx = (*b)->values[0];
            auto by = (*b)->values[1];

            auto cross = (ax - a2x) * (by - a2y) - (ay - a2y) * (bx - a2x);
            if (cross == zero || cross > zero ) {
                this->upper_hull.erase(a); // remove middle point
            } else break;
        }
        return true;
    }
    return false;
}


// for (auto f = hull.facets_begin(); f != hull.facets_end(); ++f) {
//     auto h = f->facet_begin();
//
//     const Point& A = h->vertex()->point(); ++h;
//     const Point& B = h->vertex()->point(); ++h;
//     const Point& C = h->vertex()->point();
//
//     // vectors
//     auto AB = B - A;
//     auto AC = C - A;
//
//     // normal = AB x AC
//     auto normal = CGAL::cross_product(AB, AC);
//
//     if (normal.z() > 0) {
//         // ✅ this is an upper face
//     }
// }

bool Hull::convex_add3(const shared_ptr<MWP> &mwp) {
    if (this->update_pareto_front(mwp)) {
        Point3 current_point(mwp->get(0), mwp->get(1), mwp->get(2));
        if (this->poly3.size_of_vertices() == 0) {
            points3.push_back(current_point);
            CGAL::convex_hull_3(this->points3.begin(), this->points3.end(), this->poly3);
            return true;
        }
        if (this->is_upper_hull(mwp)) { // check if it affects upper hull

        }
        return false;
    }
    return false;
}

Hull::Hull(const int &dimension, const bool &convexify) {
    this->dimension = dimension;
    this->convexify = convexify;
}

bool Hull::add_point(const shared_ptr<MWP> &mwp) {
    if (this->convexify) {
        if (this->dimension == 2) {
            return this->convex_add2(mwp);
        }
        if (this->dimension == 3) {
            return this->convex_add3(mwp);
        }
        return this->convex_add4(mwp);
    }
    return this->update_pareto_front(mwp);
}
