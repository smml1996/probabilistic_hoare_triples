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
bool Experiment::is_parse = false;


cpp_int Experiment::count_naive_strats(const int &horizon, const vector<shared_ptr<POMDPAction>> &actions) {

    if (horizon == 0) {
        return 1;
    }

    cpp_int result = 1;

    for (auto action : actions) {
        int succ_obs = 1;
        for (auto ins : action->instruction_sequence) {
            if (ins.gate_name == GateName::Meas) {
                succ_obs *= 2;
            }
        }
        result += succ_obs * this->count_naive_strats(horizon-1, actions);
    }
    return result;
}

cpp_int Experiment::get_naive_stats(const MethodType &method, const int &horizon, HardwareSpecification &spec, const unordered_map<int, int> &embedding) {
    int num_initial_beliefs = 0;

    if (method == MethodType::SingleDistBellman) {
        num_initial_beliefs = 1;
    } else {
        assert (method == MethodType::Convex);
        num_initial_beliefs = this->get_initial_distribution(embedding).size();
    }

    auto actions = this->get_actions(spec, embedding);
    auto temp = this->count_naive_strats(horizon, actions);

    return temp * num_initial_beliefs;
}

const set<string> Experiment::experiment_names = {
    "ghz",
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

map<cpp_int, Belief> Experiment::get_successor_beliefs(Belief &current_belief, HardwareSpecification &spec,
    const unordered_map<int, int> &embedding, shared_ptr<POMDPAction> &action) const {
    map<cpp_int, Belief> result;
    POMDP pomdp(this->precision);

    for (auto it : current_belief.probs) {
        auto current_v = it.first;
        if (this->guard(current_v, embedding, action)) {
            auto successors = action->get_successor_states(spec, current_v);
            for (auto s_it : successors) {
                auto succ = s_it.first;
                auto prob = s_it.second;

                auto obs = succ->get_obs()->get_memory_val();
                auto new_vertex = pomdp.create_new_vertex(succ->hybrid_state, succ->hidden_index);
                if (result.find(obs) == result.end()) {
                    result[obs] = Belief();
                    result[obs].obs = obs;
                }
                result[obs].add_val(new_vertex,  it.second * MyFloat(prob, this->precision * (max_horizon + 1)));
            }
        }
    }
    return result;
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
    //for (const auto &child: algorithm->children) {
    //    update_classical_state(child, classical_state);
    //}
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

    shared_ptr<Algorithm> head = make_shared<Algorithm>(meas_action, current_cstate->get_memory_val(), this->precision, -1);
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
    if (Experiment::is_parse) {
        return this->parse_results();
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
                        ConvexSolver solver(pomdp, actual_reward_f, this->precision * (max_horizon + 1),
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
                                    "tot_ins",
                                  "cstate_size",
                                  "verify_time"
                              })
                              , ",") << "\n";

    int batch = 0;
    vector<shared_ptr<Algorithm> > unique_algorithms;
    this->setup_params();
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
            cout << quantum_hardware << " / e=" << embedding_index << " / k="<<horizon<< " / m=" << method_str << endl;

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
            this->setup_params();

            {
                this->set_min_max_horizon(SingleDistBellman);
                int temp = this->max_horizon;
                this->set_min_max_horizon(Convex);
                this->max_horizon = max(temp, this->max_horizon);
            }

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
            auto tot_ins = this->get_actions(spec, embedding).size();

            cout << "started verification" << endl;
            auto verify_time = this->get_verify_time(method, spec, algorithm, stod(probability), embedding);
            cout <<"end verification" << endl;

            auto textbook_alg = this->get_textbook_algorithm(method, stoi(horizon));

            int baseline_index = get_algorithm_from_list(unique_algorithms, textbook_alg);
            if (baseline_index == -1) {
                baseline_index = unique_algorithms.size();
                unique_algorithms.push_back(textbook_alg);
                // dump algorithm
                dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(baseline_index) + ".txt"), textbook_alg);
                dump_to_file(parsed_algorithms_path / ("A_" + to_string(baseline_index) + ".txt"), textbook_alg);
            }
            auto baseline_probability = this->verify(method, spec, textbook_alg, -1, embedding);
            auto diff = this->verify(method, spec, algorithm, stod(probability), embedding) - baseline_probability;
            int cstate_size = pow(2, this->ncvars);
            parsed_stats_file << join(vector<string>({
                                              quantum_hardware,
                                              embedding_index,
                                              horizon,
                                              probability,
                                              to_string(baseline_probability),
                                              to_string(diff),
                                              method_str,
                                              to_string(round_to(method_time,
                                                                 Experiment::round_in_file)),
                                              to_string(algorithm_index),
                                              to_string(baseline_index),
                                              to_string(tot_strats),
                                              to_string(tot_ins),
                                              to_string(cstate_size),
                                              to_string(round_to(verify_time, Experiment::round_in_file)),
                                          }
                                      )
                                      , ",") << "\n";
            parsed_stats_file.flush();
        }

        batch += 1;
    }

    parsed_stats_file.close();
}

