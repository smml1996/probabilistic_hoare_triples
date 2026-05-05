from enum import Enum
from math import isclose
from typing import Optional, List, Dict, Union, Set
import os
import pandas as pd
import math

pomdps_path = os.path.join("results", "pomdps.csv")

class FileType(Enum):
    abhsvi = 0
    ours = 1
    ours2 = 2


class POMDPData:
    benchmark: str
    num_states: int
    num_actions: int
    num_obs: int

    def __init__(self, line: str):
        tokens = line.split(',')
        self.benchmark = tokens[0]
        self.num_states = int(tokens[1])
        self.num_actions = int(tokens[2])
        self.num_obs = int(tokens[3])
        self.initial_states = int(tokens[4])

    def get_initial_states(self):
        assert(self.initial_states > 0)
        return self.initial_states


def get_all_rs_tests() -> List[str]:
    tests = [
        "RockSample_POMDP_N3_G1_K2_R18",
        "RockSample_POMDP_N3_G1_K3_R13",
        "RockSample_POMDP_N3_G1_K4_R83",
        "RockSample_POMDP_N3_G2_K3_R4",
        "RockSample_POMDP_N4_G1_K2_R33",
        "RockSample_POMDP_N5_G1_K2_R96",
        "RockSample_POMDP_N6_G1_K2_R77"
    ]
    n = 3
    density_total_rocks = [0.3, 0.5, 0.75]
    density_good_rocks = [0.25, 0.5, 0.75]
    for dt in density_total_rocks:
        k = math.ceil(n * n * dt)
        for dg in density_good_rocks:
            g = math.ceil(dg * k)
            r = k + g
            test = f"RockSample_POMDP_N{n}_G{g}_K{k}_R{r}_"
            tests.append(test)
    return tests

    return testcases

def get_all_robot_tests() -> List[str]:
    testcases = []
    for pomdp_name in ["cit.POMDP", "mit.POMDP", "pentagon.POMDP", "sunysb.POMDP"]:
        for i in range(1, 4):
            testcases.append(f"{pomdp_name}_{i}")

    return testcases


def get_all_iff_tests() -> List[str]:
    testcases = []

    # iff testcases
    visibilities = [0, 2, 4]
    for d1 in range(1, 4):
        for d2 in range(d1 + 1, 4):
            for v1 in visibilities:
                for v2 in visibilities:
                    if v1 != v2:
                        testcases.append(f"iff_{d1}_{d2}_{v1}_{v2}")
    return testcases

def get_all_f1_tests() -> List[str]:
    return get_all_robot_tests() + get_all_iff_tests()

def load_pomdps() -> Dict[str, POMDPData]:
    f = open(pomdps_path, "r")
    pomdps = {}
    for line in f.readlines()[1:]:
        pomdp = POMDPData(line[:-1])
        pomdps[pomdp.benchmark] = pomdp
    return pomdps

class Row:
    benchmark: str
    horizon: int
    time: Union[float, str]
    value: float
    initial_states: int
    size_to_convexify: Optional[int]
    file_type: FileType

    def __init__(self, line: str, file_type: FileType):
        self.file_type = file_type
        tokens = line.split(',')
        self.check_num_columns(tokens)

        self.benchmark = tokens[0]
        self.horizon = int(tokens[1])
        if file_type == FileType.abhsvi:
            self.time = "timeout" if float(tokens[-2]) >= 3600 else float(tokens[-2])
        elif file_type == FileType.ours2:
            self.time = "timeout" if float(tokens[2]) >= 3600 else float(tokens[2])
        else:
            assert(self.file_type == FileType.ours)
            self.time = "timeout" if float(tokens[-3]) >= 3600 else float(tokens[-3])
        self.value = float(tokens[-1])

        if self.time != "timeout" and isclose(self.value, -1):
            self.time = "error"

        self.initial_states = self.get_initial_states()
        self.size_to_convexify = self.get_size_to_convexify(tokens)

    def get_initial_states(self) -> int:
        pomdps = load_pomdps()
        if self.benchmark in pomdps.keys():
            return pomdps[self.benchmark].get_initial_states()
        return pomdps[self.benchmark + ".txt"].get_initial_states()

    def get_size_to_convexify(self, tokens: list[str]) -> int:
        return -1


    def check_num_columns(self, tokens):
        if self.file_type == FileType.ours:
            assert(len(tokens) == 5)
        elif self.file_type == FileType.ours2:
            assert (len(tokens) == 6)
        elif self.file_type == FileType.abhsvi:
            assert(len(tokens) == 4 or len(tokens) == 5)
        else:
            raise Exception("file type not supported", self.file_type)

    def __getitem__(self, key):
        return getattr(self, key)


