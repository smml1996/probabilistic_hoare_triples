#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include "experiments.hpp"

#include <cassert>

#include "solvers.hpp"
#include "utils.hpp"
using namespace  std;

int Benchmark::round_in_file = 6;

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
            for (int i_ = 0; i_ <= n_states; i_++) {
                initial_states.push_back(pomdp.states[i_]);
            }
            for (auto horizon : horizons) {
                cout << "running " << pomdp_name << " -- h=" << horizon << "\n";
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
        10, 50, 100
    };
    bool convexify = true;
    for (auto hull_size : convexify_sizes) {
        Hull::size_to_convexify = hull_size;
        // output file setup
        string name = "convexify_s" + to_string(Hull::size_to_convexify);
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
        for (auto pomdp_name : {"RockSample_POMDP_N3_G1_K4_R83.txt"}) {
            POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
            for (auto horizon : horizons) {

                cout << "running " << pomdp_name << " -- h=" << horizon  << " --hs=" << Hull::size_to_convexify << "\n";
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



Benchmark::Benchmark(const string &file) {
    auto pomdp_path = abhsvi_benchmarks_path / file;
    this->pomdp = POMDP();
    // load pomdp file
    ifstream pomdp_file(pomdp_path);

    if (!pomdp_file.is_open()) {
        std::cerr << "Failed to open pomdp file: " << pomdp_path << "\n";
        assert(false);
    }

    string line;

    while (getline(pomdp_file, line)) {
        if (line.size() > 0) {
            vector<string> tokens;
            split_str(line, ' ', tokens);
            if (tokens[0] == "states:") {
                int num_states;
                if (tokens.size() > 2) {
                    num_states = tokens.size() -1;
                } else {
                    num_states = stoi(tokens[1]);
                }
                for (int i = 0; i < num_states; i++) {
                    this->pomdp.states.push_back(make_shared<POMDPVertex>());
                }
            } else if (tokens[0] == "actions:") {
                int num_actions;
                bool is_list;
                if (tokens.size() > 2) {
                    num_actions = tokens.size() -1;
                    is_list = true;
                } else {
                    num_actions = stoi(tokens[1]);
                    is_list = false;
                }
                for (int i = 0; i < num_actions; i++) {
                    string name;
                    if (is_list) {
                        name = tokens[i+1];
                    }
                    this->pomdp.actions.push_back(make_shared<POMDPAction>(name));
                }
            } else if (tokens[0] == "observations:") {
                int num_obs;
                if (tokens.size() > 2) {
                    num_obs = tokens.size() -1;
                } else {
                    num_obs = stoi(tokens[1]);
                }
                assert(num_obs > 0);
                for (int i = 0; i < num_obs; i++) {
                    this->pomdp.observations.insert(i);
                }
            } else {
                vector<shared_ptr<POMDPAction>> actions;
                if (tokens[1] == "*") {
                    actions = pomdp.actions;
                } else {
                    actions.push_back(pomdp.get_action(tokens[1]));
                }

                if (line[0] == 'T') {
                    int from_bot = 0;
                    int from_top = this->pomdp.states.size() -1;
                    // state transition
                    int to_bot = 0;
                    int to_top = from_top;
                    if (tokens.size() == 7) {
                        if (tokens[3] != "*") {
                            from_bot = stoi(tokens[3]);
                            from_top = from_bot;
                        }

                        if (tokens[5] != "*") {
                            to_bot = stoi(tokens[5]);
                            to_top = to_bot;
                        }
                        double probability = stod(tokens[6]);
                        for (const auto& action : actions) {
                            for (int from_v = from_bot; from_v <= from_top; from_v++) {
                                for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                    this->pomdp.add_transition(action, from_v, to_v, probability);
                                }
                            }
                        }
                    } else if (tokens.size() == 4) {
                        if (tokens[3] != "*") {
                            from_bot = stoi(tokens[3]);
                            from_top = from_bot;
                        }

                        getline(pomdp_file, line);

                        vector<string> probs_toks;
                        split_str(line, ' ', probs_toks);

                        assert(probs_toks.size() == pomdp.states.size());

                        for (const auto& action : actions) {
                            for (int from_v = from_bot; from_v <= from_top; from_v++) {
                                for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                    double probability  = stod(probs_toks[to_v]);
                                    this->pomdp.add_transition(action, from_v, to_v, probability);
                                }
                            }
                        }
                    } else if (tokens.size() == 2) {
                        unordered_map<int, unordered_map<int, double>> temp_transitions;

                        for (int from_v = from_bot; from_v <= from_top; from_v++) {
                            getline(pomdp_file, line);
                            vector<string> probs_toks;
                            split_str(line, ' ', probs_toks);
                            assert(probs_toks.size() == pomdp.states.size());
                            for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                double probability  = stod(probs_toks[to_v]);
                                temp_transitions[from_v][to_v] = probability;
                            }
                        }

                        for (const auto& action : actions) {
                            for (int from_v = from_bot; from_v <= from_top; from_v++) {
                                for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                    double probability  = temp_transitions[from_v][to_v];
                                    this->pomdp.add_transition(action, from_v, to_v, probability);
                                }
                            }
                        }

                    } else {
                        assert(false);
                    }
                } else if (line[0] == 'O') {

                    int to_bot = 0;
                    int to_top = this->pomdp.states.size() -1;

                    // observation transition
                    int obs_bot = 0;
                    int obs_top = pomdp.observations.size()-1;
                    if (tokens.size() == 7) {
                        if (tokens[3] != "*") {
                            to_bot = stoi(tokens[3]);
                            to_top = to_bot;
                        }

                        if (tokens[5] != "*") {
                            obs_bot = stoi(tokens[5]);
                            obs_top = obs_bot;
                        }
                        double probability = stod(tokens[6]);
                        for (const auto& action : actions) {
                            for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                for (int obs = obs_bot; obs <= obs_top; obs++) {
                                    this->pomdp.add_obs_transition(action, to_v, obs, probability);
                                }
                            }
                        }
                    } else if (tokens.size() == 4) {
                        if (tokens[3] != "*") {
                            to_bot = stoi(tokens[3]);
                            to_top = to_bot;
                        }

                        getline(pomdp_file, line);

                        vector<string> probs_toks;
                        split_str(line, ' ', probs_toks);

                        assert(probs_toks.size() == pomdp.observations.size());

                        for (const auto& action : actions) {
                            for (int to_v = to_bot; to_v <= to_top; to_v++) {
                                for (int obs = obs_bot; obs <= obs_top; obs++) {
                                    double probability  = stod(probs_toks[obs]);
                                    this->pomdp.add_obs_transition(action, to_v, obs, probability);
                                }
                            }
                        }
                    } else if (tokens.size() == 2) {
                        unordered_map<int, unordered_map<int, double>> temp_transitions;

                        for (int from_v = to_bot; from_v <= to_top; from_v++) {
                            getline(pomdp_file, line);
                            vector<string> probs_toks;
                            split_str(line, ' ', probs_toks);
                            assert(probs_toks.size() == pomdp.observations.size());
                            for (int obs = obs_bot; obs <= obs_top; obs++) {
                                double probability  = stod(probs_toks[obs]);
                                temp_transitions[from_v][obs] = probability;
                            }
                        }

                        for (const auto& action : actions) {
                            for (int from_v = to_bot; from_v <= to_top; from_v++) {
                                for (int obs = obs_bot; obs <= obs_top; obs++) {
                                    double probability  = temp_transitions[from_v][obs];
                                    this->pomdp.add_obs_transition(action, from_v, obs, probability);
                                }
                            }
                        }

                    } else {
                        assert(false);
                    }
                }
            }
        }
    }
}

