

#include <iostream>
#include <string>
#include <memory>

#include "solvers.hpp"
#include "include/cxxopts.hpp"
#include "include/experiments.hpp"
#include "src/experiments/bv.cpp"
#include "src/experiments/discrimination.cpp"
#include "src/experiments/qec.cpp"
#include "src/experiments/reset.cpp"
#include "src/experiments/rus.cpp"
#include "src/experiments/superdense_coding.cpp"


using namespace std;

inline LOGFile LOG;

int main(int argc, char* argv[]) {
    cxxopts::Options options("main", "me pomdps artifact");

    options.add_options()
        ("command", "command", cxxopts::value<string>()) // run or preview
        ("exp", "experiment name", cxxopts::value<string>()) // experiment name
        ("is_debug", "debug mode, sets horizons from 1 to 3")
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }

    // 1. QExperiment name validation
    std::string command = result["command"].as<std::string>();
    std::string exp = result["exp"].as<std::string>();
    bool is_debug = result.count("is_debug") > 0;

    if (is_debug) {
        cout << "Running in debug mode" << endl;
        Config::is_debug = true;
    } else {
        Config::is_debug = false;
    }
    LOG.open(fs::path("..")/"results" / ("log_"+ exp + "_" + command + "_" + to_string(is_debug) + ".txt"));
    LOG.write_debug_ln("");

    shared_ptr<QuantumExperiment> experiment;
    if (exp == "bv") {
        experiment = make_shared<BernsteinVazirani>(true);
    } else if (exp == "bv0") {// oracle without noise
        experiment = make_shared<BernsteinVazirani>(false);
    } else if (exp == "discr") {
        experiment = make_shared<ZeroPlusDiscrimination>();
    } else if (exp == "qec") {
        experiment = make_shared<ThreeQubitCode>();
    }
    else if (exp == "reset") {
        experiment = std::make_shared<ResetProblem>();
    }
    else if (exp == "rus") {
        experiment = make_shared<RUS>();
    }
    else if (exp == "superdense_cod") {
        experiment = make_shared<SuperdenseCoding>();
    }
    else if (exp == "superdense_cod0001") {
        experiment = make_shared<SuperdenseCoding0001>();
    }
    else if (exp == "superdense_cod0002") {
        experiment = make_shared<SuperdenseCoding0002>();
    }
    else if (exp == "superdense_cod0003") {
        experiment = make_shared<SuperdenseCoding0003>();
    }
    else if (exp == "superdense_cod0102") {
        experiment = make_shared<SuperdenseCoding0102>();
    }
    else if (exp == "superdense_cod0103") {
        experiment = make_shared<SuperdenseCoding0103>();
    }
    else if (exp == "superdense_cod0203") {
        experiment = make_shared<SuperdenseCoding0203>();
    }
    else {
        cerr << "Invalid experiment name" << endl;
        return 0;
    }

    experiment->init();

    if (command == "run") {
        experiment->run();
    }else if (command == "preview") {
        experiment->dump_preview();
    } else {
        cerr << "Invalid command" << endl;
    }
    LOG.close();
    return 0;
}