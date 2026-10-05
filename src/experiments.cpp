#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include "experiments.hpp"

#include <cassert>

#include "solvers.hpp"
#include "utils.hpp"
using namespace std;

int Experiment::round_in_file = 5;
bool Experiment::is_naive = false;

int Experiment::get_naive_stats(const int &horizon) const {
    assert(false);
    return 0.0;
}

const set<string> Experiment::experiment_names = {
    "ghz",
    "ipma",
    "ipma2",
    "cxh",
    "reset",
    "lphase"
};

std::string join(const std::vector<std::string> &parts, const std::string &delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << parts[i];
    }
    return oss.str();
}

string get_method_string(MethodType method) {
    if (method == MethodType::Convex) {
        return "convex";
    }

    if (method == MethodType::SingleDistBellman) {
        return "bellman";
    }

    if (method == MethodType::Naive) {
        return "naive";
    }

    throw invalid_argument("Method type not recognized");
}

string gate_to_string(const set<MethodType> &methods) {
    string result;
    for (auto m: methods) {
        if (!result.empty()) {
            result += ", ";
        }
        result += gate_to_string(m);
    }
    return result;
}

set<string> get_solver_methods_strings() {
    set<string> solver_methods;
    for (int i = 0; i < MethodType::MethodCount; i++) {
        solver_methods.insert(gate_to_string(static_cast<MethodType>(i)));
    }

    return solver_methods;
}

string gate_to_string(const MethodType &method) {
    switch (method) {
        case MethodType::SingleDistBellman:
            return "bellman";
        case MethodType::Convex:
            return "convex";
        case MethodType::Naive:
            return "naive";
        default:
            assert(false);
    }
}

MethodType str_to_method_type(const string &method) {
    for (int i = 0; i < MethodType::MethodCount; i++) {
        string m_str = gate_to_string(static_cast<MethodType>(i));
        if (m_str == method) {
            return static_cast<MethodType>(i);
        }
    }
    throw invalid_argument("Method type not recognized: " + method);
}

bool Experiment::guard(const shared_ptr<POMDPVertex> &, const unordered_map<int, int> &,
                       const shared_ptr<POMDPAction> &a) const {
    if (*a == HALT_ACTION) return false;
    return true;
}

fs::path Experiment::get_wd() const {
    return fs::path("..") / "results" / this->name;
}

fs::path Experiment::get_final_wd() const {
    return fs::path("..") / "parsed_results" / this->name;
}

fs::path get_final_wd(const string &name) {
    return fs::path("..") / "parsed_results" / name;
}

