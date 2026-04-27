import AB_HSVI
import AB_HSVI_RockSample_experts
import os

# Experiment parameters
horizons = [i for i in range(1, 8)]
epsilons = [i for i in range(0, 8)]
discount_factor = 1.0

results_path = os.path.join("my_results")
tests_f1 = ["cit.POMDP",
            "fourth.POMDP",
            "mit.POMDP",
            "pentagon.POMDP",
            "sunysb.POMDP"]
# end experiment parameters


def run_abhsvi_benchmark(test: str, horizon: int, epsilon: float, f_out) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}", discount_factor, epsilon, os.path.join("Results", f"{test}.txt"),
                    max_t=horizon, f_out=f_out, f_name=test)

if __name__ == "__main__":
    f_out = open(os.path.join(results_path, "results_f1.csv"), "w")
    f_out.write("benchmark,horizon,epsilon,time,val\n")

    # f1 benchmarks
    for f1_test in tests_f1:
        for horizon in horizons:
            for n_states in range(1, 5):
                test = f"{f1_test}_{horizon}_{n_states}"
                for epsilon in epsilons:
                    print(f"\nStarting test {test} h={horizon} e={epsilon}:\n", flush=True)
                    run_abhsvi_benchmark(test, horizon, epsilon, f_out)
                    f_out.flush()


    f_out.close()
