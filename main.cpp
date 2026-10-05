

#include <iostream>
#include <set>
#include <string>
#include <memory>

#include "include/cxxopts.hpp"
#include "include/experiments.hpp"
#include "include/hardware_specification.hpp"
#include "src/experiments/bitflip.cpp"
#include "src/experiments/ghz.cpp"
#include "src/experiments/reset.cpp"
#include "src/experiments/bellstate_reach.cpp"
#include "src/experiments/phase_reach.cpp"


using namespace std;

int main(int argc, char* argv[]) {
    cxxopts::Options options("main", "Synthesize quantum algorithms using POMDPs");

    options.add_options()
        ("run", "", cxxopts::value<std::string>())
        ("parse", "")
        ("custom_name", "a directory will be created with this name in results/.", cxxopts::value<std::string>()->default_value(""))
        ("hardware", "Comma-separated list of hardware specs. Check hardware_specifications/ directory. E.g. almaden", cxxopts::value<std::string>()->default_value(""))
        ("round_in_file", "All numbers in the generated files will be formatted to show no more than this number of decimal places.", cxxopts::value<int>()->default_value("5"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    Experiment::round_in_file =  result["round_in_file"].as<int>();

    // 1. Experiment name validation
    std::string experiment = result["run"].as<std::string>();

    // 1.1 custom name
    std::string custom_name = result["custom_name"].as<std::string>();

    // 3. Hardware validation (split by commas)
    std::string hw_string = result["hardware"].as<std::string>();
    set<QuantumHardware> hw_list;
    std::stringstream ss(hw_string);
    std::string item;
    while (std::getline(ss, item, ',')) {
        hw_list.insert(to_quantum_hardware(item));
    }

    cout << "running " << experiment << endl;


    Experiment::is_parse = result["parse"].as<bool>();

    if (experiment == "setup") {
        vector<shared_ptr<Experiment>> all_experiments{
            static_pointer_cast<Experiment>(make_shared<IPMA2>(IPMA2("ipma2", hw_list))),
            static_pointer_cast<Experiment>(make_shared<CXH>(CXH("cxh", hw_list))),
            static_pointer_cast<Experiment>(make_shared<ResetProblem>(ResetProblem("reset", hw_list))),
            static_pointer_cast<Experiment>(make_shared<GHZStatePrep>(GHZStatePrep("ghz", hw_list))),
            static_pointer_cast<Experiment>(make_shared<PhaseReach>(PhaseReach("lphase", hw_list))),
        };
        for (auto e : all_experiments) {
            e->generate_script();
        }
    } else if (experiment == "ipma2") {
        IPMA2 bitflip_ipma2 = IPMA2(custom_name, hw_list);
        bitflip_ipma2.run();
    } else if (experiment == "cxh") {
        CXH bitflip_cxh = CXH(custom_name, hw_list);
        bitflip_cxh.run();
    }
    else if (experiment == "reset") {
        ResetProblem reset_problem = ResetProblem(custom_name, hw_list);
        reset_problem.run();
    } else if (experiment == "ghz") {
        GHZStatePrep ghz_problem = GHZStatePrep(custom_name, hw_list);
        ghz_problem.run();
    // } else if (experiment == "lbell") {
    //     auto lbell_problem = BellStateReach(custom_name, hw_list);
    //     lbell_problem.run();
    } else if (experiment == "lphase") {
        auto lphase_problem = PhaseReach(custom_name, hw_list);
        lphase_problem.run();
    }  else {
        throw std::invalid_argument("Invalid experiment: " + experiment);
    }
}