def get_rows(file_path: str, file_type: FileType) -> List[Row]:
    f = open(file_path, "r")
    result = []
    for line in f.readlines()[1:]:
        result.append(Row(line[:-1], file_type))
    return result

def get_all_abhsvi_rs_lines() -> List[Row]:
    tests = get_all_rs_tests()

    rows = []
    for test in tests:
        results_path = os.path.join("AB-HSVI_NeurIPS_2025", "my_results", "unparsed", f"{test}.csv")
        rows += get_rows(results_path, FileType.abhsvi)
    return rows

def get_all_our_rs_lines() -> List[Row]:
    ABHSVI_RESULTS_PATH = os.path.join("results", "abhsvi.csv")
    MORE_ROCKS_RESULTS_PATH = os.path.join("results", "more_rocks.csv")
    return get_rows(ABHSVI_RESULTS_PATH, FileType.ours) + get_rows(MORE_ROCKS_RESULTS_PATH, FileType.ours)

def format_benchmark_name(name: str) -> str:
    return name.replace("POMDP_", "").replace("RockSample", "RS")

## Cassandra thesis pomdps
def get_all_f1_lines(is_ours: bool) -> List[Row]:
    f1_tests = get_all_f1_tests()

    rows = []
    for test in f1_tests:
        if is_ours:
            test_path = os.path.join("results", "unparsed", f"f1_{test}.csv")
            file_type = FileType.ours2
        else:
            test_path = os.path.join("AB-HSVI_NeurIPS_2025", "my_results", "unparsed", f"{test}.csv")
            file_type = FileType.abhsvi
        rows += get_rows(test_path, file_type)
    return rows


## utils for gap analysis
gap_index = 3
time_index = 4
def get_test_gaps(test: str, horizon: int) -> Set[float]:

    result = set()
    min_so_far = None
    name = f"{test}_{horizon}"
    if test[-1] == '_':
        name += "_0"
    # name += ".txt"
    f = open(os.path.join("AB-HSVI_NeurIPS_2025", "Results", name), "r")
    lines = f.readlines()

    for (index, line) in enumerate(lines):
        if len(line) == 1:
            break
        tokens = line.split('\t')
        assert(len(tokens) == 5)

        gap = float(tokens[gap_index])

        if min_so_far is None:
            min_so_far = gap
            result.add(gap)
        elif gap < min_so_far:
            result.add(gap)
        min_so_far = min(min_so_far, gap)


    f.close()
    return result

def get_all_gaps(rows: List[Row], max_horizon: int) -> Set[float]:
    result = set()
    for horizon in range(1, max_horizon + 1):
        for row in rows:
            gaps = get_test_gaps(row.benchmark, horizon)
            for g in gaps:
                result.add(g)
    return result

def get_gap_finish_time(row: Row, current_gap) -> float:
    test = row.benchmark
    horizon = row.horizon
    name = f"{test}_{horizon}"
    if test[-1] == '_':
        name += "_0"
    # name += ".txt"
    f = open(os.path.join("AB-HSVI_NeurIPS_2025", "Results", name), "r")
    lines = f.readlines()
    for (index, line) in enumerate(lines):
        if len(line) == 1:
            break
        tokens = line.split('\t')
        assert(len(tokens) == 5)

        if index+1 >= len(lines) or len(lines[index+1])  < 5:
            return row.time
        gap = float(tokens[gap_index])
        time = float(tokens[time_index])
        if math.isclose(current_gap, gap, abs_tol=1e-6, rel_tol=1e-6) or gap < current_gap:
            return time
    return row.time

def dump_gap_times(rows: List[Row], max_horizon: int, out_name: str) -> None:
    df_rows = []
    gaps = get_all_gaps(rows, max_horizon)
    gaps = sorted(gaps)
    for row in rows:
        finish_times = set()
        for gap in gaps:
            finish_times.add(get_gap_finish_time(row, gap))
        if len(finish_times) > 1:
            df_row = {
                'benchmark': row.benchmark,
                'horizon': row.horizon,
                'total_time': row.time
            }
            for gap in gaps:
                df_row[str(gap)] = get_gap_finish_time(row, gap)
            df_rows.append(df_row)

    df = pd.DataFrame(df_rows)
    df.to_csv(os.path.join("results", f"{out_name}.csv"), index=False)

