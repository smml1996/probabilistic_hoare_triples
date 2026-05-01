from enum import Enum
from typing import Optional, List, Dict, Union
import os
import pandas as pd
import math

pomdps_path = os.path.join("results", "pomdps.csv")



class FileType(Enum):
    abhsvi = 0
    ours = 1


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
        else:
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
        else:
            raise Exception("file type not supported", self.file_type)

    def get_size_to_convexify(self, tokens: list[str]) -> int:
        return -1


    def check_num_columns(self, tokens):
        if self.file_type == FileType.ours:
            assert(len(tokens) == 5)
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



if __name__ == "__main__":
    tab_rs_abhsvi_vs_ours()



