

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
#include "src/experiments/discrimination.cpp"
#include "src/experiments/bellstate_reach.cpp"


using namespace std;
int main(int argc, char* argv[]) {
    generate_all_experiments_file();
    return 0;
    // Valid sets
    set<string> valid_experiments = {
        "ghz3",
        "ghz4",
        "bitflip_ipma",
        "bitflip_ipma2",
        "bitflip_ipma3",
        "bitflip_cxh",
        "reset",
        "bell_state_discr_ipma2",
        "bell_state_discr_ipma3",
        "basis_state_discr",
        "cbasis_state_discr",
        "basic_zero_plus_discr",
        "bell_state_reach"
    };
    string all_experiments_str;
    for (const auto& e : valid_experiments) {
        if (!all_experiments_str.empty()) {
            all_experiments_str += ", ";
        }
        all_experiments_str += e;
    }
    set<string> valid_methods = get_solver_methods_strings();
    set<string> valid_hardware = get_hardware_strings();

    cxxopts::Options options("main", "Synthesize quantum algorithms using POMDPs");

    options.add_options()
        ("experiment", "can be any of the following: " + all_experiments_str +".", cxxopts::value<std::string>())
        ("custom_name", "a directory will be created with this name in results/.", cxxopts::value<std::string>()->default_value(""))
        ("method", "can be any of the following: bellman, pbvi, convex.", cxxopts::value<std::vector<std::string>>())
        ("hardware", "Comma-separated list of hardware specs. Check hardware_specifications/ directory. E.g. almaden", cxxopts::value<std::string>()->default_value(""))
        ("min_horizon", "Minimum horizon", cxxopts::value<int>())
        ("max_horizon", "Maximum horizon", cxxopts::value<int>())
        ("precision", "precision of the POMDP", cxxopts::value<int>()->default_value("8"))
        ("with_thermalization", "Enable thermalization", cxxopts::value<bool>()->default_value("false"))
        ("optimize", "Optimize noise models", cxxopts::value<bool>()->default_value("false"))
        ("round_in_file", "All numbers in the generated files will be formatted to show no more than this number of decimal places.", cxxopts::value<int>()->default_value("5"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    Experiment::round_in_file =  result["round_in_file"].as<int>();

    // 1. Experiment name validation
    std::string experiment = result["experiment"].as<std::string>();
    if (!valid_experiments.count(experiment)) {
        throw std::invalid_argument("Invalid experiment: " + experiment);
    }

    // 1.1 custom name
    std::string custom_name = result["custom_name"].as<std::string>();

    // 2. Method validation
    auto raw_methods = result["method"].as<std::vector<std::string>>();
    set<MethodType> methods;
    for (auto& m : raw_methods) {
        if (!valid_methods.count(m)) {
            throw std::invalid_argument("Invalid method: " + m);
        }
        methods.insert(str_to_method_type(m));
    }

    // 3. Hardware validation (split by commas)
    std::string hw_string = result["hardware"].as<std::string>();
    set<QuantumHardware> hw_list;
    std::stringstream ss(hw_string);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!valid_hardware.count(item)) {
            throw std::invalid_argument("Invalid hardware spec: " + item);
        }
        hw_list.insert(to_quantum_hardware(item));
    }

    // 4 & 5. Horizon validation
    int min_horizon = result["min_horizon"].as<int>();
    int max_horizon = result["max_horizon"].as<int>();
    if (max_horizon < min_horizon) {
        throw std::invalid_argument("max_horizon must be >= min_horizon");
    }

    // 6. Optional arguments
    int precision = result["precision"].as<int>();
    bool with_thermalization = result["with_thermalization"].as<bool>();
    bool optimize = result["optimize"].as<bool>();


    if (experiment == "bitflip_ipma") {
        IPMABitflip bitflip_ipma = IPMABitflip(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bitflip_ipma.run();
    }
    else if (experiment == "bitflip_ipma2") {
        IPMA2Bitflip bitflip_ipma2 = IPMA2Bitflip(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bitflip_ipma2.run();
    }
    else if (experiment == "bitflip_ipma3") {
        IPMA3Bitflip bitflip_ipma3 = IPMA3Bitflip(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bitflip_ipma3.run();
    }
    else if (experiment == "bitflip_cxh") {
        CXHBitflip bitflip_cxh = CXHBitflip(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bitflip_cxh.run();
    }
    else if (experiment == "reset") {
        ResetProblem reset_problem = ResetProblem(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        reset_problem.run();
    }
    else if (experiment == "ghz3") {
        GHZStatePreparation3 ghz3_problem = GHZStatePreparation3(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        ghz3_problem.run();
    }
    else if (experiment == "ghz4") {
        throw::invalid_argument("ghz4 experiment not ready");
        GHZStatePreparation4 ghz3_problem = GHZStatePreparation4(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        ghz3_problem.run();

    } else if (experiment == "bell_state_discr_ipma2") {
        auto bell_state_discr_problem = BellStateDiscrimination2(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bell_state_discr_problem.run();
    }
    else if (experiment == "bell_state_discr_ipma3") {
        auto bell_state_discr_problem = BellStateDiscrimination3(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bell_state_discr_problem.run();
    }
    else if (experiment == "basis_state_discr") {
        auto basis_state_discr_problem = BasisStatesDiscrimination(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        basis_state_discr_problem.run();
    }
    else if (experiment == "basic_zero_plus_discr") {
        auto basic_zero_plus_discr = BasicZeroPlusDiscrimination(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        basic_zero_plus_discr.run();
    }
    else if (experiment == "cbasis_state_discr") {
        auto cbasis_state_discr_problem = CBasisStatesDiscrimination(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        cbasis_state_discr_problem.run();
    }
    else if (experiment == "bell_state_reach") {
        auto bell_state_reach_problem = BellStateReach(custom_name, precision, with_thermalization,min_horizon, max_horizon, methods, hw_list, optimize);
        bell_state_reach_problem.run();
    }
    else {
        throw std::invalid_argument("Invalid experiment: " + experiment);
    }
}