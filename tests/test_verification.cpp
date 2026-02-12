//
// Created by Stefanie Muroya Lei on 02.01.26.
//

#include <gtest/gtest.h>
#include "verifier.hpp"
#include "grammars/assertions/AssertionLexer.h"
#include "grammars/assertions/AssertionParser.h"
#include "grammars/assertions/PreconditionAssertionLexer.h"
#include "grammars/assertions/PreconditionAssertionParser.h"
#include "grammars/assertions/PreconditionAssertionBaseVisitor.cpp"
#include "grammars/PL/ProgrammingLanguageLexer.h"
#include "src/experiments/reset.cpp"
#include "src/experiments/ghz.cpp"
#include "src/experiments/bellstate_reach.cpp"
#include "src/experiments/bitflip.cpp"
#include "src/experiments/discrimination.cpp"

vector<shared_ptr<Experiment>> get_experiments () {
    ResetProblem reset_problem;
    reset_problem.name = "reset";

    IPMABitflip ipma_problem;
    ipma_problem.name = "bitflip_ipma";

    IPMA2Bitflip ipma2_problem;
    ipma2_problem.name = "bitflip_ipma2";

    CXHBitflip cxh_problem;
    cxh_problem.name = "bitflip_cxh";

    BellStateReach bell_reach_problem;
    bell_reach_problem.name = "bell_state_reach";

    BasicZeroPlusDiscrimination discr_problem;
    discr_problem.name = "basic_zero_plus_discr";

    GHZStatePreparation3 ghz_problem;
    ghz_problem.name = "ghz3";

    vector<shared_ptr<Experiment>> experiments;
    experiments.push_back(make_shared<IPMABitflip>(ipma_problem));
    experiments.push_back(make_shared<BellStateReach>(bell_reach_problem));
    experiments.push_back(make_shared<BasicZeroPlusDiscrimination>(discr_problem));
    experiments.push_back(make_shared<ResetProblem>(reset_problem));
    experiments.push_back(
        make_shared<GHZStatePreparation3>(ghz_problem)
    );
    return experiments;
}

vector<MethodType> get_exp_methods(shared_ptr<Experiment> &experiment) {
    if (experiment->name == "reset" || experiment->name == "bell_state_reach") {
        return {MethodType::ConvexDist, MethodType::SingleDistBellman};
    }

    if (experiment->name == "ghz3" || experiment->name == "bitflip_ipma" || experiment->name == "bitflip_ipma2" || experiment->name == "bitflip_cxh") {
        return {MethodType::SingleDistBellman};
    }
    assert(experiment->name == "basic_zero_plus_discr");
    return {MethodType::ConvexDist};
}

// TEST(AllExperiments, ProgramParser) {
//     vector<pair<string, int>> problems({
//         make_pair("reset", 160),
//         make_pair("basic_zero_plus_discr", 283),
//         make_pair("bell_state_reach", 18),
//         make_pair("bitflip_cxh", 6),
//         make_pair("bitflip_ipma", 26),
//         make_pair("bitflip_ipma2", 39),
//         make_pair("ghz3", 11)
//     });
//     for (auto problem: problems) {
//         for (int i = 0; i < problem.second + 1; i++) {
//             std::ifstream curr_alg_file(get_final_wd(problem.first) / "raw_algorithms" / ("R_" + to_string(i) + ".txt"));
//             json current_algorithm;
//             curr_alg_file >> current_algorithm;
//             curr_alg_file.close();
//             Algorithm alg_object(current_algorithm);
//
//             auto raw_program = v_to_string(make_shared<Algorithm>(alg_object));
//             antlr4::ANTLRInputStream input(raw_program);
//             ProgrammingLanguageLexer lexer(&input);
//             antlr4::CommonTokenStream tokens(&lexer);
//             ProgrammingLanguageParser parser(&tokens);
//             EXPECT_EQ(parser.getNumberOfSyntaxErrors(), 0);
//         }
//     }
// }

