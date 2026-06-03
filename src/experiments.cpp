#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include "experiments.hpp"
#include <cassert>
#include "solvers.hpp"
#include "utils.hpp"
using namespace  std;

long long QuantumExperiment::timelimit = 3600;

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

        columns = {pomdp_path_, to_string(pomdp.get_reachable(7)),
            to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
        file << join(columns, ",") << endl;
    }

    {
        for (auto dt : {0.3, 0.5, 0.75}) {
            int n = 3;
            int k = ceil(n*n*dt);
            for (auto dg : {0.25, 0.5, 0.75}) {
                int g = ceil(dg*k);
                int r = k+g;
                auto pomdp_name = "RockSample_POMDP_N" + to_string(n) +"_G" + to_string(g) + "_K" + to_string(k) + "_R" + to_string(r) +"_.txt";
                POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);

                columns = {pomdp_name, to_string(pomdp.get_reachable(7)),
                    to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
                file << join(columns, ",") << endl;
            }
        }
    }

    {
        // iff
        vector<int> visibilities = {0, 2, 4};
        for (int d1 = 1; d1 < 4; d1 ++) {
            for (int d2 = d1+1; d2 < 4; d2++) {
                for (auto v1: visibilities) {
                    for (auto v2 : visibilities) {
                        if (v1 != v2) {
                            auto pomdp_name = "iff_" + to_string(d1) +"_" + to_string(d2) + "_" + to_string(v1) + "_" +to_string(v2);
                            POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);

                            columns = {pomdp_name, to_string(pomdp.get_reachable(6)),
                                to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
                            file << join(columns, ",") << endl;

                        }
                    }
                }
            }
        }
    }

    {
        // robot
        vector<string> names = {"cit.POMDP", "mit.POMDP", "pentagon.POMDP", "sunysb.POMDP"};
        for (auto name : names) {
            for (int d = 1; d < 4; d++) {
                string pomdp_name = name + "_" + to_string(d);
                POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);

                columns = {pomdp_name, to_string(pomdp.get_reachable(6)),
                    to_string(pomdp.actions.size()), to_string(pomdp.observations.size()), to_string(pomdp.initial_states.size())};
                file << join(columns, ",") << endl;
            }
        }
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
                to_string(round_to(result.second, round_in_file)),
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
    for (auto dt : {0.3, 0.5, 0.75}) {
        int n = 3;
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
                    to_string(round_to(result.second, round_in_file)),
                }), ",") << "\n";
                results_file.flush();
            }
        }
    }
    results_file.close();
}

