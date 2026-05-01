

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
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    // 1. QExperiment name validation
    std::string command = result["command"].as<std::string>();
    string pomdp_name = result["name"].as<std::string>();

    if (command == "pomdps") {
        dump_pomdps();
        return 0;
    }

    if (command == "gen_f1") {
        MyFloat::precision = 15;
        generate_f1_benchmarks();
        return 0;
    }

    if (command == "abhsvi") {
        run_experiments(MethodType::Pareto);
        return 0;
    }

    if (command == "more_rocks") {
        MyFloat::precision = 15;
        run_exp_more_rocks(MethodType::Pareto);
        return 0;
    }

    if (command == "f1") {
        MyFloat::precision = 15;
        f1_run_experiments(MethodType::Pareto, pomdp_name);
        return 0;
    }

    if (command == "convexify") {
        MyFloat::precision = 15;
        run_convexify_sizes_experiment(pomdp_name);
        return 0;
    }

    if (command == "pomdp_to_python") {
        pomdps_to_python();
        return 0;
    }


    cout << "command not recognized" << endl;
    return 0;
}