// TEST(AllExperiments, ProgramDepth) {
//     auto experiments = get_experiments();
//     for (auto experiment : experiments) {
//         auto f = StatsFile(experiment->name, *experiment);
//         unordered_set<int> processed_algs;
//         for (auto row : f.stats) {
//             if (processed_algs.find(row.algorithm_index) == processed_algs.end()) {
//                 cout << "experiment " << experiment->name << " " << to_string(row.quantum_hardware) << " " << row.horizon << " " << row.embedding_index << endl;
//                 processed_algs.insert(row.algorithm_index);
//
//                 // process algorithm
//                 auto raw_program = v_to_string(make_shared<Algorithm>(row.algorithm));
//                 // cout << to_string(make_shared<Algorithm>(row.algorithm)) << endl;
//                 antlr4::ANTLRInputStream input(raw_program);
//                 ProgrammingLanguageLexer lexer(&input);
//                 antlr4::CommonTokenStream tokens(&lexer);
//                 ProgrammingLanguageParser parser(&tokens);
//                 antlr4::tree::ParseTree *tree = parser.program();
//                 auto *program = dynamic_cast<ProgrammingLanguageParser::ProgramContext*>(tree);
//
//                 // some ensemble (it doesn't matter for computing the depth)
//                 Ensemble<double> some_ensemble;
//
//                 // Markov Chain
//                 HardwareSpecification spec(row.quantum_hardware, false, true);
//                 MarkovChain mc(spec, row.embedding, 8);
//                 int depth = MarkovChain::get_depth(make_shared<Configuration>(program, make_shared<Ensemble<double>>(some_ensemble)));
//                 EXPECT_LE(depth, row.horizon+2);
//
//                 // cout << "********" << endl;
//             }
//         }
//     }
// }

// TEST(AllExperiments, Precondition) {
//     auto experiments = get_experiments();
//     for (auto experiment : experiments) {
//         auto methods = get_exp_methods(experiment);
//         for (auto method: methods) {
//             // cout << experiment->name << " " << gate_to_string(method) << endl;
//             auto precondition = experiment->get_precondition(method);
//             // cout << precondition << endl;
//             antlr4::ANTLRInputStream prec_input(precondition);
//             PreconditionAssertionLexer prec_lexer(&prec_input);
//             antlr4::CommonTokenStream prec_tokens(&prec_lexer);
//             PreconditionAssertionParser prec_parser(&prec_tokens);
//             antlr4::tree::ParseTree* tree = prec_parser.precon_assertion();
//             // std::cout << tree->toStringTree(&prec_parser) << std::endl;
//             EXPECT_EQ(prec_parser.getNumberOfSyntaxErrors(), 0);
//             // cout << "*******" << endl;
//         }
//
//     }
// }
//
// TEST(AllExperiments, Postcondition) {
//     auto experiments = get_experiments();
//     for (auto experiment : experiments) {
//         auto postcondition = experiment->get_target_postcondition(0.75);
//         // cout << experiment->name << endl;
//         // cout << postcondition << endl;
//         antlr4::ANTLRInputStream postcon_input(postcondition);
//         AssertionLexer post_lexer(&postcon_input);
//         antlr4::CommonTokenStream post_tokens(&post_lexer);
//         AssertionParser post_parser(&post_tokens);
//         antlr4::tree::ParseTree* tree = post_parser.assertion();
//         // std::cout << tree->toStringTree(&post_parser) << std::endl;
//         EXPECT_EQ(post_parser.getNumberOfSyntaxErrors(), 0);
//         // cout << "*******" << endl;
//     }
// }
//
// TEST(ResetExp, InitialEnsembleBellman) {
//     ResetProblem reset_problem;
//     reset_problem.name = "reset";
//     assert(reset_problem.nqvars == 1);
//     assert(reset_problem.ncvars == 1);
//     unordered_map<int, int> embedding;
//     embedding[0] = 0;
//
//     for (auto method : {MethodType::ConvexDist, MethodType::SingleDistBellman}) {
//         // parse precondition
//         auto raw_precondition = reset_problem.get_precondition(method);
//         antlr4::ANTLRInputStream prec_input(raw_precondition);
//         PreconditionAssertionLexer prec_lexer(&prec_input);
//         antlr4::CommonTokenStream prec_tokens(&prec_lexer);
//         PreconditionAssertionParser prec_parser(&prec_tokens);
//         antlr4::tree::ParseTree *raw_prec = prec_parser.precon_assertion();
//         PreconVisitor visitor(reset_problem.nqvars, reset_problem.ncvars, 8, embedding);
//         auto polygons = std::any_cast<vector<Polygon<double>>>(visitor.visit(raw_prec));
//         assert(polygons.size() == 1);
//         if (method == MethodType::SingleDistBellman) {
//             assert(polygons[0].corners.size() == 1);
//         } else {
//             assert (method == MethodType::ConvexDist);
//             assert(polygons[0].corners.size() == 2);
//         }
//     }
// }
//
TEST(AllExperiments, GenerateVerifyFile) {
    auto experiments = get_experiments();
    for (auto experiment : experiments) {
        cout << experiment->name << endl;
        experiment->verify();
    }
}