bool Experiment::setup_working_dir() const {
    fs::path dir_path = this->get_wd();

    if (!fs::exists(dir_path)) {
        if (fs::create_directories(dir_path)) {
            std::cout << "main experiments directory created successfully.\n";
        } else {
            std::cerr << "Failed to create main experiments directory.\n";
            return false;
        }
    }

    dir_path = fs::path("..") / "results" / this->name / "algorithms";

    if (!fs::exists(dir_path)) {
        if (fs::create_directories(dir_path)) {
            std::cout << "Algorithms directory created successfully.\n";
        } else {
            std::cerr << "Failed to create directory for storing algorithms.\n";
            return false;
        }
    }
    dir_path = fs::path("..") / "results" / this->name / "raw_algorithms";

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

vector<HardwareSpecification> Experiment::get_hardware_specs() const {
    auto quantum_hardwares = this->get_allowed_hardware();
    vector<HardwareSpecification> result;

    result.reserve(quantum_hardwares.size());
    for (QuantumHardware qw: quantum_hardwares) {
        result.emplace_back(qw, this->with_thermalization, this->optimize);
    }

    return result;
}

vector<int> Experiment::get_qubits_used(const unordered_map<int, int> &embedding) {
    vector<int> result;
    result.reserve(embedding.size());
    for (auto it: embedding) {
        result.push_back(it.second);
    }
    return result;
}

Belief Experiment::get_initial_belief(const POMDP &pomdp) const {
    Belief initial_belief;
    auto INIT_CHANNEL = make_shared<POMDPAction>("INIT__", vector<Instruction>({}), this->precision,
                                                 vector<Instruction>({}));
    if (pomdp.transition_matrix.at(pomdp.initial_state).find(INIT_CHANNEL) != pomdp.transition_matrix.at(
            pomdp.initial_state).end()) {
        for (const auto &it: pomdp.transition_matrix.at(pomdp.initial_state).at(INIT_CHANNEL)) {
            initial_belief.add_val(it.first, it.second);
        }
    } else {
        initial_belief.set_val(pomdp.initial_state, MyFloat("1", this->precision * (this->max_horizon + 1)));
    }
    initial_belief.obs = pomdp.initial_state->hybrid_state->classical_state->get_memory_val();
    return initial_belief;
}

vector<shared_ptr<POMDPVertex> > Experiment::get_initial_states(const POMDP &pomdp) const {
    vector<shared_ptr<POMDPVertex> > initial_states;

    auto INIT_CHANNEL = make_shared<POMDPAction>("INIT__", vector<Instruction>({}), this->precision,
                                                 vector<Instruction>({}));

    if (pomdp.transition_matrix.at(pomdp.initial_state).find(INIT_CHANNEL) != pomdp.transition_matrix.at(
            pomdp.initial_state).end()) {
        for (const auto &it: pomdp.transition_matrix.at(pomdp.initial_state).at(INIT_CHANNEL)) {
            initial_states.push_back(it.first);
        }
    } else {
        initial_states.push_back(pomdp.initial_state);
    }

    return initial_states;
}

void Experiment::update_classical_state(const shared_ptr<Algorithm> &algorithm, const cpp_int &classical_state) {
    algorithm->classical_state = classical_state;
    for (const auto &child: algorithm->children) {
        update_classical_state(child, classical_state);
    }
}

shared_ptr<Algorithm> Experiment::build_meas_sequence(const int &total_meas, const int &write_address,
                                                      const shared_ptr<POMDPAction> &meas_action,
                                                      const shared_ptr<ClassicalState> &current_cstate,
                                                      const shared_ptr<Algorithm> &on_most0,
                                                      const shared_ptr<Algorithm> &on_most1, int ones_count,
                                                      int zeros_count) {
    assert(total_meas >= 0);
    if (total_meas == 0) {
        shared_ptr<Algorithm> result;
        if (ones_count > zeros_count) {
            result = deep_copy_algorithm(on_most1);
        } else {
            result = deep_copy_algorithm(on_most0);
        }
        update_classical_state(result, current_cstate->get_memory_val());
        return result;
    }

    shared_ptr<Algorithm> head = make_shared<Algorithm>(meas_action, current_cstate->get_memory_val(), 10, -1);
    auto state0 = current_cstate->write(write_address, false);
    auto state1 = current_cstate->write(write_address, true);
    head->children.push_back(this->build_meas_sequence(total_meas - 1, write_address, meas_action, state0, on_most0,
                                                       on_most1, ones_count, zeros_count + 1));
    head->children.push_back(this->build_meas_sequence(total_meas - 1, write_address, meas_action, state1, on_most0,
                                                       on_most1, ones_count + 1, zeros_count));

    return head;
}

Experiment::Experiment(const string &name, const set<QuantumHardware> &hw_list) {
    this->name = name;
    this->hw_list = hw_list;
}

void Experiment::run() {
    if (this->is_naive) {
        return this->run_naive();
    }
    this->setup_params();
    if (!setup_working_dir()) {
        return;
    }

    fs::path results_path = this->get_wd() / "stats.csv";

    // Open file for writing (this overwrites the file if it exists)
    std::ofstream results_file(results_path);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open file: " << results_path << "\n";
        return;
    }


    // algorithms folder
    fs::path algorithms_folder = this->get_wd() / "algorithms";
    fs::path raw_algorithms_folder = this->get_wd() / "raw_algorithms";

    // write header in results file
    results_file << join(vector<string>({
                             "hardware",
                             "embedding_index",
                             "horizon",
                             "pomdp_build_time",
                             "probability",
                             "method",
                             "method_time",
                             "algorithm_index",
                             "tot_strats"
                         })
                         , ",") << "\n";

    auto actual_guard = [this](const shared_ptr<POMDPVertex> &v, const std::unordered_map<int, int> &m,
                               const shared_ptr<POMDPAction> &a) {
        return this->guard(v, m, a);
    };

    auto actual_reward_f = [this](const Belief &b, const unordered_map<int, int> &embedding) -> MyFloat {
        return this->postcondition(b, embedding);
    };

    auto actual_reward_f_double = [this](const VertexDict &b, const unordered_map<int, int> &embedding) -> double {
        return this->postcondition_double(b, embedding);
    };

    // we store all unique algorithms
    vector<shared_ptr<Algorithm> > unique_algorithms;

    for (auto qh: this->hw_list) {
        auto hardware_spec = HardwareSpecification(qh, this->with_thermalization, this->optimize);
        string hardware_name = hardware_spec.get_hardware_name();
        cout << hardware_name << endl;
        auto embeddings = this->get_hardware_scenarios(hardware_spec);
        int embedding_index = 0;
        for (auto embedding: embeddings) {
            for (auto method: this->method_types) {
                this->set_min_max_horizon(method);
                this->check_params();
                // initial distribution
                auto initial_distribution = this->get_initial_distribution(embedding);
                // actions
                auto actions = this->get_actions(hardware_spec, embedding);
                // POMDP build
                this->target_vertices.clear();
                POMDP pomdp = POMDP(this->precision);
                auto qubits_used = get_qubits_used(embedding);
                cout << "start building pomdp" << endl;
                auto start_pomdp_build = chrono::high_resolution_clock::now();
                pomdp.build_pomdp(actions, hardware_spec, this->max_horizon, embedding, nullptr, initial_distribution,
                                  qubits_used, actual_guard, this->set_hidden_index);
                auto end_pomdp_build = chrono::high_resolution_clock::now(); // end time
                auto pomdp_build_time = chrono::duration<double>(end_pomdp_build - start_pomdp_build).count();
                cout << "end building pomdp" << endl;
                // pomdp.print_pomdp();
                // initial belief
                auto initial_belief = this->get_initial_belief(pomdp);
                auto initial_states = this->get_initial_states(pomdp);
                for (int horizon = this->min_horizon; horizon <= this->max_horizon; horizon++) {
                    auto HALT_ALGORITHM = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION),
                                                                 get_belief_cs(initial_belief), 0);
                    cout << "horizon:" << horizon << "\n";
                    double method_time;
                    pair<shared_ptr<Algorithm>, double> result;
                    int tot_strats = 1;
                    if (method == MethodType::SingleDistBellman) {
                        SingleDistributionSolver solver(pomdp, actual_reward_f, this->precision * (max_horizon + 1),
                                                        embedding);
                        auto result_temp = solver.solve(initial_belief, horizon);
                        assert(result_temp.second.precision == precision *(max_horizon+1));
                        result = make_pair(make_shared<Algorithm>(*result_temp.first), to_double(result_temp.second));
                        method_time = solver.running_time;
                    } else {
                        assert(method == MethodType::Convex || method == MethodType::Naive);
                        bool use_pareto = true;
                        if (method == MethodType::Naive) {
                            use_pareto = false;
                        }
                        ConvexSolver solver(pomdp, actual_reward_f, actual_reward_f_double,
                                            this->precision * (max_horizon + 1),
                                            embedding, use_pareto);
                        auto result_temp = solver.solve(initial_states, horizon);
                        result = make_pair(make_shared<Algorithm>(*result_temp.first), result_temp.second);
                        method_time = solver.running_time;
                        tot_strats = solver.total_strategies;
                    }

                    auto algorithm_index = get_algorithm_from_list(unique_algorithms, result.first);
                    if (algorithm_index == -1) {
                        algorithm_index = unique_algorithms.size();
                        unique_algorithms.push_back(result.first);
                        fs::path algorithm_path = algorithms_folder / ("A_" + to_string(algorithm_index + 1) + ".txt");
                        dump_to_file(algorithm_path, result.first);

                        fs::path raw_algorithm_path =
                                raw_algorithms_folder / ("R_" + to_string(algorithm_index + 1) + ".txt");
                        dump_raw_algorithm(raw_algorithm_path, result.first);
                    }

                    results_file << join(vector<string>({
                                             hardware_name,
                                             to_string(embedding_index),
                                             to_string(horizon),
                                             to_string(round_to(pomdp_build_time, Experiment::round_in_file)),
                                             to_string(round_to(result.second, Experiment::round_in_file)),
                                             gate_to_string(method),
                                             to_string(round_to(method_time, Experiment::round_in_file)),
                                             to_string(algorithm_index),
                                             to_string(tot_strats)
                                         })
                                         , ",") << "\n";
                    results_file.flush();
                }
            }
            embedding_index++;
        }
    }

    results_file.close();

    cout << "Done" << endl;
}

