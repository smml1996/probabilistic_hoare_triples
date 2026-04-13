//
// Created by Stefanie Muroya Lei on 02.10.25.
//
#include <fstream>
#include <iostream>


#include "include/instruction.hpp"
#include "include/algorithm.hpp"
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "include/experiments.hpp"
#include <nlohmann/json.hpp>
#include "src/experiments/bitflip.cpp"
#include "src/experiments/bellstate_reach.cpp"
#include "src/experiments/reset.cpp"
#include "src/experiments/discrimination.cpp"

using namespace std;
namespace fs = std::filesystem;

unordered_set<shared_ptr<Instruction>, InstructionHash, InstructionPtrEqual> get_unique_instructions(const vector<shared_ptr<POMDPAction>> &actions) {
    unordered_set<shared_ptr<Instruction>, InstructionHash, InstructionPtrEqual> result;
    for (auto action : actions) {
        for (auto instruction : action->instruction_sequence) {
            result.insert(make_shared<Instruction>(instruction));
        }
    }
    return result;
}

class Setup {
public:
    string name;
    int num_batches;
    shared_ptr<Experiment> experiment;
    bool with_cnot;
    int max_horizon;
    bool with_thermalization;
    Setup(string name_, int num_batches_, shared_ptr<Experiment> exp_, bool with_cnot_, int max_horizon_, bool with_thermalization_=false) {
        this->name = std::move(name_);
        this->with_thermalization = with_thermalization_;
        this->num_batches = num_batches_;
        this->experiment = std::move(exp_);
        this->with_cnot = with_cnot_;
        this->max_horizon = max_horizon_;
    }
};

int get_algorithm_index(const Algorithm &current_algorithm, const bool &is_convex, const vector<pair<Algorithm, bool>> &unique_algorithms) {
    for (int i = 0; i < unique_algorithms.size(); i++) {
        if (unique_algorithms[i].first == current_algorithm && unique_algorithms[i].second == is_convex) {
            return i;
        }
    }
    return -1;
}

bool are_actions_valid(const HardwareSpecification &hardware_spec, const vector<shared_ptr<POMDPAction>> &actions) {
    for (auto action : actions) {
        for (auto instruction : action->instruction_sequence) {
            if (hardware_spec.instructions_to_channels.find(make_shared<Instruction>(instruction)) == hardware_spec.instructions_to_channels.end()) {
                return false;
            }
        }
    }

    return true;
}

