#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include "experiments.hpp"

#include <cassert>

#include "solvers.hpp"
#include "utils.hpp"
using namespace  std;

void dump_pomdps() {
    auto file_path = results_path / "pomdps.csv";
    std::ofstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "Failed to open pomdps description file: " << file_path << "\n";
        assert(false);
    }

    vector<string> columns = {"benchmark","num_states",  "num_actions", "num_obs",  "num_initial_states"};
    file << join(columns, ",") << endl;
    for (auto pomdp_path_ : get_all_pomdp_names()) {
        cout << pomdp_path_ << endl;
        POMDP pomdp(pomdp_path_, POMDPFormat::ABHSVI);

        columns = {pomdp_path_, to_string(pomdp.states.size()),
            to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
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
    for (auto pomdp_name : get_final_f1_pomdp_names()) {
        cout << "running " << pomdp_name << "\n";

        POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
        int horizon = get_pomdp_horizon(pomdp_name);
        int n_states = pomdp.initial_states.size();

        ParetoSolver solver(pomdp, convexify);
        auto result = solver.solve(pomdp.initial_states, horizon);
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
        for (auto pomdp_name : get_final_f1_pomdp_names()) {
            POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
            int horizon = get_pomdp_horizon(pomdp_name);
            int n_states = pomdp.initial_states.size();
            cout << "running " << pomdp_name <<  " --hs=" << Hull::size_to_convexify << "\n";
            ParetoSolver solver(pomdp, convexify);
            auto result = solver.solve(pomdp.initial_states, horizon);
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
    results_file.close();
}

void generate_f1_benchmarks() {
    for (auto pomdp_name : f1_pomdps) {
        POMDP pomdp(pomdp_name, POMDPFormat::F1);
        for (auto horizon : horizons) {
            for (int n_states = 1; n_states <= 4; n_states++) {
                pomdp.to_abhsvi_format(horizon, n_states);
            }
        }
    }
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

vector<string> get_final_f1_pomdp_names() {
    vector<string> result;

    for (auto pomdp_name : f1_pomdps) {
        for (auto horizon : f1_horizons) {
            for (int n_states = 1; n_states <= 4; n_states++) {
                auto final_pomdp_name = pomdp_name  + "_" + to_string(horizon)+"_" + to_string(n_states);
                auto pomdp_path = abhsvi_benchmarks_path / final_pomdp_name;
                if (std::filesystem::exists(pomdp_path)) {
                    result.push_back(final_pomdp_name);
                }
            }
        }
    }

    return result;
}

int get_pomdp_horizon(const string &pomdp_name) {
    vector<string> tokens;
    split_str(pomdp_name, "_", tokens);
    assert(tokens.size() == 3);

    return stoi(tokens[1]);
}

vector<string> get_all_pomdp_names() {

    vector<string> result;

    for (auto name : abhsvi_pomdps) {
        result.push_back(name);
    }

    for (auto name : get_final_f1_pomdp_names()) {
        result.push_back(name);
    }
    return result;

}

