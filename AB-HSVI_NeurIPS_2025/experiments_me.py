import AB_HSVI
import AB_HSVI_RockSample_experts
import os

# Experiment parameters
horizons = [i for i in range(1, 5)]
epsilon = 0.0
discount_factor = 1.0

results_path = os.path.join("my_results")
tests = ["RockSample_POMDP_N3_G1_K2_R18",
         "RockSample_POMDP_N3_G1_K3_R13",
         "RockSample_POMDP_N3_G1_K4_R83",
         "RockSample_POMDP_N3_G2_K3_R4",
         "RockSample_POMDP_N4_G1_K2_R33",
         "RockSample_POMDP_N5_G1_K2_R96",
         "RockSample_POMDP_N6_G1_K2_R77"]
assert(len(tests) == 7)
# end experiment parameters


def run_my_experiment(test: str, horizon: int) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}.txt", discount_factor, epsilon, os.path.join(results_path, f"{test}.txt"),
                    max_t=horizon)

if __name__ == "__main__":
    for test in tests:
        for horizon in horizons:
            run_my_experiment(test, horizon)