void Experiment::run_naive() {
    if (!setup_working_dir()) {
        return;
    }

    fs::path results_path = this->get_wd() / "stats.csv";

    // Open file for writing (this overwrites the file if it exists)
    std::ofstream results_file(results_path);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open file: " << results_path << "\n";
        return;
    }

    // write header in results file
    results_file << join(vector<string>({
                             "horizon",
                             "pomdp_build_time",
                             "probability",
                             "method_time",
                             "tot_strats"
                         })
                         , ",") << "\n";

    auto actual_guard = [this](const shared_ptr<POMDPVertex> &v, const std::unordered_map<int, int> &m,
                               const shared_ptr<POMDPAction> &a) {
        return this->guard(v, m, a);
    };

    auto actual_reward_f = [this](const Belief &b, const unordered_map<int, int> &embedding) -> MyFloat {
        return this->postcondition(b, embedding);
    };

    auto actual_reward_f_double = [this](const VertexDict &b, const unordered_map<int, int> &embedding) -> double {
        return this->postcondition_double(b, embedding);
    };


    auto hardware_spec = HardwareSpecification(QuantumHardware::PerfectHardware, this->with_thermalization,
                                               this->optimize);
    string hardware_name = hardware_spec.get_hardware_name();
    auto embeddings = this->get_hardware_scenarios(hardware_spec);
    assert(embeddings.size() == 1);
    auto embedding = embeddings[0];


    this->set_min_max_horizon(MethodType::Naive);
    // initial distribution
    auto initial_distribution = this->get_initial_distribution(embedding);
    // actions
    auto actions = this->get_actions(hardware_spec, embedding);
    // POMDP build
    this->target_vertices.clear();
    POMDP pomdp = POMDP(this->precision);
    auto qubits_used = get_qubits_used(embedding);
    cout << "start building pomdp" << endl;
    auto start_pomdp_build = chrono::high_resolution_clock::now();
    pomdp.build_pomdp(actions, hardware_spec, this->max_horizon, embedding, nullptr, initial_distribution, qubits_used,
                      actual_guard, this->set_hidden_index);
    auto end_pomdp_build = chrono::high_resolution_clock::now(); // end time
    auto pomdp_build_time = chrono::duration<double>(end_pomdp_build - start_pomdp_build).count();
    cout << "end building pomdp" << endl;
    // initial belief
    auto initial_belief = this->get_initial_belief(pomdp);
    auto initial_states = this->get_initial_states(pomdp);
    for (int horizon = this->min_horizon; horizon <= this->max_horizon; horizon++) {
        auto HALT_ALGORITHM = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION),
                                                     get_belief_cs(initial_belief), 0);
        cout << "horizon:" << horizon << "\n";
        pair<shared_ptr<Algorithm>, double> result;
        bool use_pareto = false;

        ConvexSolver solver(pomdp, actual_reward_f, actual_reward_f_double, this->precision * (max_horizon + 1),
                            embedding, use_pareto);
        auto result_temp = solver.solve(initial_states, horizon);
        result = make_pair(make_shared<Algorithm>(*result_temp.first), result_temp.second);
        double method_time = solver.running_time;
        int tot_strats = solver.total_strategies;
        results_file << join(vector<string>({
                                 to_string(horizon),
                                 to_string(round_to(pomdp_build_time, Experiment::round_in_file)),
                                 to_string(round_to(result.second, Experiment::round_in_file)),
                                 to_string(round_to(method_time, Experiment::round_in_file)),
                                 to_string(tot_strats)
                             })
                             , ",") << "\n";
        results_file.flush();
    }

    results_file.close();

    cout << "Done" << endl;
}