void f1_run_experiments(const MethodType &method, const string &pomdp_name, const int &max_horizon) {
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

    for (int horizon = 1; horizon <= max_horizon; ++horizon) {
        POMDP pomdp(pomdp_name, POMDPFormat::ABHSVI);
        ParetoSolver solver(pomdp, convexify);
        auto result = solver.solve(pomdp.initial_states, horizon);
        results_file << join(vector<string>({
            pomdp_name,
            to_string(horizon),
            to_string(round_to(solver.running_time, round_in_file)),
            to_string(solver.final_hull_size),
            to_string(pomdp.initial_states.size()),
            to_string(round_to(result.second, round_in_file)),
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
            bool convexify = true;
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
                to_string(round_to(result.second, round_in_file)),
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
        POMDP pomdp1("cit.POMDP", POMDPFormat::F1);
        pomdp1.to_abhsvi_format({53, 73}, 1, true);
    }

    {
        POMDP pomdp1("mit.POMDP", POMDPFormat::F1);
        pomdp1.to_abhsvi_format({174, 190}, 1, true);
    }

    {
        POMDP pomdp1("pentagon.POMDP", POMDPFormat::F1);
        pomdp1.to_abhsvi_format({41, 173}, 1, true);
    }

    {
        POMDP pomdp1("sunysb.POMDP", POMDPFormat::F1);
        pomdp1.to_abhsvi_format({183, 227}, 1, true);
    }
}

void pomdps_to_python() {

    for (auto pomdp_name : f1_pomdps) {
        POMDP pomdp(pomdp_name, POMDPFormat::F1);
        auto pomdp_path = fs::path("..")/ "python_code" / pomdp_name;
        pomdp.to_python_code(pomdp_path);
    }
}

string to_string(const set<MethodType> &methods) {
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

void QuantumExperiment::check_time() {
    auto now = chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::duration<double>>(now-this->start_time).count() > Solver::timelimit) {
        this->is_timeout = true;
    }
}

// Quantum experiments
fs::path QuantumExperiment::get_wd() const {
    return  fs::path("..") / "results" / this->name;
}

bool QuantumExperiment::clean_wd() const {
    auto folder = this->get_wd() / "algorithms";
    try {
        std::uintmax_t count = fs::remove_all(folder);

        std::cout << "Deleted " << count << " files from " << folder << endl;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    folder = this->get_wd() / "raw_algorithms";
    try {
        std::uintmax_t count = fs::remove_all(folder);

        std::cout << "Deleted " << count << " files from " << folder << endl;

    }
    catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    auto f = this->get_wd() / "stats.csv";
    try {
        std::uintmax_t count = fs::remove(f);

        std::cout << "Deleted " << count << " stats file " << f << endl;

    }
    catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

bool QuantumExperiment::setup_working_dir(const bool &clean_wd) const {
    fs::path dir_path = this->get_wd();

    if (!fs::exists(dir_path)) {
        if (fs::create_directories(dir_path)) {
            std::cout << "main experiments directory created successfully.\n";
        } else {
            std::cerr << "Failed to create main experiments directory.\n";
            return false;
        }
    }

    if (clean_wd) {
        assert(this->clean_wd());
    }

    dir_path = this->get_wd() / "algorithms";

    if (!fs::exists(dir_path)) {
        if (fs::create_directories(dir_path)) {
            std::cout << "Algorithms directory created successfully.\n";
        } else {
            std::cerr << "Failed to create directory for storing algorithms.\n";
            return false;
        }
    }


    dir_path = this->get_wd() / "raw_algorithms";

    if (!fs::exists(dir_path)) {
        if (fs::create_directories(dir_path)) {
            std::cout << "Algorithms directory created successfully.\n";
        } else {
            std::cerr << "Failed to create directory for storing algorithms.\n";
            return false;
        }
    }
    return true;
}

int QuantumExperiment::get_or_add_algorithm(vector<shared_ptr<MixedStrategy>> &unique_algorithms,
        const shared_ptr<MixedStrategy> &new_algorithm) const {
    int index = 0;
    for (auto algorithm : unique_algorithms) {
        if (*algorithm == *new_algorithm) {
            return index;
        }
        index++;
    }

    assert(index == unique_algorithms.size());

    fs::path algorithms_folder = this->get_wd() / "algorithms";

    unique_algorithms.push_back(new_algorithm); // add new algorithm
    fs::path algorithm_path = algorithms_folder / ("A_" + to_string(index) + ".txt");
    assert(new_algorithm->dump(algorithm_path));

    fs::path raw_algorithms_folder = this->get_wd() / "raw_algorithms";
    fs::path raw_algorithm_path = raw_algorithms_folder / ("R_" + to_string(index) + ".txt");
    assert(new_algorithm->dump_raw(raw_algorithm_path));

    return index;
}

POMDP QuantumExperiment::build_pomdp(HardwareSpecification &hardware_specification, const vector<shared_ptr<const QAction>> &actions) {
    QPOMDP pomdp;
    for (auto element : actions) {
        pomdp.actions.push_back(element);
    }

    queue<pair<shared_ptr<const QVertex>, int>> q;
    auto initial_states = this->get_initial_states();
    for (auto state : initial_states) {
        auto r = pomdp.create_new_vertex(state);
        pomdp.initial_states.push_back(r);
        pomdp.initial_states.push_back(r);
        pomdp.observations.insert(r->classical_state()->get_memory_val());
        q.push(make_pair(r, 0));
    }


    unordered_set<shared_ptr<const POMDPVertex>, POMDPVertexHash, POMDPVertexPtrEqual> visited; // equality in terms of hybrid states is handled below

    while (!q.empty()) {
        auto temp = q.front();
        q.pop();
        auto current_v = temp.first;
        auto current_horizon = temp.second;
        if (max_horizon != -1) {
            if (current_horizon == max_horizon) {
                continue;
            }
        }

        if (visited.find(current_v) != visited.end()) {
            continue;
        }

        visited.insert(current_v);


        // reward function
        pomdp.add_reward(POMDPAction::HALT_ACTION, current_v, this->get_reward(current_v));

        for (auto action : actions) {
            if (guard(current_v, action)) {
                auto successors = action->get_successor_states(hardware_specification, current_v);
                assert(!successors.values.empty());
                for (auto it : successors.values ) {
                    auto succ = it.first;
                    auto prob = it.second;
                    assert(succ!= nullptr);
                    assert(succ->quantum_state != nullptr);
                    assert(succ->classical_state != nullptr);
                    auto new_vertex = pomdp.create_new_vertex(succ); // handles equality of hybrid states

                    // transition probabilities
                    // pomdp.add_transition(action, current_v, new_vertex, prob);
                    pomdp.transition_matrix[current_v][action][new_vertex] = prob;
                    assert(pomdp.transition_matrix[current_v][action][new_vertex] == prob);

                    // observations
                    pomdp.observations.insert(succ->classical_state->get_memory_val());

                    // observations transition function
                    pomdp.add_obs_transition(action, new_vertex, succ->classical_state->get_memory_val(), MyFloat(1));
                    if (visited.find(new_vertex) == visited.end()) {
                        if (max_horizon == -1 || (current_horizon + 1 < max_horizon)) {
                            q.push(make_pair(new_vertex, current_horizon+1));
                        } else {
                            // LOG.write_debug_ln("Not explored vertex: " + new_vertex->str());
                        }
                    } else {
                        // LOG.write_debug_ln("Not explored vertex [already visited]: " + new_vertex->str());
                    }
                }
            }

        }

        if (this->is_timeout) {
            return pomdp;
        }
    }
    // pomdp.check();

    return pomdp;
}

vector<int> QuantumExperiment::get_unused(const Embedding &embedding, const int &n) const {
    unordered_set<int> current_set;
    for (auto e : embedding) {
        current_set.insert(e.second);
    }

    return this->get_unused(current_set, n);
}

vector<int> QuantumExperiment::get_unused(unordered_set<int> used_qubits, const int &n) {
    vector<int> result;

    int q = 0;

    while (result.size() < n) {
        if (used_qubits.find(q) == used_qubits.end()) {
            result.push_back(q);
        }
        q++;
    }

    return result;
}

shared_ptr<QuantumState> QuantumExperiment::get_choi_id_state(const vector<pair<int, int>> &qubit_pairs) const {
    shared_ptr<QuantumState> result = make_shared<QuantumState>(this->qubits_used);

    // create Choi state
    for (pair<int, int> p : qubit_pairs) {
        Instruction H(GateName::H, p.first);
        result = result->apply_instruction(H);
        Instruction CX(GateName::Cnot, vector<int>{p.first}, p.second);
        result = result->apply_instruction(CX);
    }

    return result;
}

bool QuantumExperiment::guard(const shared_ptr<const QVertex> &v, const shared_ptr<const QAction> &a) const {
    return true;
}

void QuantumExperiment::init() {
    this->set_experiment_name();
    this->set_thermalization();
    this->set_qubits_used();
    this->set_horizons();
    this->set_precision();
    this->set_optimize();
    this->set_method_types();
    this->set_quantum_hardware();
}

void QuantumExperiment::set_method_types() {
    this->method_types = {MethodType::Pareto};
}

void QuantumExperiment::set_quantum_hardware() {
    for(int i = 0; i < QuantumHardware::HardwareCount; i++)  {
        QuantumHardware qw = static_cast<QuantumHardware>(i);
        this->hardware_list.insert(qw);
    }
}

void QuantumExperiment::set_thermalization() {
    this->with_thermalization = false;
}

void QuantumExperiment::set_optimize() {
    this->optimize = true;
}

void QuantumExperiment::set_horizons() {
    this->min_horizon = 1;
    this->max_horizon = 7;
}

void QuantumExperiment::set_precision() {
    MyFloat::precision = 9;
}

void QuantumExperiment::run() {
    LOG.write_debug_ln("MAX HORIZON: " + to_string(this->max_horizon));
    this->dump_preview();

    assert(setup_working_dir());

    fs::path results_path_local = this->get_wd() / "stats.csv";

    // Open file for writing (this overwrites the file if it exists)
    std::ofstream results_file(results_path_local);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open results file: " << results_path_local << "\n";
        return;
    }

    // write header in results file
    results_file << join(vector<string>({"hardware",
        "embedding_index",
        "horizon",
        "pomdp_build_time",
        "probability",
        "solver_time",
        "algorithm_index",
        })
        , ",") << "\n";


    vector<shared_ptr<MixedStrategy>> unique_algorithms;

    for (auto [hardware_index, hardware] : enumerate(this->hardware_list)) {
        auto hardware_spec = HardwareSpecification(hardware, this->with_thermalization, this->optimize);
        auto embeddings = this->get_embeddings(hardware_spec);
        string hardware_name = hardware_spec.get_hardware_name();
        for (auto [embedding_index, embedding] : enumerate(embeddings)) {
            this->is_timeout = false;
            this->start_time = chrono::steady_clock::now();
            auto local_hardware_spec = hardware_spec.get_normalized(embedding);
            auto actions = this->get_actions(local_hardware_spec);
            auto pomdp = this->build_pomdp(local_hardware_spec, actions);
            auto end_pomdp_build = chrono::steady_clock::now();    // end time
            auto pomdp_build_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_pomdp_build - this->start_time).count();
            for (int horizon = this->min_horizon; horizon <= this->max_horizon; horizon++) {
                LOG.write_info_ln(to_string(hardware_spec.get_hardware()) + " " + to_string(hardware_index+1) + "/" + to_string(this->hardware_list.size()) + " -- " + to_string(embedding_index+1)
                    + "/" + to_string(embeddings.size()) + " -- k=" + to_string(horizon));
                for (auto method : this->method_types) {
                    if (! this->is_timeout) {
                        long long method_time;
                        pair<shared_ptr<MixedStrategy>, double> result;
                        if (method == MethodType::Pareto) {
                            bool convexify = false;
                            ParetoSolver solver(pomdp, convexify);
                            auto start_method = chrono::high_resolution_clock::now();
                            result = solver.solve(pomdp.initial_states, horizon);
                            auto end_method = chrono::high_resolution_clock::now();
                            method_time = chrono::duration<double>(end_method - start_method).count();
                        }
                        int algorithm_index = get_or_add_algorithm(unique_algorithms, result.first);
                        assert(algorithm_index >= 0);

                        results_file << join(vector<string>({hardware_name,
                                                        to_string(embedding_index),
                                                        to_string(horizon),
                                                        to_string(round_to(pomdp_build_time, round_in_file)),
                                                        to_string(round_to(result.second, round_in_file)),
                                                        to_string(round_to(method_time, round_in_file)),
                                                        to_string(algorithm_index)})
                                                        , ",") << "\n";
                        results_file.flush();
                    } else {
                        results_file << join(vector<string>({hardware_name,
                                                        to_string(embedding_index),
                                                        to_string(horizon),
                                                        to_string(round_to(pomdp_build_time, round_in_file)),
                                                        "-1",
                                                        "-1",
                                                        "-1"})
                                                        , ",") << "\n";
                    }

                }
            }
        }
    }

    results_file.close();

    LOG.write_debug_ln("TOTAL ALGORITHMS: " + to_string(unique_algorithms.size()));
    cout << "Done" << endl;
}

void QuantumExperiment::dump_preview() {
    assert(setup_working_dir(false));
    fs::path results_path_local = this->get_wd() / "preview.csv";

    // Open file for writing (this overwrites the file if it exists)
    std::ofstream results_file(results_path_local);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open results file: " << results_path_local << "\n";
        return;
    }

    results_file << "name: " << this->name << "\n";
    results_file << "optimize: " << this->optimize << "\n";
    results_file << "min. horizon: " << this->min_horizon << "\n";
    results_file << "max. horizon: " << this->max_horizon << "\n";
    results_file << "methods: " << to_string(this->method_types) << endl;
    results_file << "specs: " << to_string(this->hardware_list) << endl;
    results_file << "thermalization: " << this->with_thermalization << "\n";


    int average_pomdp_build_time = 0;
    int num_timeouts = 0;
    int total_embeddings = 0;
    int avg_num_instructions = 0;
    for (auto hardware : this->hardware_list) {
        auto hardware_spec = HardwareSpecification(hardware, this->with_thermalization, this->optimize);
        auto embeddings = this->get_embeddings(hardware_spec);
        total_embeddings += embeddings.size();
        for (int embedding_index = 0; embedding_index < embeddings.size(); embedding_index++) {
            this->is_timeout = false;
            auto embedding = embeddings[embedding_index];
            this->start_time = chrono::steady_clock::now();
            auto local_hardware_spec = hardware_spec.get_normalized(embedding);
            auto actions = this->get_actions(local_hardware_spec);
            avg_num_instructions += actions.size();
            LOG.write_info_ln(to_string(hardware_spec.get_hardware()) + " "
                + to_string(embedding_index+1) + "/" + to_string(embeddings.size())
                + " num_actions=" + to_string(actions.size()));
            auto pomdp = this->build_pomdp(local_hardware_spec, actions);
            auto end_pomdp_build = chrono::steady_clock::now();    // end time
            auto pomdp_build_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_pomdp_build - this->start_time).count();
            LOG.write_info_ln("POMDP BUILD TIME: " + to_string(pomdp_build_time));

            if (this->is_timeout) {
                num_timeouts++;
            } else {
                average_pomdp_build_time += pomdp_build_time;
            }

        }
    }
    average_pomdp_build_time /= total_embeddings;
    avg_num_instructions /= total_embeddings;

    results_file << join(vector<string>({
        "total_embeddings",
        to_string(total_embeddings)
        })
        , ": ") << "\n";

    results_file << join(vector<string>({
        "avg_num_instructions",
        to_string(avg_num_instructions)
        })
        , ": ") << "\n";

    results_file << join(vector<string>({
        "num_timeouts",
        to_string(num_timeouts)
        })
        , ": ") << "\n";

    results_file << join(vector<string>({
        "average_pomdp_build_time",
        to_string(average_pomdp_build_time)
        })
        , ": ") << "\n";

    results_file.close();
    cout << "Done" << endl;
}


ReadoutNoise::ReadoutNoise(int target, double success0, double success1) {
    this->target = target;
    this->success0 = success0;
    this->success1 = success1;
    this->diff = success0 - success1;
    this->acc_err = 1-success0 + 1-success1;
    this->abs_diff = abs(success0-success1);
}

set<int> get_meas_pivot_qubits(const HardwareSpecification &hardware_spec, const int &min_indegree) {
    if (hardware_spec.get_hardware() == QuantumHardware::PerfectHardware) {
        return {0};
    }
    set<int> result;
    vector<ReadoutNoise> noises;

    for (int qubit = 0; qubit < hardware_spec.num_qubits; qubit++) {
        if (hardware_spec.get_qubit_indegree(qubit) >= min_indegree) {
            auto instruction = make_shared<Instruction>(GateName::Meas, qubit, qubit);
            shared_ptr<MeasurementChannel> noise_data = static_pointer_cast<MeasurementChannel>(hardware_spec.get_channel(instruction));
            auto success0 = noise_data->correct_0;
            auto success1 = noise_data->correct_1;
            noises.emplace_back(qubit, success0, success1);
        }
    }
    assert(!noises.empty());

    // success0
    sort(noises.begin(), noises.end(), [](const ReadoutNoise &a, const ReadoutNoise &b) {
        return a.success0 < b.success0;
    });
    result.insert(noises.front().target);
    result.insert(noises.back().target);

    // success1
    sort(noises.begin(), noises.end(), [](const ReadoutNoise &a, const ReadoutNoise &b) {
        return a.success1 < b.success1;
    });
    result.insert(noises.front().target);
    result.insert(noises.back().target);

    // accumulated error
    sort(noises.begin(), noises.end(), [](const ReadoutNoise &a, const ReadoutNoise &b) {
        return a.acc_err < b.acc_err;
    });
    result.insert(noises.front().target);
    result.insert(noises.back().target);

    // diff
    sort(noises.begin(), noises.end(), [](const ReadoutNoise &a, const ReadoutNoise &b) {
        return a.diff < b.diff;
    });
    if (noises.front().diff != noises.back().diff) {
        result.insert(noises.front().target);
        result.insert(noises.back().target);
    }

    // abs_diff
    sort(noises.begin(), noises.end(), [](const ReadoutNoise &a, const ReadoutNoise &b) {
        return a.abs_diff < b.abs_diff;
    });
    if (noises.front().abs_diff != noises.back().abs_diff) {
        result.insert(noises.front().target);
        result.insert(noises.back().target);
        assert(noises.front().abs_diff < noises.back().abs_diff);
    }

    return result;
}