void generate_improvements_file(const Setup &setup, vector<pair<Algorithm, bool>> unique_algorithms, const map<QuantumHardware, HardwareSpecification> &qw_to_spec, map<QuantumHardware, map<string, map<int, map<int, string>>>> embedding_to_opt_prob) {
    filesystem::path parsed_results_path = fs::path("..") /"parsed_results";
    string therm_str;
    if (setup.with_thermalization) {
        therm_str = "_therm";
    }
    fs::path exp_dir = parsed_results_path / (setup.name + therm_str);
    fs::path improvements_path = exp_dir / "improvements.csv";
    ofstream improvements_file(improvements_path);
    improvements_file << join(vector<string>({
    "hardware",
        "embedding_index",
        "method",
        "horizon",
        "algorithm_index",
        "baseline_index",
        "algorithm_prob",
        "baseline_prob",
        "diff",
    }), ",") << "\n";

    for (int batch = 0; batch < setup.num_batches; batch++) {
            fs::path raw_exp_path = fs::path("..") / "results" / (setup.name + "_" + to_string(batch) + therm_str);
            ifstream f(raw_exp_path / "stats.csv");

            string line;
            getline(f, line);

            while (getline(f, line)) {
                vector<string> tokens;
                split_str(line, ',', tokens);
                string quantum_hardware = tokens[0];
                int embedding_index = stoi(tokens[1]);
                int horizon = stoi(tokens[2]);
                // string pomdp_build_time = tokens[3];
                string probability = tokens[4];
                string str_method = tokens[5];
                int algorithm_index_ = stoi(tokens[7]);

                std::ifstream curr_alg_file(raw_exp_path / "raw_algorithms" / ("R_" + to_string(algorithm_index_+1) + ".txt"));
                json current_algorithm;
                curr_alg_file >> current_algorithm;
                curr_alg_file.close();
                Algorithm alg_object(current_algorithm);

                MethodType method;
                bool is_convex;
                if (str_method == "bellman") {
                    method = MethodType::SingleDistBellman;
                    is_convex = false;
                } else {
                    assert(str_method == "convex");
                    method = MethodType::ConvexDist;
                    is_convex = true;
                }



                int algorithm_index = get_algorithm_index(alg_object, is_convex, unique_algorithms);
                auto baseline_algorithm = setup.experiment->get_textbook_algorithm(method, horizon);

                int baseline_index = get_algorithm_index(*baseline_algorithm, is_convex, unique_algorithms);
                if (algorithm_index == -1 or baseline_index == -1) {
                    cout << "algorithms not found ("<< algorithm_index << ", " << baseline_index << ")" << endl;
                    cout << quantum_hardware << " " << embedding_index << " " << horizon << endl;
                    if (algorithm_index ==-1) {
                        cout << to_string(make_shared<Algorithm>(alg_object)) << endl;
                    }
                    if (baseline_index == -1) {
                        cout << to_string(baseline_algorithm) << endl;
                    }
                    assert(false);
                }

                assert(unique_algorithms[baseline_index].second == unique_algorithms[algorithm_index].second);

                QuantumHardware qw = to_quantum_hardware(quantum_hardware);
                auto spec = qw_to_spec.at(qw);
                auto embeddings = setup.experiment->get_hardware_scenarios(spec);
                auto embedding = embeddings[embedding_index];
                POMDP pomdp(setup.experiment->precision);
                auto actions = setup.experiment->get_actions(spec, embedding);
                auto initial_distribution = setup.experiment->get_initial_distribution(embedding);
                auto qubits_used = setup.experiment->get_qubits_used(embedding);
                auto guard = [&setup](const shared_ptr<POMDPVertex>& v, const std::unordered_map<int,int>& m, const shared_ptr<POMDPAction>& a) {
                    return setup.experiment->guard(v, m, a);
                };
                pomdp.build_pomdp(actions, spec, setup.max_horizon, embedding, nullptr, initial_distribution, qubits_used, guard, setup.experiment->set_hidden_index);
                double algorithm_prob = stod(embedding_to_opt_prob[qw][str_method][horizon][embedding_index]);
                auto baseline_prob = to_double(precise_verify_algorithm(pomdp, *setup.experiment, *baseline_algorithm, embedding, is_convex, setup.max_horizon));
                improvements_file << join(vector<string>({
                    quantum_hardware,
                        to_string(embedding_index),
                        str_method,
                    to_string(horizon),
                        to_string(algorithm_index),
                        to_string(baseline_index),
                        to_string(round_to(algorithm_prob, 5)),
                        to_string(round_to(baseline_prob,5)),
                        to_string(round_to(algorithm_prob-baseline_prob, 5)),
                    }), ",") << "\n";

            }

        }


    improvements_file.close();
}

