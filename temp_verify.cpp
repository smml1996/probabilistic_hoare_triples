//
// Created by Stefanie Muroya Lei on 09.10.26.
//


#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <memory>

#include "include/cxxopts.hpp"
#include "include/experiments.hpp"
#include "include/hardware_specification.hpp"
#include "src/experiments/bitflip.cpp"


using namespace std;

int main(int argc, char* argv[]) {
    IPMA2 bitflip_ipma2 = IPMA2("ipma2", {});
    fs::path raw_exp_path = fs::path("..") / "results" / ("ipma2_4");
    vector<shared_ptr<Algorithm> > unique_algorithms;

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
            if (quantum_hardware != "manhattan" || method_str == "bellman" || horizon != "5") continue;
            MethodType method = str_to_method_type(method_str);
            bitflip_ipma2.setup_params();
            bitflip_ipma2.set_min_max_horizon(method);
            double method_time = stod(tokens[6]);
            int algorithm_index = stoi(tokens[7]);
            int tot_strats = stoi(tokens[8]);
            cout << quantum_hardware << " / e=" << embedding_index << " / k="<<horizon<< " / m=" << method_str << endl;

            std::ifstream curr_alg_file(
                raw_exp_path / "raw_algorithms" / ("R_" + to_string(algorithm_index + 1) + ".txt"));
            if (!curr_alg_file.is_open()) {
                std::cerr << "Error opening file\n";
                return 1;
            }
            json current_algorithm;
            curr_alg_file >> current_algorithm;
            curr_alg_file.close();
            shared_ptr<Algorithm> algorithm = make_shared<Algorithm>(current_algorithm);
            auto real_index = get_algorithm_from_list(unique_algorithms, algorithm);


            if (real_index == -1) {
                real_index = unique_algorithms.size();
                unique_algorithms.push_back(algorithm);
            }
            algorithm_index = real_index;
            auto spec = HardwareSpecification(to_quantum_hardware(quantum_hardware), false, true);
            unordered_map<int, int> embedding = bitflip_ipma2.get_hardware_scenarios(spec)[stoi(embedding_index)];
            auto tot_ins = bitflip_ipma2.get_actions(spec, embedding).size();

            cout << "started verification" << endl;
            auto start_time = chrono::steady_clock::now();
            auto my_prob = bitflip_ipma2.verify(method, spec, algorithm, stod(probability), embedding);
            auto end_time = chrono::steady_clock::now();
            auto verify_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
            cout <<"end verification" << endl;

            auto textbook_alg = bitflip_ipma2.get_textbook_algorithm(method, stoi(horizon));
            auto baseline_probability = bitflip_ipma2.verify(method, spec, textbook_alg, -1, embedding);
            auto diff = round_to(my_prob - baseline_probability, Experiment::round_in_file-1);
            cout << quantum_hardware << " " << embedding_index << " " << probability << " " << my_prob << " " << baseline_probability << " " << diff << endl;
        }
    return 0;
}