void Experiment::generate_script() {
    this->setup_params();
    filesystem::path p = fs::path("..") / "scripts" / (this->name + ".sh");


    std::ofstream results_file(p);

    if (!results_file.is_open()) {
        std::cerr << "Failed to open file: " << p << "\n";
        return;
    }

    auto allowed_hardware = this->get_allowed_hardware();
    int size_batch = allowed_hardware.size() / this->num_batches;
    if (size_batch == 0) {
        size_batch = 1;
    }

    int current_batch = 0;
    string custom_name = this->name + "_" + to_string(current_batch);
    results_file << "sbatch server_script.sh " << this->name << " " << custom_name << " ";
    int count = 0;
    for (auto hw: allowed_hardware) {
        if (count > size_batch) {
            results_file << endl;
            current_batch += 1;
            custom_name = this->name + "_" + to_string(current_batch);
            results_file << "sbatch server_script.sh " << this->name << " " << custom_name << " ";
            count = 0;
        }
        if (count > 0) {
            results_file << ",";
        }
        results_file << to_string(hw);
        count += 1;
    }
    results_file << endl;
    results_file.close();
}

void Experiment::parse_results() {
    map<QuantumHardware, HardwareSpecification> qw_to_spec;
    for (auto qw: this->get_allowed_hardware()) {
        auto hs = HardwareSpecification(qw, this->with_thermalization, this->optimize);
        assert(qw_to_spec.find(qw) == qw_to_spec.end());
        qw_to_spec.insert({qw, hs});
    }

    filesystem::path parsed_results_path = fs::path("..") / "parsed_results";

    if (!fs::exists(parsed_results_path)) {
        fs::create_directory(parsed_results_path);
    }

    cout << "parsing experiment " << this->name << endl;
    fs::path exp_dir = parsed_results_path / this->name;
    fs::create_directories(exp_dir);

    fs::path parsed_algorithms_path = exp_dir / "raw_algorithms";
    fs::create_directories(parsed_algorithms_path);

    fs::path parsed_stats_path = exp_dir / "stats.csv";
    ofstream parsed_stats_file(parsed_stats_path);
    parsed_stats_file << join(vector<string>({
                                  "hardware",
                                  "embedding_index",
                                  "horizon",
                                  "probability",
                                  "baseline_prob",
                                  "diff_probs",
                                  "method",
                                  "time",
                                  "algorithm_index",
                                  "baseline_index",
                                  "tot_strats",
                                  "strats_naive",
                                  "verify_time"
                              })
                              , ",") << "\n";

    int batch = 0;
    vector<shared_ptr<Algorithm> > unique_algorithms;
    auto actual_guard = [this](const shared_ptr<POMDPVertex> &v, const std::unordered_map<int, int> &m,
                               const shared_ptr<POMDPAction> &a) {
        return this->guard(v, m, a);
    };
    while (true) {
        fs::path raw_exp_path = fs::path("..") / "results" / (this->name + "_" + to_string(batch));
        if (!fs::exists(raw_exp_path)) break;
        ifstream f(raw_exp_path / "stats.csv");

        string line;
        getline(f, line);
        while (getline(f, line)) {
            vector<string> tokens;
            split_str(line, ',', tokens);
            string quantum_hardware = tokens[0];
            string embedding_index = tokens[1];
            string horizon = tokens[2];
            double pomdp_build_time = stod(tokens[3]);
            string probability = tokens[4];
            string method_str = tokens[5];
            double method_time = stod(tokens[6]);
            int algorithm_index = stoi(tokens[7]);
            int tot_strats = stoi(tokens[8]);

            std::ifstream curr_alg_file(
                raw_exp_path / "raw_algorithms" / ("R_" + to_string(algorithm_index + 1) + ".txt"));
            if (!curr_alg_file.is_open()) {
                std::cerr << "Error opening file\n";
                return;
            }

            MethodType method = str_to_method_type(method_str);

            json current_algorithm;
            curr_alg_file >> current_algorithm;
            curr_alg_file.close();
            shared_ptr<Algorithm> algorithm = make_shared<Algorithm>(current_algorithm);
            auto real_index = get_algorithm_from_list(unique_algorithms, algorithm);


            if (real_index == -1) {
                real_index = unique_algorithms.size();
                unique_algorithms.push_back(algorithm);
                // dump algorithm
                dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(real_index) + ".txt"), algorithm);
                dump_to_file(parsed_algorithms_path / ("A_" + to_string(real_index) + ".txt"), algorithm);
            }
            algorithm_index = real_index;
            auto spec = qw_to_spec.at(to_quantum_hardware(quantum_hardware));
            unordered_map<int, int> embedding = this->get_hardware_scenarios(spec)[stoi(embedding_index)];

            POMDP pomdp(this->precision);

            auto qubits_used = Experiment::get_qubits_used(embedding);
            auto actions = this->get_actions(spec, embedding);
            auto initial_distribution = this->get_initial_distribution(embedding);
            pomdp.build_pomdp(actions, spec, stoi(horizon), embedding, nullptr, initial_distribution, qubits_used,
                              actual_guard, this->set_hidden_index);
            auto verify_time = this->get_verify_time(method, pomdp, algorithm, stod(probability));


            auto textbook_alg = this->get_textbook_algorithm(method, stoi(horizon));

            int baseline_index = get_algorithm_from_list(unique_algorithms, textbook_alg);
            if (baseline_index == -1) {
                baseline_index = unique_algorithms.size();
                unique_algorithms.push_back(textbook_alg);
                // dump algorithm
                dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(real_index) + ".txt"), textbook_alg);
                dump_to_file(parsed_algorithms_path / ("A_" + to_string(real_index) + ".txt"), textbook_alg);
            }


            auto baseline_probability = this->verify(method, pomdp, textbook_alg);

            auto strats_naive = this->get_naive_stats(stoi(horizon));

            parsed_stats_file << join(vector<string>({
                                              quantum_hardware,
                                              embedding_index,
                                              horizon,
                                              probability,
                                              to_string(baseline_probability),
                                              to_string(round_to(stod(probability) - baseline_probability,
                                                                 Experiment::round_in_file)),
                                              method_str,
                                              to_string(round_to(method_time + pomdp_build_time,
                                                                 Experiment::round_in_file)),
                                              to_string(algorithm_index),
                                              to_string(baseline_index),
                                              to_string(tot_strats),
                                              to_string(strats_naive),
                                              to_string(round_to(verify_time, Experiment::round_in_file)),
                                          }
                                      )
                                      , ",") << "\n";
        }

        batch += 1;
    }

    parsed_stats_file.close();
}

