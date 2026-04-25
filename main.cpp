

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
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    // 1. QExperiment name validation
    std::string command = result["command"].as<std::string>();

    if (command == "pomdps") {
        dump_pomdps();
        return 0;
    }

    if (command == "pareto") {
        run_experiments(MethodType::Pareto);
        return 0;
    }

    if (command == "f1_pareto") {
        run_experiments_f1(MethodType::Pareto);
        return 0;
    }

    if (command == "convexify") {
        // run_experiments(MethodType::ConvexPareto);
        run_convexify_sizes_experiment();
        return 0;
    }


    cout << "command not recognized" << endl;
    return 0;
}