double Experiment::get_verify_time(const MethodType &method, HardwareSpecification &hw,
                                   shared_ptr<Algorithm> &algorithm, const double &actual_prob, const unordered_map<int, int> &embedding) {
    auto start_time = chrono::steady_clock::now();
    this->verify(method, hw, algorithm, actual_prob, embedding);
    auto end_time = chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();;
}

double Experiment::verify(const MethodType &method, HardwareSpecification &hw, shared_ptr<Algorithm> &algorithm,
                          const double &actual_prob, const unordered_map<int, int> &embedding) {
    vector<Belief> initial_beliefs;

    int hidden_index = -1;

    if (method == MethodType::SingleDistBellman) {
        auto initial_belief = Belief();

        for (auto it : this->get_initial_distribution(embedding)) {
            if (this->set_hidden_index) {
                hidden_index+=1;
            }
            initial_belief.set_val(make_shared<POMDPVertex>(it.first, hidden_index), MyFloat(it.second, this->precision * (max_horizon + 1)));
            initial_belief.obs = it.first->classical_state->get_memory_val();
        }

        initial_beliefs.push_back(initial_belief);
    } else {
        assert (method == MethodType::Convex);
        for (const auto & it : this->get_initial_distribution(embedding)) {
            if (this->set_hidden_index) {
                hidden_index+=1;
            }
            auto initial_state = make_shared<POMDPVertex>(it.first, hidden_index);
            auto belief = Belief();
            belief.set_val(initial_state, MyFloat(1, this->precision * (max_horizon + 1)));
            belief.obs = initial_state->hybrid_state->classical_state->get_memory_val();
            initial_beliefs.push_back(belief);
        }
    }
    double result = 1;
    assert (!initial_beliefs.empty());
    for (auto belief : initial_beliefs) {
        result = min(result, to_double(this->verify_at_belief(hw, algorithm, belief, embedding)));
    }
    if (actual_prob != -1) {
        if (!is_close(actual_prob, result, Experiment::round_in_file)) {
            cout << "Verification failed for: " << this->name << " -- " << actual_prob << "!=" << result << endl;
        }
    }
    return result;
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

MyFloat Experiment::verify_at_belief(HardwareSpecification &spec, shared_ptr<Algorithm> &algorithm, Belief & current_belief, const unordered_map<int, int>
                                     &embedding) {
    MyFloat curr_belief_val = this->postcondition(current_belief, embedding);
    // cout << "---------" << endl;
    // cout << curr_belief_val << endl;
    // current_belief.print();

    if (algorithm == nullptr ) {
        return curr_belief_val;
    }

    if (*algorithm->action == HALT_ACTION) {
        return curr_belief_val;
    }

    auto all_actions = this->get_actions(spec, embedding);

    shared_ptr<POMDPAction> action = nullptr;
    for (auto a : all_actions) {
        if (a->name == algorithm->action->name) {
            action = a;
            break;
        }
    }
    assert(action != nullptr);

    // build next_beliefs, separate them by different observables
    map<cpp_int, Belief> obs_to_next_beliefs = this->get_successor_beliefs(current_belief, spec, embedding, action);

    if (!obs_to_next_beliefs.empty()) {
        MyFloat bellman_val("0", this->precision * (max_horizon + 1));
        set<cpp_int> visited_cstates;
        for (int i = 0; i < algorithm->children.size(); i++) {
            if(obs_to_next_beliefs.find(algorithm->children[i]->classical_state) != obs_to_next_beliefs.end()) {
                visited_cstates.insert(algorithm->children[i]->classical_state);
                bellman_val = bellman_val + this->verify_at_belief(spec, algorithm->children[i], obs_to_next_beliefs[algorithm->children[i]->classical_state], embedding);
            }
        }

        for (const auto& it: obs_to_next_beliefs) {
            if (visited_cstates.find(it.first) == visited_cstates.end()) {
                bellman_val = bellman_val + this->postcondition(it.second, embedding);
            }
        }
        return bellman_val;
    } else {
        return curr_belief_val;
    }
}

shared_ptr<Algorithm> Experiment::get_textbook_algorithm(MethodType &method, const int &horizon) {
    auto halt_algorithm = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, 0);
    return halt_algorithm;
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

void Experiment::check_params() const {
    assert(this->precision == 8);
    assert(!this->with_thermalization);
    assert(this->optimize);
    assert(!this->method_types.empty());
    assert(this->nqvars > 0);
    assert(this->ncvars > 0);
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