double Experiment::get_verify_time(const MethodType &method, POMDP &pomdp,
                                   shared_ptr<Algorithm> &algorithm, const double &actual_prob) {
    return 1;
}

double Experiment::verify(const MethodType &method, const POMDP &pomdp, shared_ptr<Algorithm> &algorithm) {
    return 1;
}

map<string, shared_ptr<POMDPAction> > Experiment::get_actions_dictionary(
    HardwareSpecification &hardware_spec, const int &num_qubits) const {
    map<string, shared_ptr<POMDPAction> > actions_dictionary;
    unordered_map<int, int> embedding;
    for (int i = 0; i < num_qubits; i++) embedding[i] = i;

    auto actions = this->get_actions(hardware_spec, embedding);

    for (const auto &action: actions) {
        actions_dictionary[action->name] = action;
    }

    return actions_dictionary;
}

string Experiment::get_postcondition(const MethodType &method) {
    throw runtime_error("not implemented!");
}

shared_ptr<Algorithm> Experiment::get_textbook_algorithm(MethodType &method, const int &horizon) {
    throw runtime_error("Not implemented");
}

void Experiment::set_with_thermalization() {
    this->with_thermalization = false;
}

void Experiment::set_optimize() {
    this->optimize = true;
}

void Experiment::set_hidden_index_to() {
    this->set_hidden_index = false;
}

