//
// Created by Stefanie Muroya Lei on 25.04.26.
//

#include <gtest/gtest.h>
#include "experiments.hpp"
#include "solvers.hpp"
string POMDPR18_name = "RockSample_POMDP_N3_G1_K2_R18.txt";

// TEST(POMDPR18, RewardInitialStates) {
//     POMDP pomdp(POMDPR18_name, POMDPFormat::ABHSVI);
//
//     EXPECT_EQ(pomdp.initial_states.size(), 2);
//     auto v0 = pomdp.initial_states[0];
//     EXPECT_EQ(v0->id, 1);
//
//     auto v1 = pomdp.initial_states[1];
//     EXPECT_EQ(v1->id, 2);
//
//     for (auto a : pomdp.actions) {
//         EXPECT_EQ(pomdp.get_reward(v0, a), 0.0);
//         EXPECT_EQ(pomdp.get_reward(v1, a), 0.0);
//     }
// }
//
// TEST(POMDPR18, Rewards) {
//     POMDP pomdp(POMDPR18_name, POMDPFormat::ABHSVI);
//
//     vector<vector<int>> rewards;
//
//     for (int state : {6,7,8, 15, 16, 17, 24, 25, 26}) {
//         vector<int> temp = {state, 2, 10};
//         rewards.emplace_back(temp);
//     }
//
//     for (int state : {11, 22}) {
//         vector<int> temp = {state, 4, 10};
//         rewards.emplace_back(temp);
//     }
//
//     for (int state : {9, 10, 21, 23}) {
//         vector<int> temp = {state, 4, -10};
//         rewards.emplace_back(temp);
//     }
//
//
//     for (auto t_r : rewards) {
//         auto state = pomdp.states[t_r[0]];
//         auto action = pomdp.actions[t_r[1]];
//         EXPECT_EQ(pomdp.get_reward(state, action).value, t_r[2]);
//     }
//
// }

TEST(POMDPF1_cit21, Solver) {
    MyFloat::precision = 6;
    int horizon = 2;
    bool convexify = false;
    POMDP pomdp("cit.POMDP_2_2", POMDPFormat::ABHSVI);

    ParetoSolver solver(pomdp, convexify);
    cout << solver.solve(pomdp.initial_states, horizon) << endl;
}