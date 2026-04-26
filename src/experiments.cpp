#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include "experiments.hpp"

#include <cassert>

#include "solvers.hpp"
#include "utils.hpp"
using namespace  std;

std::string join(const std::vector<std::string>& parts, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << parts[i];
    }
    return oss.str();
}

void dump_pomdps() {
    auto file_path = results_path / "pomdps.csv";
    std::ofstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open pomdps description file: " << file_path << "\n";
        assert(false);
    }

    vector<string> columns = {"benchmark","num_states",  "num_actions", "num_obs",  "num_initial_states"};
    file << join(columns, ",") << endl;
    for (auto pomdp_path_ : abhsvi_pomdps) {
        cout << pomdp_path_ << endl;
        POMDP pomdp(pomdp_path_, POMDPFormat::ABHSVI);

        columns = {pomdp_path_, to_string(pomdp.states.size()),
            to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
        file << join(columns, ",") << endl;
    }

    for (auto pomdp_path_ : f1_pomdps) {
        cout << pomdp_path_ << endl;
        POMDP pomdp(pomdp_path_, POMDPFormat::F1);

        columns = {pomdp_path_, to_string(pomdp.states.size()),
            to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), "-"};
        file << join(columns, ",") << endl;
    }

    file.close();
}

void run_experiments(const MethodType &method) {
    string name;
    bool convexify;
    if (method == MethodType::Pareto) {
        name = "pareto";
        convexify = false;
    } else {
        assert(method == MethodType::ConvexPareto);
        name = "convex_pareto";
        convexify = true;
    }

    // output file setup
    fs::path f_results_path = results_path / (name + ".csv");
    std::ofstream results_file(f_results_path);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open results file: " << f_results_path << "\n";
        return;
    }

    // write header in output file
    results_file << join(vector<string>({
        "benchmark",
        "horizon",
        "time",
        "hull_size",
        "val"
    }), ",") << "\n";
    for (auto pomdp_name : abhsvi_pomdps) {
        POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
        for (auto horizon : horizons) {
            cout << "running " << pomdp_name << " -- h=" << horizon << "\n";
            ParetoSolver solver(pomdp, convexify);
            auto result = solver.solve(pomdp.initial_states, horizon);
            results_file << join(vector<string>({
                pomdp_name,
                to_string(horizon),
                to_string(round_to(solver.running_time, round_in_file)),
                to_string(solver.final_hull_size),
                to_string(round_to(result, round_in_file)),
            }), ",") << "\n";
            results_file.flush();
        }
    }
    results_file.close();
}

void f1_run_experiments(const MethodType &method) {
    string name;
    bool convexify;
    if (method == MethodType::Pareto) {
        name = "f1_pareto";
        convexify = false;
    } else {
        assert(method == MethodType::ConvexPareto);
        name = "f1_convex_pareto";
        convexify = true;
    }

    // output file setup
    fs::path f_results_path = results_path / (name + ".csv");
    std::ofstream results_file(f_results_path);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open results file: " << f_results_path << "\n";
        return;
    }

    // write header in output file
    results_file << join(vector<string>({
        "benchmark",
        "horizon",
        "time",
        "hull_size",
        "n_initial_states",
        "val"
    }), ",") << "\n";
    for (auto pomdp_name : f1_pomdps) {
        POMDP pomdp(pomdp_name, POMDPFormat::F1);
        for (int n_states = 1; n_states <= 4; n_states++) {
            vector<shared_ptr<POMDPVertex>> initial_states;
            for (int i_ = 0; i_ < n_states; i_++) {
                initial_states.push_back(pomdp.states[i_]);
            }
            for (auto horizon : horizons) {
                cout << "running " << pomdp_name << " -- h=" << horizon << " n=" << n_states << "\n";
                ParetoSolver solver(pomdp, convexify);
                auto result = solver.solve(initial_states, horizon);
                results_file << join(vector<string>({
                    pomdp_name,
                    to_string(horizon),
                    to_string(round_to(solver.running_time, round_in_file)),
                    to_string(solver.final_hull_size),
                    to_string(n_states),
                    to_string(round_to(result, round_in_file)),
                }), ",") << "\n";
                results_file.flush();
            }
        }
    }
    results_file.close();
}

void run_convexify_sizes_experiment() {
    vector<int> convexify_sizes = {
        50, 100, 500, 1000
    };
    bool convexify = true;

    // output file setup
    string name = "f1_convexify";
    fs::path f_results_path = results_path / (name + ".csv");
    std::ofstream results_file(f_results_path);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open results file: " << f_results_path << "\n";
        return;
    }

    // write header in output file
    results_file << join(vector<string>({
        "benchmark",
        "horizon",
        "time",
        "final_hull_size",
        "size_to_convexify",
        "n_initial_states",
        "val"
    }), ",") << "\n";

    for (auto hull_size : convexify_sizes) {
        Hull::size_to_convexify = hull_size;
        for (auto pomdp_name : f1_pomdps) {
            POMDP pomdp(pomdp_name, POMDPFormat::F1);
            for (auto horizon : horizons) {
                for (int n_states = 1; n_states <= 4; n_states++) {
                    vector<shared_ptr<POMDPVertex>> initial_states;
                    for (int i_ = 0; i_ < n_states; i_++) {
                        initial_states.push_back(pomdp.states[i_]);
                    }
                    cout << "running " << pomdp_name << " -- h=" << horizon  << " --hs=" << Hull::size_to_convexify << "\n";
                    ParetoSolver solver(pomdp, convexify);
                    auto result = solver.solve(initial_states, horizon);
                    results_file << join(vector<string>({
                        pomdp_name,
                        to_string(horizon),
                        to_string(round_to(solver.running_time, round_in_file)),
                        to_string(solver.final_hull_size),
                        to_string(Hull::size_to_convexify),
                        to_string(n_states),
                        to_string(round_to(result, round_in_file)),
                    }), ",") << "\n";
                    results_file.flush();
                }
            }
        }
    }
    results_file.close();
}

string methods_to_string(const set<MethodType> &methods) {
    string result;
    for (auto m : methods) {
        if (!result.empty()) {
            result += ", ";
        }
        result += method_to_string(m);
    }
    return result;
}

set<string> get_solver_methods_strings() {
    set<string> solver_methods;
    for (int i = 0; i < MethodType::MethodCount; i++) {
        solver_methods.insert(method_to_string(static_cast<MethodType>(i)));
    }

    return solver_methods;
}

string method_to_string(const MethodType &method) {
    switch(method) {
        case MethodType::Pareto:
            return "pareto";
        case MethodType::ConvexPareto:
            return "convex pareto";
        default:
            assert(false);
    }
}

MethodType str_to_method_type(const string &method) {
    for (int i = 0; i < MethodType::MethodCount; i++) {
        string m_str = method_to_string(static_cast<MethodType>(i));
        if (m_str == method) {
            return static_cast<MethodType>(i);
        }
    }
    throw invalid_argument("Method type not recognized: " + method);
}

