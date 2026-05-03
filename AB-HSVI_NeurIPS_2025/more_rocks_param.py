import sys
import math
import AB_HSVI
import math
import os


# Experiment parameters
horizons = [4]
epsilons = [i for i in range(0, 1)]
discount_factor = 1.0

results_path = os.path.join("my_results")
# end experiment parameters


def run_abhsvi_benchmark(test: str, horizon: int, epsilon: float, f_out) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}", discount_factor, epsilon, os.path.join("Results", f"test_{test}_{horizon}_{epsilon}__.txt"),
                    max_t=horizon, f_out=f_out, f_name=test)

if __name__ == "__main__":
    test = sys.argv[1]

    f_out = open(os.path.join(results_path, f"test_more_rocks_{test}.csv"), "w")
    f_out.write("benchmark,horizon,epsilon,time,val\n")

    for horizon in horizons:
        for epsilon in epsilons:
            print(f"\nStarting test {test} h={horizon} e={epsilon}:\n", flush=True)
            run_abhsvi_benchmark(test, horizon, epsilon, f_out)
            f_out.flush()
    f_out.close()
