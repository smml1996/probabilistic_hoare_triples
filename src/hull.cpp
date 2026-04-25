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




bool Hull::convex_add3(const shared_ptr<MWP> &mwp) {
    if (this->update_pareto_front(mwp)) {
        Point3 current_point(mwp->get(0), mwp->get(1), mwp->get(2));
        if (this->poly3.size_of_vertices() < 3 || this->poly3.size_of_facets()  == 0) {
            points3.push_back(current_point);
            CGAL::convex_hull_3(this->points3.begin(), this->points3.end(), this->poly3);
            return true;
        }
        if (this->is_upper_hull(mwp)) { // check if it affects upper hull
            points3.push_back(current_point);
            CGAL::convex_hull_3(this->points3.begin(), this->points3.end(), this->poly3);
            unordered_map<Polyhedron3::Vertex_handle, Flags> vertex_flags;
            for (auto f = this->poly3.facets_begin(); f != this->poly3.facets_end(); ++f) {
                auto h = f->facet_begin();

                auto v0 = h->vertex(); ++h;
                auto v1 = h->vertex(); ++h;
                auto v2 = h->vertex();

                auto normal = CGAL::cross_product(
                    v1->point() - v0->point(),
                    v2->point() - v0->point()
                );

                double nz = normal.z();

                for (auto v : {v0, v1, v2}) {
                    if (nz > 0) {
                        vertex_flags[v].has_upper = true;
                    } else if (nz < 0) {
                        vertex_flags[v].has_lower = true;
                    }
                }
            }
            int count_removed = 0;
            points3.clear();
            for (auto& [v, flags] : vertex_flags) {
                shared_ptr<MWP> current_mwp = make_shared<MWP>(3);

                current_mwp->values[0] = MyFloat(CGAL::to_double(v->point().x()));
                current_mwp->values[1] = MyFloat(CGAL::to_double(v->point().y()));
                current_mwp->values[2] = MyFloat(CGAL::to_double(v->point().z()));

                if (flags.has_upper) {
                    points3.push_back(v->point());
                    assert(this->upper_hull.find(current_mwp) != this->upper_hull.end());
                } else if (flags.has_lower) {
                    auto temp_p = v->point();
                    count_removed++;
                    this->upper_hull.erase(current_mwp);
                }
            }
            if (count_removed > 0) {
                this->poly3.clear();
                CGAL::convex_hull_3(this->points3.begin(), this->points3.end(), this->poly3);
            }
            return true;
        }

        this->upper_hull.erase(mwp);
        return false;
    }
    return false;
}

bool Hull::convex_add4(const shared_ptr<MWP> &mwp) {
    if (this->update_pareto_front(mwp)) {
        array<double, 4> coords = {mwp->get(0), mwp->get(1), mwp->get(2), mwp->get(3)};
        PointD current_point (4, coords.begin(), coords.end());
        if (this->pointsD.size() < 4 || this->poly4.number_of_facets() == 0) {

            pointsD.emplace_back(4, coords.begin(), coords.end());
            this->poly4.insert(pointsD.back());
            return true;
        }
        if (this->is_upper_hull(mwp)) { // check if it affects upper hull
            pointsD.push_back(current_point);
            this->poly4.insert(pointsD.back());
            this->pointsD.clear();
            for (auto it = this->poly4.facets_begin(); it != this->poly4.facets_end(); ++it) {
                // Get the hyperplane equation of the facet
                typename KernelD::Hyperplane_d hyperplane = this->poly4.hyperplane_supporting(it);

                // In CGAL's Convex_hull_d, the hyperplane coefficients
                // are stored such that the outward normal is accessible.
                // For a 4D point (x, y, z, w), the coefficients are [0, 1, 2, 3, 4]
                // where 4 is the coefficient for the last dimension (w).

                if (hyperplane.coefficient(3) > 0) { // Index 3 is the 4th coord (w)
                    // This facet is part of the Upper Hull
                    // You can now access the vertices of this facet
                    for (int i = 0; i < 4; ++i) {
                        auto v_handle = this->poly4.vertex_of_facet(it, i);
                        auto p = this->poly4.associated_point(v_handle);
                        double x = CGAL::to_double(p.cartesian((0)));
                        double y = CGAL::to_double(p.cartesian((1)));
                        double z = CGAL::to_double(p.cartesian((2)));
                        double w = CGAL::to_double(p.cartesian((3)));
                        array<double, 4> coords2 = {x,y,z,w};
                        this->pointsD.emplace_back(4, coords2.begin(), coords2.end());
                    }
                }
            }

            if (this->pointsD.size() != this->upper_hull.size()) {
                this->upper_hull.clear();
                for (auto p : this->pointsD) {
                    shared_ptr<MWP> current_mwp = make_shared<MWP>(4);
                    current_mwp->values[0] = MyFloat(CGAL::to_double(p.cartesian((0))));
                    current_mwp->values[1] = MyFloat(CGAL::to_double(p.cartesian((1))));
                    current_mwp->values[2] = MyFloat(CGAL::to_double(p.cartesian((2))));
                    current_mwp->values[3] = MyFloat(CGAL::to_double(p.cartesian((3))));
                    this->upper_hull.insert(current_mwp);
                }
                this->poly4.clear(4);
                this->poly4.insert(pointsD.begin(), pointsD.end());
            }
        }

        this->upper_hull.erase(mwp);
        return false;
    }
    return false;
}

bool Hull::is_upper_hull(const shared_ptr<MWP> &mwp) {
    if (this->dimension == 3) {
        Point3 p(mwp->get(0), mwp->get(1), mwp->get(2));
        for (auto f = this->poly3.facets_begin(); f != this->poly3.facets_end(); ++f) {
            auto h = f->facet_begin();

            Point3 A = h->vertex()->point(); ++h;
            Point3 B = h->vertex()->point(); ++h;
            Point3 C = h->vertex()->point();

            auto normal = CGAL::cross_product(B - A, C - A);

            // consider only upper faces
            if (normal.z() <= 0) continue;

            if (CGAL::orientation(A, B, C, p) == CGAL::POSITIVE) {
                return true;  // p is outside and affects upper hull
            }
        }
        return false; // p is inside or below upper hull
    } else {
        assert(this->dimension == 4);
        array<double, 4> coords = {mwp->get(0), mwp->get(1), mwp->get(2), mwp->get(3)};
        PointD p(4, coords.begin(), coords.end());

        for (auto fit = this->poly4.facets_begin(); fit != this->poly4.facets_end(); ++fit) {
            KernelD::Hyperplane_d h = this->poly4.hyperplane_supporting(fit);

            // Only check facets that belong to the Upper Hull (w-coefficient > 0)
            if (h.coefficient(3) > 0) {
                // Check if point p is on the positive side of this hyperplane
                // 'oriented_side' returns ON_POSITIVE_SIDE, ON_NEGATIVE_SIDE, or ON_ORIENTED_BOUNDARY
                if (h.oriented_side(p) == CGAL::ON_POSITIVE_SIDE) {
                    return true; // It's outside at least one upper facet!
                }
            }
        }
        return false; // It's under all upper facets
    }
}

Hull::Hull(const int &dimension, const bool &convexify) : poly4(4){
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

void Hull::clear(const int &dimension_, const bool &convexify_) {
    this->dimension = dimension_;
    this->convexify = convexify_;
    this->upper_hull.clear();
    this->points3.clear();
    this->pointsD.clear();

    this->poly3.clear();
    this->poly4.clear(4);
}

int Hull::size() const {
    return this->upper_hull.size();
}