int main(int argc, char* argv[]) {
    // load all hardware specs
    cout << "started loading hardware specs" << endl;
    map<bool, vector<HardwareSpecification>> all_specs;
    map<QuantumHardware, HardwareSpecification> qw_to_spec;
    all_specs[false] = vector<HardwareSpecification>({});
    all_specs[true] = vector<HardwareSpecification>({});
    for (auto index = 0; index < QuantumHardware::HardwareCount; index++) {
        auto qw = static_cast<QuantumHardware>(index);
        auto hs = HardwareSpecification(qw, false, true);
        assert(qw_to_spec.find(qw) == qw_to_spec.end());
        qw_to_spec.insert({qw, hs});
        all_specs[false].push_back(hs);
        // all_specs[true].push_back(HardwareSpecification(static_cast<QuantumHardware>(index), true, true));
    }
    cout << "end loading hardware specs" << endl << endl;

    filesystem::path parsed_results_path = fs::path("..") /"parsed_results";

    if (!fs::exists(parsed_results_path)) {
        fs::create_directory(parsed_results_path);
    }

    vector<Setup> setups({
        // Setup("basic_zero_plus_discr", 10, make_shared<BasicZeroPlusDiscrimination>(), false, 6),
        // Setup("ghz3", 5, make_shared<GHZStatePreparation3>(), true, 3),
        // Setup("bitflip_ipma2", 50, make_shared<IPMA2Bitflip>(), true, 8, false),
        // Setup("bitflip_ipma", 20, make_shared<IPMABitflip>(), true, 7),
        // Setup("bitflip_cxh", 20, make_shared<CXHBitflip>(), true, 7),
        // Setup("bell_state_reach", 1, make_shared<BellStateReach>(), true, 6),
        Setup("reset", 2, make_shared<ResetProblem>(), false, 9, false),
    });

    for (auto setup : setups) {
        string therm_str;
        if (setup.with_thermalization) {
            therm_str = "_therm";
        }
        cout << "parsing experiment " << setup.experiment->name << endl;
        fs::path exp_dir = parsed_results_path / (setup.name + therm_str);
        fs::create_directories(exp_dir);

        fs::path parsed_algorithms_path = exp_dir / "raw_algorithms";
        fs::create_directories(parsed_algorithms_path);

        vector<pair<Algorithm, bool>> unique_algorithms;

        fs::path parsed_stats_path = exp_dir / "stats.csv";
        ofstream parsed_stats_file(parsed_stats_path);
        parsed_stats_file << join(vector<string>({"hardware",
        "embedding_index",
        "horizon",
        "pomdp_build_time",
        "probability",
        "method",
        "method_time",
        "algorithm_index"})
        , ",") << "\n";

        map<QuantumHardware, map<string, map<int, map<int, string>>>> embedding_to_opt_prob;
        for (int batch = 0; batch < setup.num_batches; batch++) {
            fs::path raw_exp_path = fs::path("..") / "results" / (setup.name + "_" + to_string(batch) + therm_str);
            ifstream f(raw_exp_path / "stats.csv");

            string line;
            getline(f, line);

            while (getline(f, line)) {
                vector<string> tokens;
                split_str(line, ',', tokens);
                string quantum_hardware = tokens[0];
                string embedding_index = tokens[1];
                string horizon = tokens[2];
                string pomdp_build_time = tokens[3];
                string probability = tokens[4];
                string method = tokens[5];
                string method_time = tokens[6];
                int algorithm_index = stoi(tokens[7]);
                if (embedding_to_opt_prob.find(to_quantum_hardware(quantum_hardware)) == embedding_to_opt_prob.end()) {
                    embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)] = {};
                }

                if(embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)].find(method) == embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)].end()) {
                    embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method] = {};
                }
                if(embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method].find(stoi(horizon)) == embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method].end()) {
                    embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method][stoi(horizon)] = {};
                }
                assert(embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method][stoi(horizon)].find(stoi(embedding_index)) == embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method][stoi(horizon)].end());
                embedding_to_opt_prob[to_quantum_hardware(quantum_hardware)][method][stoi(horizon)][stoi(embedding_index)] = probability;
                std::ifstream curr_alg_file(raw_exp_path / "raw_algorithms" / ("R_" + to_string(algorithm_index+1) + ".txt"));
                if (!curr_alg_file.is_open()) {
                    std::cerr << "Error opening file\n";
                    return 1;
                }

                bool is_current_convex = false;
                if (method == "convex") {
                    is_current_convex = true;
                } else {
                    assert(method == "bellman");
                }

                json current_algorithm;
                curr_alg_file >> current_algorithm;
                curr_alg_file.close();
                Algorithm alg_object(current_algorithm);
                int real_index = get_algorithm_index(alg_object, is_current_convex, unique_algorithms);


                if (real_index == -1) {
                    real_index = unique_algorithms.size();
                    unique_algorithms.emplace_back(alg_object, is_current_convex);
                    // dump algorithm
                    dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(real_index) + ".txt"), make_shared<Algorithm>(alg_object));
                    dump_to_file(parsed_algorithms_path / ("A_" + to_string(real_index) + ".txt"), make_shared<Algorithm>(alg_object));
                }
                algorithm_index = real_index;

                parsed_stats_file << join(vector<string>({quantum_hardware,
                embedding_index,
                horizon,
                pomdp_build_time,
                probability,
                method,
                method_time,
                to_string(algorithm_index)})
                , ",") << "\n";
            }

        }
        parsed_stats_file.close();
        if (setup.name == "reset" || setup.name == "bitflip_ipma" || setup.name == "bitflip_ipma2" || setup.name == "basic_zero_plus_discr" || setup.name == "bell_state_reach") {
            int min_horizon;

            if (setup.name == "basic_zero_plus_discr" || setup.name == "bell_state_reach") {
                min_horizon = 1;
            } else if (setup.name == "reset") {
                min_horizon = 1;
            } else {
                min_horizon = 3;
            }
            vector<MethodType> methods = {MethodType::SingleDistBellman};
            if (setup.name == "bell_state_reach") {
                methods.push_back(MethodType::ConvexDist);
            } else if (setup.name == "basic_zero_plus_discr") {
                methods.clear();
                methods.push_back(MethodType::ConvexDist);
            }


            for (auto m : methods) {
                bool is_current_convex = false;
                if (m == MethodType::ConvexDist) {
                    is_current_convex = true;
                } else {
                    assert (m == MethodType::SingleDistBellman);
                }
                for (int h = min_horizon; h <= setup.max_horizon; h++) {

                    if (setup.name == "reset" && h < 8) {
                        auto m_ = ConvexDist;
                        auto textbook_alg = setup.experiment->get_textbook_algorithm(m_, h);
                        int real_index = get_algorithm_index(*textbook_alg, true, unique_algorithms);
                        if (real_index == -1) {
                            real_index = unique_algorithms.size();
                            unique_algorithms.emplace_back(*textbook_alg, true);
                            // dump algorithm
                            dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(real_index) + ".txt"), textbook_alg);
                            dump_to_file(parsed_algorithms_path / ("A_" + to_string(real_index) + ".txt"), textbook_alg);
                        }
                        cout <<"[convex] textbook alg at h=" << h << " is index " << real_index << endl;

                    }
                    if (setup.name != "reset" || h > 1) {
                        auto textbook_alg = setup.experiment->get_textbook_algorithm(m, h);
                        int real_index = get_algorithm_index(*textbook_alg, is_current_convex, unique_algorithms);
                        if (real_index == -1) {
                            real_index = unique_algorithms.size();
                            unique_algorithms.emplace_back(*textbook_alg, is_current_convex);
                            // dump algorithm
                            dump_raw_algorithm(parsed_algorithms_path / ("R_" + to_string(real_index) + ".txt"), textbook_alg);
                            dump_to_file(parsed_algorithms_path / ("A_" + to_string(real_index) + ".txt"), textbook_alg);
                        }
                        cout <<"[" <<get_method_string(m)<< "] textbook alg at h=" << h << " is index " << real_index << endl;
                    }

                }
            }

        }

        generate_improvements_file(setup, unique_algorithms, qw_to_spec, embedding_to_opt_prob);
        // test all algorithms in all hardware specifications
        fs::path verification_path = exp_dir / "verification.csv";
        ofstream verification_file(verification_path);
        verification_file << join(vector<string>({"hardware",
        "embedding_index",
        "algorithm_index",
        "probability",
        "method"})
        , ",") << "\n";

        if (!verification_file.is_open()) {
            std::cerr << "Error opening verification file " << verification_path << endl;
            return 1;
        }
        for (auto &hardware_spec : all_specs[setup.with_thermalization]) {
            if ((setup.with_cnot && hardware_spec.basis_gates_type != BasisGates::TYPE5 && hardware_spec.basis_gates_type != BasisGates::TYPE2) or !setup.with_cnot) {
                auto embeddings = setup.experiment->get_hardware_scenarios(hardware_spec);
                for (int embedding_index = 0; embedding_index < embeddings.size(); ++embedding_index) {
                    cout << "testing hardware spec " << hardware_spec.get_hardware_name() << ", embedding " << (embedding_index+1) << "/ " << embeddings.size() << endl;
                    auto embedding = embeddings[embedding_index];
                    POMDP pomdp(setup.experiment->precision);
                    auto actions = setup.experiment->get_actions(hardware_spec, embedding);
                    auto initial_distribution = setup.experiment->get_initial_distribution(embedding);
                    auto qubits_used = setup.experiment->get_qubits_used(embedding);
                    auto guard = [&setup](const shared_ptr<POMDPVertex>& v, const std::unordered_map<int,int>& m, const shared_ptr<POMDPAction>& a) {
                        return setup.experiment->guard(v, m, a);
                    };
                    pomdp.build_pomdp(actions, hardware_spec, setup.max_horizon, embedding, nullptr, initial_distribution, qubits_used, guard, setup.experiment->set_hidden_index);
                    for (int algorithm_index = 0; algorithm_index < unique_algorithms.size(); ++algorithm_index) {

                        auto unique_algorithm_pair = unique_algorithms[algorithm_index];
                        auto unique_algorithm = unique_algorithm_pair.first;
                        string method = "bellman";
                        if (unique_algorithm_pair.second) {
                            method = "convex";
                        }

                        double probability = 0.0;

                        probability = verify_algorithm(pomdp, *setup.experiment, unique_algorithm, hardware_spec, embeddings[embedding_index], unique_algorithm_pair.second, setup.max_horizon);

                        verification_file << join(vector<string>({hardware_spec.get_hardware_name(),
                            to_string(embedding_index),
                            to_string(algorithm_index),
                            to_string(round_to(probability, 5)),
                            method})
                            , ",") << "\n";
                    }
                    verification_file.flush();
                }
            }
        }
        verification_file.close();

        // dump hardware specifications
        fs::path unitary_specs = exp_dir / "unitary_specs.csv";
        ofstream unitary_specs_file(unitary_specs);
        unitary_specs_file << join(vector<string>({"hardware",
        "embedding_index",
        "gate_name",
        "probability"})
        , ",") << "\n";

        fs::path measurement_specs = exp_dir / "measurement_specs.csv";
        ofstream measurement_specs_file(measurement_specs);
        measurement_specs_file << join(vector<string>({"hardware",
        "embedding_index",
        "meas0_sucess",
        "meas1_sucess"})
        , ",") << "\n";

        for (auto &hardware_spec : all_specs[setup.with_thermalization]) {
            if ((setup.with_cnot && hardware_spec.basis_gates_type != BasisGates::TYPE5 && hardware_spec.basis_gates_type != BasisGates::TYPE2) or !setup.with_cnot) {
                auto embeddings = setup.experiment->get_hardware_scenarios(hardware_spec);
                for (int embedding_index = 0; embedding_index < embeddings.size(); ++embedding_index) {
                    auto embedding = embeddings[embedding_index];
                    auto actions = setup.experiment->get_actions(hardware_spec, embedding);
                    auto unique_instructions = get_unique_instructions(actions);
                    for (auto instruction : unique_instructions) {
                        if (instruction->instruction_type == InstructionType::Measurement) {
                            shared_ptr<MeasurementChannel> meas_channel = static_pointer_cast<MeasurementChannel>(hardware_spec.get_channel(instruction));
                            measurement_specs_file << join(vector<string>({hardware_spec.get_hardware_name(),
                                to_string(embedding_index),
                                to_string(meas_channel->get_ind_probability(0,0)),
                                to_string(meas_channel->get_ind_probability(1,1)),})
                                , ",") << "\n";
                        } else if (instruction->instruction_type == InstructionType::UnitaryMultiQubit || instruction->instruction_type == InstructionType::UnitarySingleQubit) {
                            shared_ptr<QuantumChannel> quantum_channel = static_pointer_cast<QuantumChannel>(hardware_spec.get_channel(instruction));
                            unitary_specs_file << join(vector<string>({hardware_spec.get_hardware_name(),
                        to_string(embedding_index),
                        gate_to_string(instruction->gate_name),
                        to_string(quantum_channel->estimated_success_prob)})
                        , ",") << "\n";
                        }
                    }
                }
            }
        }

        unitary_specs_file.close();
        measurement_specs_file.close();

    }

    return 0;
}
