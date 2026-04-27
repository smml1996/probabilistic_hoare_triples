import AB_HSVI
import AB_HSVI_RockSample_experts
import os

# Experiment parameters
horizons = [i for i in range(1, 8)]
epsilons = [i for i in range(0, 10)]
discount_factor = 1.0

results_path = os.path.join("my_results")
tests_abhsvi = ["RockSample_POMDP_N3_G1_K2_R18",
         "RockSample_POMDP_N3_G1_K3_R13",
         "RockSample_POMDP_N3_G1_K4_R83",
         "RockSample_POMDP_N3_G2_K3_R4",
         "RockSample_POMDP_N4_G1_K2_R33",
         "RockSample_POMDP_N5_G1_K2_R96",
         "RockSample_POMDP_N6_G1_K2_R77"]
assert(len(tests_abhsvi) == 7)
# end experiment parameters


def run_abhsvi_benchmark(test: str, horizon: int, epsilon: float, f_out) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}.txt", discount_factor, epsilon, os.path.join("Results", f"{test}_{horizon}_{epsilon}.txt"),
                    max_t=horizon, f_out=f_out, f_name=test)

if __name__ == "__main__":
    f_out = open(os.path.join(results_path, "results_abhsvi.csv"), "w")
    f_out.write("benchmark,horizon,epsilon,time,val\n")

    # ABHSVI benchmarks
    for test in tests_abhsvi:
        for horizon in horizons:
            for epsilon in epsilons:
                print(f"\nStarting test {test} h={horizon} e={epsilon}:\n", flush=True)
                run_abhsvi_benchmark(test, horizon, epsilon, f_out)
                f_out.flush()


    f_out.close()
