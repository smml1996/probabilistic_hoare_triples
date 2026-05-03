from enum import Enum
from typing import Optional, List, Dict, Union
import os
import pandas as pd
import math

pomdps_path = os.path.join("results", "pomdps.csv")



class FileType(Enum):
    abhsvi = 0
    ours = 1
    ours2 = 2


class POMDP:
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
        if tokens[4] == "-":
            self.initial_states = -1
        else:
            self.initial_states = int(tokens[4])

    def get_initial_states(self):
        assert(self.initial_states > 0)
        return self.initial_states

def load_pomdps() -> Dict[str, POMDP]:
    f = open(pomdps_path, "r")
    pomdps = {}
    for line in f.readlines()[1:]:
        pomdp = POMDP(line[:-1])
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
        self.initial_states = self.get_initial_states(tokens)
        self.size_to_convexify = self.get_size_to_convexify(tokens)

    def get_initial_states(self, tokens: list[str]) -> int:
        if self.file_type in [FileType.abhsvi, FileType.ours]:
            pomdps = load_pomdps()
            if self.benchmark in pomdps.keys():
                return pomdps[self.benchmark].get_initial_states()
            return pomdps[self.benchmark + ".txt"].get_initial_states()
        elif self.file_type == FileType.ours2:
            return 2
        else:
            raise Exception("file type not supported", self.file_type)

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
    ABHSVI_RESULTS_PATH = os.path.join("AB-HSVI_NeurIPS_2025", "my_results", "results_abhsvi.csv")
    MORE_ROCKS_RESULTS_PATH = os.path.join("AB-HSVI_NeurIPS_2025", "my_results", "more_rocks.csv")
    return get_rows(ABHSVI_RESULTS_PATH, FileType.abhsvi) + get_rows(MORE_ROCKS_RESULTS_PATH, FileType.abhsvi)

def get_all_our_rs_lines() -> List[Row]:
    ABHSVI_RESULTS_PATH = os.path.join("results", "abhsvi.csv")
    MORE_ROCKS_RESULTS_PATH = os.path.join("results", "more_rocks.csv")
    return get_rows(ABHSVI_RESULTS_PATH, FileType.ours) + get_rows(MORE_ROCKS_RESULTS_PATH, FileType.ours)

def format_benchmark_name(name: str) -> str:
    return name.replace("POMDP_", "").replace("RockSample", "RS")

def tab_rs_abhsvi_vs_ours(save_path=os.path.join("results", "vs_rock_sampling.csv")) -> None:
    pomdps = load_pomdps()
    abhsvi_rows = get_all_abhsvi_rs_lines()
    our_rows = get_all_our_rs_lines()


    assert(len(abhsvi_rows) == len(our_rows))

    columns = ["benchmark", "num_states" , "num_actions", "num_obs",  "num_initial_states", "horizon", "time_abhsvi", "time_ours"]

    rows_df = []
    for (row_a, row_o) in zip(abhsvi_rows, our_rows):
        benchmark_name = row_a.benchmark.split(".")[0]
        assert(row_a.benchmark.split(".")[0] == row_o.benchmark.split(".")[0])
        assert(row_a.initial_states == row_o.initial_states)
        assert(row_a.horizon == row_o.horizon)
        pomdp = pomdps[row_a.benchmark + ".txt"]
        assert pomdp.initial_states == row_o.initial_states
        row_df = {
            "benchmark": format_benchmark_name(benchmark_name),
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

## IFF utils

iff_grid = dict()
for i in range(3):
    iff_grid[i+1] = dict()

iff_grid[1][1] = 59
iff_grid[1][2] = 61
iff_grid[1][3] = 63


iff_grid[2][1] = 64
iff_grid[2][2] = 66
iff_grid[2][3] = 68

iff_grid[3][1] = 69
iff_grid[3][2] = 71
iff_grid[3][3] = 73

def parse_iff_results():
    out_f = open(os.path.join("results", "iff.csv"), "w")
    out_f.write(
       ",".join(["benchmark",
        "horizon",
        "time",
        "n_initial_states",
        "val"
    ]) + "\n")
    pomdp_name = "iff.POMDP"
    for d in range(1, 4):
        for v in range(1, 4):
            for d2 in range(d, 4):
                for v2 in range(v, 4):
                    if d != d2 or v != v2:
                        distance = (d-d2)*(d-d2) + (v-v2)*(v-v2)
                        result_f_name = f"f1_{pomdp_name}_{iff_grid[d][v]}_{iff_grid[d2][v2]}_{distance}.csv"
                        path = os.path.join("results", "unparsed", result_f_name)
                        rows = get_rows(path, FileType.ours2)
                        for row in rows:
                            out_f.write(",".join([
                                row.benchmark,
                                str(row.horizon),
                                str(row.time),
                                str(row.initial_states),
                                str(row.value)
                            ]) + "\n")
    out_f.close()

def get_dict_convexify(filename: str, horizon: int) -> dict:
    f = open(os.path.join("results", "unparsed", filename), "r")
    lines = f.readlines()[1:]
    result = dict()
    for line in lines:
        tokens = line.split(",")
        assert(len(tokens) == 7)
        horizon_ = int(tokens[1])
        time_ = float(tokens[2])
        time_ = "timeout" if time_ >=3600 else time_
        size_to_convexify = int(tokens[4])
        if horizon_ == horizon:
            assert(size_to_convexify not in result.keys())
            result[size_to_convexify] = time_

    f.close()

    return result


def parse_convexify_results():
    out_path = open(os.path.join("results", "convexify_iff.csv"), "w")
    df_rows = []
    pomdp_name = "iff.POMDP"
    for d in range(1, 4):
        for v in range(1, 4):
            for d2 in range(d, 4):
                for v2 in range(v, 4):
                    if d != d2 or v != v2:
                        distance = (d-d2)*(d-d2) + (v-v2)*(v-v2)
                        result_f_name = f"f1_{pomdp_name}_{iff_grid[d][v]}_{iff_grid[d2][v2]}_{distance}.csv"
                        path = os.path.join("results", "unparsed", result_f_name)
                        rows = get_rows(path, FileType.ours2)
                        for row in rows:
                            df_row = {
                                "benchmark": row.benchmark,
                                "horizon": row.horizon,
                                "time": row.time,
                            }
                            convexify_d = get_dict_convexify(f"f1_convexify_{pomdp_name}_{iff_grid[d][v]}_{iff_grid[d2][v2]}_{distance}.csv", row.horizon)
                            for (key, value) in convexify_d.items():
                                df_row[key] = value

                            df_rows.append(df_row)

    df = pd.DataFrame(df_rows)
    df.to_csv(out_path, index=False)

if __name__ == "__main__":
    tab_rs_abhsvi_vs_ours()