def dump_useful_gaps(rows: List[Row], max_horizon: int, out_name: str):
    df_rows = []
    gaps = get_all_gaps(rows, max_horizon)
    gaps = sorted(gaps)
    for row in rows:
        useful_gaps_d = {}
        for gap in gaps:
            t = get_gap_finish_time(row, gap)
            if isinstance(t, float):
                if isinstance(row.time, str) or t < row.time:
                    if t not in useful_gaps_d:
                        useful_gaps_d[t] = []
                    useful_gaps_d[t].append(str(gap))

        useful_gaps = []
        times = []
        for (k_t, t_gaps) in useful_gaps_d.items():
            useful_gaps.append(min(t_gaps))
            times.append(str(k_t))

        if len(useful_gaps) > 1:
            str_useful_gaps = "/".join(useful_gaps)
            str_times = "/".join(times)
            df_row = {
                'benchmark': row.benchmark,
                'horizon': row.horizon,
                'total_time': row.time,
                'useful_gaps': str_useful_gaps,
                'times': str_times
            }
            df_rows.append(df_row)

    df = pd.DataFrame(df_rows)
    df.to_csv(os.path.join("results", f"{out_name}.csv"), index=False)



def tab_abhsvi_vs_ours(save_path: str, is_rs: bool) -> None:
    pomdps = load_pomdps()
    if is_rs:
        abhsvi_rows = get_all_abhsvi_rs_lines()
        our_rows = get_all_our_rs_lines()
    else:
        abhsvi_rows = get_all_f1_lines(False)
        our_rows = get_all_f1_lines(True)


    assert(len(abhsvi_rows) == len(our_rows))

    columns = ["benchmark", "num_states" , "num_actions", "num_obs",  "num_initial_states", "horizon", "time_abhsvi", "time_ours"]

    rows_df = []
    for (row_a, row_o) in zip(abhsvi_rows, our_rows):
        assert(row_a.benchmark.split(".")[0] == row_o.benchmark.split(".")[0])
        assert(row_a.initial_states == row_o.initial_states)
        assert(row_a.horizon == row_o.horizon)
        pomdp = pomdps[row_a.benchmark]
        assert pomdp.initial_states == row_o.initial_states
        row_df = {
            "benchmark": row_a.benchmark,
            "num_states": pomdp.num_states,
            "num_actions": pomdp.num_actions,
            "num_obs": pomdp.num_obs,
            "num_initial_states": row_a.initial_states,
            "horizon": row_a.horizon,
            "time_abhsvi": row_a.time,
            "time_ours": row_o.time,
        }

        rows_df.append(row_df)

    df = pd.DataFrame(rows_df)
    df.to_csv(save_path, index=False)


def check_results(is_rs: bool) -> None:
    if is_rs:
        abhsvi_rows = get_all_abhsvi_rs_lines()
        our_rows = get_all_our_rs_lines()
    else:
        abhsvi_rows = get_all_f1_lines(False)
        our_rows = get_all_f1_lines(True)

    for (row_a, row_o) in zip(abhsvi_rows, our_rows):
        benchmark_name = row_a.benchmark.split(".")[0]
        assert (row_a.benchmark.split(".")[0] == row_o.benchmark.split(".")[0])
        assert(row_a.horizon == row_o.horizon)

        # whenever there are no timeouts or errors on both sides, the values should match
        if row_a.time != "timeout" and row_o.time != "timeout" and row_a.time != "error" and row_o.time != "error":
            if not (math.isclose(row_a.value, row_o.value, rel_tol=1e-6, abs_tol=1e-6)):
                print("result mismatch: ", benchmark_name, row_a.horizon, row_a.value, row_o.value)

if __name__ == "__main__":
    tab_abhsvi_vs_ours(os.path.join("results", "vs_rock_sampling.csv"), is_rs=True)
    tab_abhsvi_vs_ours(os.path.join("results", "vs_cassandra.csv"), is_rs=False)

    # computing gap finish times
    rocks_rows__ = get_all_abhsvi_rs_lines()
    f1_rows__ = get_all_f1_lines(False)

    dump_gap_times(rocks_rows__, 7, "rocks_gaps")
    dump_gap_times(f1_rows__, 6, "cassandra_gaps")

    dump_useful_gaps(rocks_rows__, 7, "rocks_useful_gaps")
    dump_useful_gaps(f1_rows__, 6, "cassandra_useful_gaps")

    check_results(is_rs = True)
    check_results(is_rs = False)