void Experiment::set_uses_cnot() {
    this->uses_cnot = true;
}

void Experiment::set_precision() {
    this->precision = 8;
}

set<QuantumHardware> Experiment::get_allowed_hardware() const {
    if (!this->hw_list.empty()) return this->hw_list;
    set<QuantumHardware> result;
    result.insert(QuantumHardware::PerfectHardware);
    if (this->uses_cnot) {
        for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
            auto quantum_hardware = static_cast<QuantumHardware>(i);
            if (quantum_hardware == QuantumHardware::PerfectHardware) {
                continue;
            }
            BasisGates basis_gates_type = get_hw_basis_gate_type(quantum_hardware);
            if ((basis_gates_type != BasisGates::TYPE5 && basis_gates_type != BasisGates::TYPE2)) {
                result.insert(quantum_hardware);
            }
        }
    } else {
        for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
            result.insert(static_cast<QuantumHardware>(i));
        }
    }

    return result;
}

void Experiment::setup_params() {
    this->set_precision();
    this->set_with_thermalization();
    this->set_optimize();
    this->set_methods();
    this->set_hidden_index_to();
    this->set_num_vars();
    this->set_global_equality();
    this->set_uses_cnot();
}

bool Experiment::check_params() const {
    assert(this->precision == 8);
    assert(!this->with_thermalization);
    assert(this->optimize);
    assert(this->method_types.size() >= 1);
    assert(this->nqvars > 0);
    assert(this->ncvars > 0);
    return true;
}

void Experiment::set_global_equality() {
    QuantumState::use_global_eq = true;
}

void Experiment::set_num_batches() {
    this->num_batches = 10;
}