Algorithm get_perfect_hard_algorithm(const Experiment &experiment, int horizon) {
    StatsFile sf(experiment.name, experiment);
    for (auto line : sf.stats) {
        if (line.quantum_hardware == QuantumHardware::PerfectHardware and line.horizon == horizon) {
            std::ifstream curr_alg_file(
                experiment.get_final_wd() / "raw_algorithms" / ("R_" + to_string(line.algorithm_index) + ".txt"));
            json current_algorithm;
            curr_alg_file >> current_algorithm;
            curr_alg_file.close();
            return Algorithm(current_algorithm);
        }
    }
    throw runtime_error("No perfect hardware algorithm found for " + experiment.name + " and horizon=" + to_string(horizon));
}

// TEST(Discr, PerfectHardware) {
//     HardwareSpecification perfect_hardware(QuantumHardware::PerfectHardware, false, true);
//     unordered_map<int, int> embedding;
//     embedding[0] = 0;
//     embedding[1] = 1;
//     BasicZeroPlusDiscrimination problem;
//     problem.name = "basic_zero_plus_discr";
//     assert(problem.nqvars == 2);
//     assert(problem.ncvars == 1);
//     auto threshold = 0.75;
//     auto precondition = problem.get_precondition(MethodType::ConvexDist);
//     auto algorithm_ = make_shared<Algorithm>(get_perfect_hard_algorithm(problem, 3));
//     cout << to_string(algorithm_) << endl;
//     auto algorithm = v_to_string(algorithm_);
//     auto postcondition = problem.get_target_postcondition(threshold);
//
//     auto verifier = Verifier(perfect_hardware, embedding, problem.nqvars, problem.ncvars, 8);
//     auto is_sat = verifier.verify(precondition,  algorithm, postcondition);
//     cout << "sat:" << is_sat << endl;
// }
//
// TEST(BitflipIPMA, PerfectHardware) {
//     HardwareSpecification perfect_hardware(QuantumHardware::PerfectHardware, false, true);
//
//     unordered_map<int, int> embedding;
//     for (int i = 0; i < 3; i++) {
//         embedding[i] = i;
//     }
//
//     IPMABitflip problem;
//     problem.name = "bitflip_ipma";
//     assert(problem.nqvars == 3);
//     assert(problem.ncvars == 1);
//
//     // auto initial_distribution = problem.get_initial_distribution(embedding);
//     // for (auto p : initial_distribution) {
//     //     cout << *p.first << " " << p.second << endl;
//     // }
//     //
//     // cout << "*** end initial distribution ***" << endl;
//
//     auto threshold = 1.00;
//     auto precondition = problem.get_precondition(MethodType::SingleDistBellman);
//     // cout << "****** precondition ******" << endl;
//     // cout << precondition << endl;
//     auto postcondition = problem.get_target_postcondition(threshold);
//     int horizon = 4;
//     auto algorithm_ = make_shared<Algorithm>(get_perfect_hard_algorithm(problem, horizon));
//     auto algorithm = v_to_string(algorithm_);
//     // cout << "****** algorithm ******" << endl;
//     // cout << to_string(algorithm_) << endl;
//
//     // cout << "****** postcondition ******" << endl;
//     // cout << postcondition << endl;
//
//     auto verifier = Verifier(perfect_hardware, embedding, problem.nqvars, problem.ncvars, 8);
//     auto is_sat = verifier.verify(precondition,  algorithm, postcondition);
//     cout << "sat:" << is_sat << endl;
// }
