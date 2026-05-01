import AB_HSVI
import math
import os

# Experiment parameters
horizons = [i for i in range(1, 8)]
epsilons = [i for i in range(0, 1)]
discount_factor = 1.0

results_path = os.path.join("my_results")
# end experiment parameters


def run_abhsvi_benchmark(test: str, horizon: int, epsilon: float, f_out) -> None:
    AB_HSVI.AB_HSVI(f"Models/{test}.txt", discount_factor, epsilon, os.path.join("Results", f"{test}_{horizon}_{epsilon}.txt"),
                    max_t=horizon, f_out=f_out, f_name=test)

if __name__ == "__main__":
    f_out = open(os.path.join(results_path, "results_more_rocks.csv"), "w")
    f_out.write("benchmark,horizon,epsilon,time,val\n")

    n = 3
    density_total_rocks = [0.3, 0.5, 0.75]
    density_good_rocks = [0.25, 0.5, 0.75]
    for dt in density_total_rocks:
        k = math.ceil(n*n * dt)
        for dg in density_good_rocks:
            g = math.ceil(dg*k)
            r = k + g
            for horizon in horizons:
                for epsilon in epsilons:
                    test = f"RockSample_POMDP_N{n}_G{g}_K{k}_R{r}_"
                    print(f"\nStarting test {test} h={horizon} e={epsilon}:\n", flush=True)
                    run_abhsvi_benchmark(test, horizon, epsilon, f_out)
                    f_out.flush()


    f_out.close()