ReadoutNoise::ReadoutNoise(int target, double success0, double success1) {
    this->target = target;
    this->success0 = success0;
    this->success1 = success1;
    this->diff = success0 - success1;
    this->acc_err = 1 - success0 + 1 - success1;
    this->abs_diff = abs(success0 - success1);
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
            shared_ptr<MeasurementChannel> noise_data = static_pointer_cast<MeasurementChannel>(
                hardware_spec.get_channel(instruction));
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

[[maybe_unused]] static double verify_single_distribution(const VertexDict &current_belief, Experiment &experiment,
                                                          HardwareSpecification &hardware_spec,
                                                          const shared_ptr<Algorithm> &algorithm,
                                                          const unordered_map<int, int> &embedding, int precision) {
    double curr_belief_val = experiment.postcondition_double(current_belief, embedding);

    if (algorithm == nullptr) {
        return curr_belief_val;
    }

    auto action_ = algorithm->action;
    if (*action_ == HALT_ACTION) {
        return curr_belief_val;
    }

    vector<Instruction> seq;

    for (const auto &instruction_: action_->pseudo_instruction_sequence) {
        for (const auto &instruction: hardware_spec.to_basis_gates_impl(instruction_)) {
            seq.push_back(instruction.rename(embedding));
            if (hardware_spec.get_hardware() != QuantumHardware::PerfectHardware && hardware_spec.
                instructions_to_channels.find(make_shared<Instruction>(seq[seq.size() - 1])) == hardware_spec.
                instructions_to_channels.end()) {
                return 0.0;
            }
        }
    }

    POMDPAction action = POMDPAction(action_->name, seq, precision, action_->pseudo_instruction_sequence);

    // build next_beliefs, separate them by different observables
    unordered_map<cpp_int, VertexDict> obs_to_next_beliefs;
    for (auto &prob: current_belief.probs) {
        auto current_v = prob.first;
        if (prob.second > 0) {
            auto successors = action.get_successor_states(hardware_spec, current_v);
            for (auto &it_next_v: successors) {
                double prob2 = it_next_v.second;
                if (prob2 > 0) {
                    obs_to_next_beliefs[it_next_v.first->hybrid_state->classical_state->get_memory_val()].add_val(
                        it_next_v.first,
                        prob.second * prob2);
                } else {
                    assert(is_close(prob2, 0.0, 10));
                }
            }
        }
    }


    if (!obs_to_next_beliefs.empty()) {
        double bellman_val = 0.0;
        set<cpp_int> visited_cstates;
        for (int i = 0; i < algorithm->children.size(); i++) {
            if (obs_to_next_beliefs.find(algorithm->children[i]->classical_state) != obs_to_next_beliefs.end()) {
                visited_cstates.insert(algorithm->children[i]->classical_state);
                bellman_val = bellman_val + verify_single_distribution(
                                  obs_to_next_beliefs[algorithm->children[i]->classical_state], experiment,
                                  hardware_spec, algorithm->children[i], embedding, precision);
            }
        }

        for (const auto &it: obs_to_next_beliefs) {
            if (visited_cstates.find(it.first) == visited_cstates.end()) {
                bellman_val = bellman_val + experiment.postcondition_double(it.second, embedding);
            }
        }
        return bellman_val;
    } else {
        return curr_belief_val;
    }
}

double verify_algorithm(POMDP &pomdp, Experiment &experiment, const Algorithm &algorithm,
                        HardwareSpecification &hardware_spec,
                        unordered_map<int, int> &embedding, bool is_convex, int max_horizon) {
    auto initial_distribution_ = experiment.get_initial_distribution(embedding);
    VertexDict initial_distribution;
    experiment.max_horizon = max_horizon;

    int hidden_index = -1;
    if (experiment.set_hidden_index) {
        hidden_index = 0;
    }
    for (const auto &it: initial_distribution_) {
        shared_ptr<POMDPVertex> v = pomdp.get_vertex(it.first, hidden_index);
        initial_distribution.add_val(v, it.second);
        if (experiment.set_hidden_index) {
            hidden_index += 1;
        }
    }

    auto postcondition = [&experiment](const VertexDict &b, const unordered_map<int, int> &embedding) -> double {
        return experiment.postcondition_double(b, embedding);
    };

    if (is_convex) {
        double current_val = 0.0;
        bool is_first = true;
        for (const auto &it: initial_distribution.probs) {
            VertexDict current_distribution;
            assert(current_distribution.probs.empty());
            current_distribution.set_val(it.first, 1.0);
            double value = 0;

            if (*algorithm.action == random_branch) {
                for (int c_index = 0; c_index < algorithm.children.size(); c_index++) {
                    double prob = algorithm.children_probs.at(c_index);

                    auto acc = get_algorithm_acc_double(pomdp, algorithm.children.at(c_index), current_distribution,
                                                        postcondition, embedding);
                    value += prob * acc;
                }
            } else {
                auto acc = get_algorithm_acc_double(pomdp, make_shared<Algorithm>(algorithm), current_distribution,
                                                    postcondition, embedding);
                value = acc;
            }

            if (is_first) {
                current_val = value;
            } else {
                current_val = min(value, current_val);
            }
            is_first = false;
        }

        return current_val;
    } else {
        auto current_val = get_algorithm_acc_double(pomdp, make_shared<Algorithm>(algorithm), initial_distribution,
                                                    postcondition, embedding);

        return current_val;
    }
}

MyFloat precise_verify_algorithm(POMDP &pomdp, Experiment &experiment, const Algorithm &algorithm,
                                 unordered_map<int, int> &embedding, bool is_convex, int max_horizon) {
    auto initial_distribution_ = experiment.get_initial_distribution(embedding);
    Belief initial_distribution;

    int precision = (max_horizon + 1) * experiment.precision;
    experiment.max_horizon = max_horizon;

    int hidden_index = -1;
    if (experiment.set_hidden_index) {
        hidden_index = 0;
    }
    for (const auto &it: initial_distribution_) {
        shared_ptr<POMDPVertex> v = pomdp.get_vertex(it.first, hidden_index);
        initial_distribution.add_val(v, MyFloat(to_string(it.second), precision));
        if (experiment.set_hidden_index) {
            hidden_index += 1;
        }
    }

    auto postcondition = [&experiment](const Belief &b, const unordered_map<int, int> &embedding) -> MyFloat {
        return experiment.postcondition(b, embedding);
    };

    if (is_convex) {
        MyFloat current_val("0", precision);
        bool is_first = true;
        assert(algorithm.children.size() < 3 && !algorithm.children.empty());
        for (const auto &it: initial_distribution.probs) {
            Belief current_distribution;
            assert(current_distribution.probs.empty());
            current_distribution.set_val(it.first, MyFloat("1", precision));

            MyFloat value("0", precision);
            if (*algorithm.action == random_branch) {
                for (int c_index = 0; c_index < algorithm.children.size(); c_index++) {
                    auto prob = MyFloat(to_string(algorithm.children_probs.at(c_index)), precision);

                    auto acc = get_algorithm_acc(pomdp, algorithm.children.at(c_index), current_distribution,
                                                 postcondition, embedding, precision);
                    value = value + prob * acc;
                }
            } else {
                auto acc = get_algorithm_acc(pomdp, make_shared<Algorithm>(algorithm), current_distribution,
                                             postcondition, embedding, precision);
                value = value + acc;
            }
            if (is_first) {
                current_val = value;
            } else {
                current_val = min(value, current_val);
            }
            is_first = false;
        }
        return current_val;
    } else {
        auto current_val = get_algorithm_acc(pomdp, make_shared<Algorithm>(algorithm), initial_distribution,
                                             postcondition, embedding, precision);

        return current_val;
    }
}

StatsLine::StatsLine(const string &exp_name, const string &line, const unordered_map<QuantumHardware,
                         vector<unordered_map<int, int> > > &embeddings) : algorithm(
    make_shared<POMDPAction>(HALT_ACTION), 0, 0, -1) {
    vector<string> tokens;
    split_str(line, ',', tokens);
    this->quantum_hardware = to_quantum_hardware(tokens[0]);
    this->embedding_index = stoi(tokens[1]);
    assert(embedding_index < embeddings.at(this->quantum_hardware).size());
    this->embedding = embeddings.at(this->quantum_hardware)[embedding_index];
    // add qubit for hidden index in state discrimination problem
    if (exp_name == "basic_zero_plus_discr") {
        assert(this->embedding.find(0) != this->embedding.end());
        if (this->embedding.at(0) == 0) {
            this->embedding[1] = 1;
        } else {
            this->embedding[1] = 0;
        }
    }

    this->horizon = stoi(tokens[2]);
    this->threshold = stod(tokens[4]);

    if (tokens[5] == "bellman") {
        this->method = MethodType::SingleDistBellman;
    } else {
        assert(tokens[5] == "convex");
        this->method = MethodType::Convex;
    }

    this->algorithm_index = stoi(tokens[7]);
    std::ifstream curr_alg_file(
        get_final_wd(exp_name) / "raw_algorithms" / ("R_" + to_string(algorithm_index) + ".txt"));
    json current_algorithm;
    curr_alg_file >> current_algorithm;
    curr_alg_file.close();
    this->algorithm = Algorithm(current_algorithm);
}

StatsFile::StatsFile(const string &experiment_name_, const Experiment &experiment) {
    // experiment object is only used to get hardware used in the experiment and to get hardware scenarios
    this->experiment_name = experiment_name_;
    std::ifstream stats_file(get_final_wd(experiment_name_) / "stats.csv");
    if (!stats_file.is_open()) {
        std::cerr << "Failed to open stats file: " << (get_final_wd(experiment_name_) / "stats.csv") << "\n";
        assert(false);
        return;
    }
    unordered_map<QuantumHardware, vector<unordered_map<int, int> > > hardware_to_embeddings;

    for (auto quantum_hardware: experiment.get_allowed_hardware()) {
        auto hardware_spec = HardwareSpecification(quantum_hardware, false, true);
        hardware_to_embeddings[quantum_hardware] = experiment.get_hardware_scenarios(hardware_spec);
    }

    string line;
    getline(stats_file, line); // do not use header line
    while (getline(stats_file, line)) {
        this->stats.emplace_back(this->experiment_name, line, hardware_to_embeddings);
    }
}
