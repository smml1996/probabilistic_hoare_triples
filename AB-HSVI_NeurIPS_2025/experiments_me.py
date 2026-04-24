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


def run_my_experiment(test: str, horizon: int, f_out) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}.txt", discount_factor, epsilon, os.path.join("Results", f"{test}.txt"),
                    max_t=horizon, f_out=f_out, f_name=test)

if __name__ == "__main__":
    f_out = open(os.path.join(results_path, "results.csv"), "w")
    f_out.write("benchmark,horizon,time,val\n")
    for test in tests:
        for horizon in horizons:
            print(f"\nStarting test {test} h={horizon}:\n", flush=True)
            run_my_experiment(test, horizon, f_out)
            f_out.flush()

    f_out.close()
