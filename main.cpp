

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
        ("precision", "optional floating point precision", cxxopts::value<int>()->default_value("15"))
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
    int precision = result["precision"].as<int>();
    MyFloat::precision = precision;

    if (command == "run") {
        f1_run_experiments(MethodType::Pareto, pomdp_name, max_horizon);
        return 0;
    }

    return 0;
}