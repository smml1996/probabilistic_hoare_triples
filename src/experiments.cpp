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

    vector<string> columns = {"benchmark","num_states",  "num_actions", "num_obs", "num_initial_states"};
    file << join(columns, ",") << endl;
    for (auto pomdp_path_ : abhsvi_pomdps) {
        cout << pomdp_path_ << endl;
        POMDP pomdp(pomdp_path_, POMDPFormat::ABHSVI);

        columns = {pomdp_path_, to_string(pomdp.states.size()),
            to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
        file << join(columns, ",") << endl;
    }

    {
        int n = 3;
        for (auto dt : {0.3, 0.5, 0.75}) {
            int k = ceil(n*n*dt);
            for (auto dg : {0.25, 0.5, 0.75}) {
                int g = ceil(dg*k);
                int r = k+g;
                auto pomdp_name = "RockSample_POMDP_N" + to_string(n) +"_G" + to_string(g) + "_K" + to_string(k) + "_R" + to_string(r) +"_.txt";
                POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);

                columns = {pomdp_name, to_string(pomdp.states.size()),
                    to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
                file << join(columns, ",") << endl;
            }
        }
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
        name = "abhsvi";
        convexify = false;
    } else {
        assert(method == MethodType::ConvexPareto);
        name = "convex_abhsvi";
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
        "set_size",
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

void run_exp_more_rocks(const MethodType &method) {
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
        "set_size",
        "val"
    }), ",") << "\n";
    int n = 3;
    for (auto dt : {0.3, 0.5, 0.75}) {
        int k = ceil(n*n*dt);
        for (auto dg : {0.25, 0.5, 0.75}) {
            int g = ceil(dg*k);
            int r = k+g;
            auto pomdp_name = "RockSample_POMDP_N" + to_string(n) +"_G" + to_string(g) + "_K" + to_string(k) + "_R" + to_string(r) +"_.txt";
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
    }
    results_file.close();
}

void f1_run_experiments(const MethodType &method, const string &pomdp_name) {
    string name;
    bool convexify;
    if (method == MethodType::Pareto) {
        name = "f1_"+pomdp_name;
        convexify = false;
    } else {
        assert(method == MethodType::ConvexPareto);
        name = "convex_f1_"+pomdp_name;
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
        "set_size",
        "n_initial_states",
        "val"
    }), ",") << "\n";
    cout << "running " << pomdp_name << "\n";

    for (auto horizon: f1_horizons) {
        POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
        ParetoSolver solver(pomdp, convexify);
        auto result = solver.solve(pomdp.initial_states, horizon);
        results_file << join(vector<string>({
            pomdp_name,
            to_string(horizon),
            to_string(round_to(solver.running_time, round_in_file)),
            to_string(solver.final_hull_size),
            to_string(pomdp.initial_states.size()),
            to_string(round_to(result, round_in_file)),
        }), ",") << "\n";
        results_file.flush();

    }


    results_file.close();
}

void run_convexify_sizes_experiment(const string &pomdp_name) {
    cout << "convexify: " << pomdp_name << "\n";
    vector<int> convexify_sizes = {
        25, 50, 500, 1000, 2000
    };
    bool convexify = true;

    // output file setup
    string name = "f1_convexify_" + pomdp_name;
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
        "final_set_size",
        "size_to_convexify",
        "n_initial_states",
        "val"
    }), ",") << "\n";

    for (auto horizon : f1_horizons) {
        for (auto hull_size : convexify_sizes) {
            Hull::size_to_convexify = hull_size;
            cout << "initial states: " << endl;
            POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);

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

    POMDP pomdp("iff.POMDP", POMDPFormat::F1);
    unordered_map<int, unordered_map<int, int>> map_specs;

    for (int d = 1; d <= 2; d++) {
        map_specs[d] = unordered_map<int, int>();
    }

    map_specs[1][1] = 54;
    map_specs[1][2] = 56;
    map_specs[1][3] = 58;


    map_specs[2][1] = 59;
    map_specs[2][2] = 61;
    map_specs[2][3] = 63;

    for (int d = 1; d <= 2; d++) {
        for (int v=1; v <= 3; v++) {
            for (int d2 = d; d2 <= 2; d2++) {
                for (int v2 = v; v2 <= 3; v2++) {
                    if (d!= d2 or v!= v2) {
                        int distance = (d-d2)*(d-d2) + (v-v2)*(v-v2);
                        pomdp.to_abhsvi_format({map_specs[d][v], map_specs[d2][v2]}, distance);
                    }
                }
            }

        }
    }

     // *********
    {
        POMDP pomdp("cit.POMDP", POMDPFormat::F1);
        pomdp.to_abhsvi_format({53, 73}, 1, true);
    }

    {
        POMDP pomdp("mit.POMDP", POMDPFormat::F1);
        pomdp.to_abhsvi_format({174, 190}, 1, true);
    }

    {
        POMDP pomdp("pentagon.POMDP", POMDPFormat::F1);
        pomdp.to_abhsvi_format({41, 173}, 1, true);
    }

    {
        POMDP pomdp("sunysb.POMDP", POMDPFormat::F1);
        pomdp.to_abhsvi_format({183, 227}, 1, true);
    }




}

void pomdps_to_python() {

    for (auto pomdp_name : f1_pomdps) {
        POMDP pomdp(pomdp_name, POMDPFormat::F1);
        auto pomdp_path = fs::path("..")/ "python_code" / pomdp_name;
        pomdp.to_python_code(pomdp_path);
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
    assert(false); // TODO: fix
    vector<string> result;
    for (auto horizon : f1_horizons) {
        for (int n_states = 1; n_states <= 4; n_states++) {
            for (auto pomdp_name : f1_pomdps) {
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

