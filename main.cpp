

#include <iostream>
#include <string>
#include <memory>

#include "include/cxxopts.hpp"
#include "include/experiments.hpp"


using namespace std;
int main(int argc, char* argv[]) {
    cxxopts::Options options("main", "me pomdps artifact");

    options.add_options()
        ("command", "command", cxxopts::value<string>())
        ("name", "optional name", cxxopts::value<std::string>()->default_value(""))
        ("horizon", "optional max_horizon", cxxopts::value<int>()->default_value("6"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    // 1. QExperiment name validation
    std::string command = result["command"].as<std::string>();
    string pomdp_name = result["name"].as<std::string>();
    int max_horizon = result["horizon"].as<int>();

    if (command == "pomdps") {
        dump_pomdps();
        return 0;
    }

    if (command == "run") {
        MyFloat::precision = 15;
        f1_run_experiments(MethodType::Pareto, pomdp_name, max_horizon);
        return 0;
    }

    if (command == "pomdp_to_python") {
        pomdps_to_python();
        return 0;
    }


    cout << "command not recognized" << endl;
    return 0;
}