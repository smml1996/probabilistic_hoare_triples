from typing import List, Set, Dict, Optional, Tuple
from enum import Enum
import os
import pandas as pd
import math
import seaborn as sns
import matplotlib.pyplot as plt

parsed_results_path: str = ""

class Experiment(Enum):
    basic_zero_plus_discr =  "basic_zero_plus_discr"
    # basic_zero_plus_discr_therm =  "basic_zero_plus_discr_therm"
    bell_state_reach = "bell_state_reach"
    bitflip_cxh = "bitflip_cxh"
    bitflip_ipma = "bitflip_ipma"
    bitflip_ipma2 = "bitflip_ipma2"
    ghz3 = "ghz3"
    reset = "reset"
    # reset_therm = "reset_therm"
    
def get_nice_name (experiment: Experiment) -> str:
    if experiment in [Experiment.basic_zero_plus_discr]:
        return "state discr."
    
    if experiment == Experiment.bell_state_reach:
        return "Bell state reach"
    
    if experiment == Experiment.bitflip_cxh:
        return "Bitflip [CXH]"
    
    if experiment == Experiment.bitflip_ipma:
        return "Bitflip [IPMA]"
    
    if experiment == Experiment.bitflip_ipma2:
        return "Bitflip [IPMA2]"
    
    if experiment == Experiment.ghz3:
        return "GHZ"
    
    if experiment in [Experiment.reset]:
        return "Reset"
    
    raise Exception(f"Nice name not implemented for {experiment}")

def get_thermalization_str(experiment: Experiment) -> str:
    # if experiment in [Experiment.reset_therm, Experiment.basic_zero_plus_discr_therm]:
    #     return "yes"
    return "no"

class FileType(Enum):
    stats = 0
    verification = 1
    improvements = 2
class StatsFileLine:
    def __init__(self, line: str, file_type: FileType = FileType.stats):
        self.file_type = file_type
        assert isinstance(file_type, FileType)
        line = line.strip()
        elements = line.split(",")
        self.hardware = elements[0]
        self.embedding_index = int(elements[1])
        if file_type == FileType.verification:
            self.horizon = None
            self.pomdp_build_time = None
            self.probability = float(elements[3])
            self.method = elements[4]
            self.method_time = None
            self.algorithm_index = int(elements[2])
        elif file_type == FileType.stats:
            self.horizon = int(elements[2])
            self.pomdp_build_time = float(elements[3])
            self.probability = float(elements[4])
            self.method = elements[5]
            self.method_time = float(elements[6])
            self.algorithm_index = int(elements[7])
        else:
            assert file_type == FileType.improvements
            self.method = elements[2]
            self.horizon = int(elements[3])
            self.algorithm_index = int(elements[4])
            self.baseline_index = int(elements[5])
            self.algorithm_prob = float(elements[6])
            self.baseline_prob = float(elements[7])
            self.diff = float(elements[8])

    def __str__(self):
        assert self.file_type == FileType.improvements
        return f"hardware={self.hardware}-{self.embedding_index}, diff={self.diff}, prob={self.algorithm_prob}, baseline_prob={self.baseline_prob}, algorithm={self.algorithm_index}, method={self.method}, horizon={self.horizon},"

def get_experiment_path(experiment: Experiment) -> str:
    return os.path.join(parsed_results_path, experiment.value)

def get_stats_path(experiment: Experiment) -> str:
    return os.path.join(get_experiment_path(experiment), "stats.csv")

def get_verification_path(experiment: Experiment) -> str:
    # these verification results are just to debug the results obtained from synthesizing
    return os.path.join(get_experiment_path(experiment), "verification.csv")

def get_improvements_path(experiment: Experiment) -> str:
    return os.path.join(get_experiment_path(experiment), "improvements.csv")

def get_paper_verification_path(experiment: Experiment) -> str:
    # results of verification using the method described in the paper
    return os.path.join(get_experiment_path(experiment), "verify.csv")

def get_all_hardware(experiment: Experiment, method: str) -> Set[str]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    diff_hardware: Set[str] = set()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method:
            diff_hardware.add(stats_line.hardware)
    return diff_hardware

def get_embedding_count(experiment: Experiment, method: str) -> int:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    diff_embeddings: Set[Tuple[str, int]] = set()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method:
            diff_embeddings.add((stats_line.hardware, stats_line.embedding_index))
    return len(diff_embeddings)

def get_algorithms(experiment: Experiment, method: str) -> Set[int]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()
    all_algorithms: Set[int] = set()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method:
            all_algorithms.add(stats_line.algorithm_index)
    return all_algorithms

def get_algorithms_count(experiment: Experiment, method: str) -> int:
    return len(get_algorithms(experiment, method))

def get_perfect_hardware_algorithms(experiment, method) -> List[Tuple[int, int]]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()
    all_algorithms_sorted: List[Tuple[int, int]] = []
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method and stats_line.hardware == "perfect_hardware":
            all_algorithms_sorted.append((stats_line.horizon, stats_line.algorithm_index))
    return sorted(all_algorithms_sorted)


def get_algorithms_at_horizon(experiment: Experiment, method: str, horizon: int) -> Set[int]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    all_algorithms: Set[int] = set()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method and stats_line.horizon == horizon:
            all_algorithms.add(stats_line.algorithm_index)
    return all_algorithms


def get_methods_used(experiment: Experiment) -> Set[str]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    all_methods: Set[str] = set()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        all_methods.add(stats_line.method)
    return all_methods

def get_horizon_to_algorithms_count(experiment: Experiment, method:str) -> Dict[int, Set[int]]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    result: Dict[int, Set[int]] = dict()
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if method == stats_line.method:
            if stats_line.horizon not in result.keys():
                result[stats_line.horizon] = set()
            result[stats_line.horizon].add(stats_line.algorithm_index)
    return result


def get_hardware_count(experiment: Experiment, method: str) -> int:
    return len(get_all_hardware(experiment, method))

def get_perfect_hardware_accuracy(experiment: Experiment, method: str) -> str:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    result = ""
    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.hardware == "perfect_hardware" and stats_line.method == method:
            if len(result) > 0:
                result += ", "
            result += str(stats_line.probability)
    return result

class Stat(Enum):
    minimum = 0
    maximum = 1
    average = 2
    std = 3

class StatField(Enum):
    probability = 0
    pomdp_build_time = 1
    method_time = 2
    total_time = 3


def get_stat(experiment: Experiment, field: StatField,  stat: Stat, method: str) -> Optional[float]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()
    f.close()

    result: Optional[float] = None
    if stat == Stat.average:
        count = 0
    else:
        count = 1

    for line in lines[1:]:
        stats_line = StatsFileLine(line)
        if stats_line.method == method:
            if result is None:
                result = stats_line.probability
            else:
                if stat == Stat.minimum:
                    if field == StatField.probability:
                        result = min(result, stats_line.probability)
                    elif field == StatField.pomdp_build_time:
                        result = min(result, stats_line.pomdp_build_time)
                    elif field == StatField.method_time:
                        result = min(result, stats_line.method_time)
                    else:
                        assert(field == StatField.total_time)
                        result = min(result, stats_line.method_time + stats_line.pomdp_build_time)

                elif stat == Stat.maximum:
                    if field == StatField.probability:
                        result = max(result, stats_line.probability)
                    elif field == StatField.pomdp_build_time:
                        result = max(result, stats_line.pomdp_build_time)
                    elif field == StatField.method_time:
                        result = max(result, stats_line.method_time)
                    else:
                        assert(field == StatField.total_time)
                        result = max(result, stats_line.method_time + stats_line.pomdp_build_time)
                else:
                    count += 1
                    assert(stat == Stat.average)
                    if field == StatField.probability:
                        result += stats_line.probability
                    elif field == StatField.pomdp_build_time:
                        result += stats_line.pomdp_build_time
                    elif field == StatField.method_time:
                        result += stats_line.method_time
                    else:
                        assert(field == StatField.total_time)
                        result += stats_line.method_time + stats_line.pomdp_build_time
    if result is None:
        raise Exception("get stat result is None")
    return round(result/count, 5)

def get_max_improvement(experiment: Experiment, method: str) -> float:
    if experiment == Experiment.ghz3:
        return 0
    result = 0
    f = open(get_improvements_path(experiment))
    lines = f.readlines()[1:]
    f.close()

    for line in lines:
        stat_line = StatsFileLine(line, FileType.improvements)
        if stat_line.diff < 0:
            raise Exception("Optimal algorithm is not optimal", get_improvements_path(experiment), "in",  line)

        if stat_line.method == method:
            result = max(result, stat_line.diff)
    return result


def get_improvements(experiment: Experiment, method: str) -> Dict[int, List[StatsFileLine]]:
    if experiment == experiment.ghz3:
        return dict()
    f = open(get_improvements_path(experiment))
    lines = f.readlines()[1:]
    f.close()

    result: Dict[int, List[StatsFileLine]] = dict()
    for line in lines:
        stat_line = StatsFileLine(line, FileType.improvements)
        assert stat_line.diff >= 0
        if stat_line.method == method and stat_line.diff > 0:
            if stat_line.horizon not in result.keys():
                result[stat_line.horizon] = []
            result[stat_line.horizon].append(stat_line)

    for (h, l) in  result.items():
        l.sort(key=lambda x : x.diff, reverse=True)
    return result

def get_count_improvements(experiment: Experiment, method: str) -> int:
    result = 0
    if experiment == experiment.ghz3:
        return result
    f = open(get_improvements_path(experiment))
    lines = f.readlines()[1:]
    f.close()

    for line in lines:
        stat_line = StatsFileLine(line, FileType.improvements)
        if stat_line.method == method:
            if stat_line.diff > 0:
                result += 1
    return result

def dump_summary_table(method, save: bool=True):
    experiment_name: List[str] = []
    horizons: List[int] = []
    hardware_count: List[int] = []
    num_embeddings: List[int] = []
    num_algorithms: List[int] = []
    max_improvements: List[float] = []
    count_improvements: List[float] = []

    avg_build: List[Optional[float]] = []
    max_build: List[Optional[float]] = []

    avg_method: List[Optional[float]] = []
    max_method: List[Optional[float]] = []

    for experiment in Experiment:
        methods_used = get_methods_used(experiment)
        if method in methods_used:
            experiment_name.append(get_nice_name(experiment))
            horizons.append(max(get_horizon_to_algorithms_count(experiment, method).keys()))
            hardware_count.append(get_hardware_count(experiment, method=method))
            num_embeddings.append(get_embedding_count(experiment, method=method))
            num_algorithms.append(get_algorithms_count(experiment, method=method))
            max_improvements.append(get_max_improvement(experiment, method))
            count_improvements.append(get_count_improvements(experiment, method))

            avg_build.append(get_stat(experiment, StatField.pomdp_build_time, Stat.average, method=method))
            max_build.append(get_stat(experiment, StatField.pomdp_build_time, Stat.maximum, method=method))

            avg_method.append(get_stat(experiment, StatField.method_time, Stat.average, method=method))
            max_method.append(get_stat(experiment, StatField.method_time, Stat.maximum, method=method))
            
    df = pd.DataFrame({
        "experiment": experiment_name,
        "horizon": horizons,
        "#hard.": hardware_count,
        "#emb.": num_embeddings,
        "#diff. algo.": num_algorithms,
        "#improv.": count_improvements,
        "max. improv.": max_improvements,
        "avg. build time": avg_build,
        "max. build time": max_build,
        "avg. method time": avg_method,
        "max. method time": max_method,
    })
    
    if save:
        df.to_csv(f"summary_{method}.csv")
    return df

def count_improvements_at_horizon(experiment: Experiment, method: str, horizon: int) -> int:
    if experiment == Experiment.ghz3:
        return 0
    f = open(get_improvements_path(experiment))
    lines = f.readlines()[1:]
    f.close()

    result = 0
    for line in lines:
        stat_line = StatsFileLine(line, FileType.improvements)
        if stat_line.horizon == horizon and stat_line.method == method:
            if stat_line.diff > 0:
                result += 1

    return result
            

def dump_stats_summary(experiment: Experiment):
    methods = get_methods_used(experiment)
    for method in methods:
        with open(f"summary_{experiment.value}_{method}.txt", "w") as f:
            print("------", experiment, "------", file=f)
            print(f"%%%%%%%%%%%% {method} %%%%%%%%%%%%", file=f)
            print(f"count hardware ({method}): ", get_hardware_count(experiment, method=method), file=f)

            print(f"count embeddings ({method}): ", get_embedding_count(experiment, method=method), file=f)
            print(f"total diff. algorithms ({method}): ", get_algorithms_count(experiment, method=method), file=f)
            print(f"accuracy perfect hardware ({method}):", get_perfect_hardware_accuracy(experiment, method=method), file=f)
            print(file=f)

            horizon_to_algorithms_count = get_horizon_to_algorithms_count(experiment, method)
            if len(horizon_to_algorithms_count) > 0:
                print(f"num. algorithms per horizon (method={method})", file=f)
            for (horizon, algos) in horizon_to_algorithms_count.items():
                print(horizon, ": ", len(algos), file=f)
            print("--", file=f)
            print(file=f)
            print(file=f)

            print("number of improvements per horizon:", file=f)
            for horizon in horizon_to_algorithms_count.keys():
                print(f"--> horizon {horizon}: {count_improvements_at_horizon(experiment, method, horizon)} ", file=f)

            print(file=f)
            print(file=f)
            print("Improvements with respect to perfect algorithms per horizon", file=f)
            stats = get_improvements(experiment, method)
            horizons = list(stats.keys())
            sorted(horizons, reverse=False)

            for horizon in horizons:
                print(f"Horizon = {horizon}", file=f)
                for stat_line in stats[horizon]:
                    print(stat_line, file=f)
                print(file=f)

            print(file=f)
            print(file=f)

class UnitarySpec:
    is_multiqubit: bool
    succ_prob: float
    gate_name: str
    hardware: str
    embedding_index: int
    def __init__(self, line: str):
        elements = line.split(",")
        self.hardware = elements[0]
        self.embedding_index = int(elements[1])
        self.gate_name = elements[2]
        self.succ_prob = float(elements[3])
        if self.gate_name == "CNOT":
            self.is_multiqubit = True
        else:
            self.is_multiqubit = False
            if self.gate_name not in ["SX", "RZ", "X", "U2", "H", "U3", "RY", "RX", "U1"]:
                raise Exception("unknown gate:", self.gate_name)


class MeasSpec:
    hardware: str
    embedding_index: int
    success0: float
    success1: float
    meas_succ: float
    diff_meas: float
    abs_meas_diff: float

    def __init__(self, line):
        elements = line.split(",")
        self.hardware = elements[0]
        self.embedding_index = int(elements[1])
        self.success0 = float(elements[2])
        self.success1 = float(elements[3])
        self.meas_succ = (self.success1 + self.success0)/2.0
        self.diff_meas = self.success0 - self.success1
        self.abs_meas_diff = abs(self.success0 - self.success1)

def get_algorithms_to_specs(experiment: Experiment, method: str) -> Dict[int, Set[Tuple[str, int]]]:
    f = open(get_stats_path(experiment))
    lines = f.readlines()[1:]
    f.close()

    result = dict()
    for line in lines:
        stat_line = StatsFileLine(line)
        if stat_line.method == method:
            if stat_line.algorithm_index not in result:
                result[stat_line.algorithm_index] = set()
            result[stat_line.algorithm_index].add((stat_line.hardware, stat_line.embedding_index))

    return result



def dump_summary_specs_per_algo(experiment: Experiment, method: str) -> None:
    experiment_path = get_experiment_path(experiment)

    # load specifications
    meas_specs_path = os.path.join(experiment_path, "measurement_specs.csv")
    meas_specs_file = open(meas_specs_path)
    meas_specs_ = meas_specs_file.readlines()[1:]
    meas_specs = []
    for line in meas_specs_:
        meas_specs.append(MeasSpec(line))
    meas_specs_file.close()

    unitary_specs_path = os.path.join(experiment_path, "unitary_specs.csv")
    unitary_specs_file = open(unitary_specs_path)
    unitary_specs_ = unitary_specs_file.readlines()[1:]
    unitary_specs = []
    for line in unitary_specs_:
        unitary_specs.append(UnitarySpec(line))
    unitary_specs_file.close()

    algorithms_to_specs = get_algorithms_to_specs(experiment, method)
    summary_specs_path = os.path.join(experiment_path, f"specs_summary_per_alg_{method}.csv")
    summary_file = open(summary_specs_path, "w")
    summary_file.write(f"algorithm_index,min_success0,avg_success0,max_success0,min_success1,avg_success1,max_success1,min_meas_succ,avg_meas_succ,max_meas_succ,min_diff_meas,avg_diff_meas,max_diff_meas,min_abs_diff_meas,avg_abs_diff_meas,max_abs_diff_meas,min_succ_1Q,avg_succ_1Q,max_succ_1Q,min_succ_2Q,avg_succ_2Q,max_succ_2Q,num_hardwares\n")
    for (algorithm_index, specs) in algorithms_to_specs.items():
        min_success0 = 100
        avg_success0 = 0.0
        max_success0 = 0.0
        min_success1 = 100
        avg_success1 = 0.0
        max_success1 = 0.0
        min_meas_succ = 100
        avg_meas_succ = 0.0
        max_meas_succ = 0.0
        min_diff_meas = 100
        avg_diff_meas = 0.0
        max_diff_meas = 0.0
        min_abs_diff_meas = 100
        avg_abs_diff_meas = 0.0
        max_abs_diff_meas = 0.0
        min_succ_1Q = 100
        avg_succ_1Q = 0.0
        max_succ_1Q = 0.0
        min_succ_2Q = 100
        avg_succ_2Q = 0.0
        max_succ_2Q = 0.0
        num_hardware_specs = 0
        for spec_ in meas_specs:
            if (spec_.hardware, spec_.embedding_index) in specs:
                min_success0 = min(min_success0, spec_.success0)
                avg_success0 = avg_success0 + spec_.success0
                max_success0 = max(max_success0, spec_.success0)
                min_success1 = min(min_success1, spec_.success1)
                avg_success1 = avg_success1 + spec_.success1
                max_success1 = max(max_success1, spec_.success1)
                min_meas_succ = min(min_meas_succ, spec_.meas_succ)
                avg_meas_succ += spec_.meas_succ
                max_meas_succ = max(max_meas_succ, spec_.meas_succ)
                min_diff_meas = min(min_diff_meas, spec_.diff_meas)
                avg_diff_meas += spec_.diff_meas
                max_diff_meas = max(max_diff_meas, spec_.diff_meas)
                min_abs_diff_meas = min(min_abs_diff_meas, spec_.abs_meas_diff)
                avg_abs_diff_meas += spec_.abs_meas_diff
                max_abs_diff_meas = max(max_abs_diff_meas, spec_.abs_meas_diff)

        count_1Q = 0
        count_2Q = 0
        for spec_ in unitary_specs:
            if (spec_.hardware, spec_.embedding_index) in specs:
                if spec_.is_multiqubit:
                    count_2Q += 1
                    min_succ_2Q = min(min_succ_2Q, spec_.succ_prob)
                    avg_succ_2Q += spec_.succ_prob
                    max_succ_2Q = max(max_succ_2Q, spec_.succ_prob)
                else:
                    count_1Q += 1
                    min_succ_1Q = min(min_succ_1Q, spec_.succ_prob)
                    avg_succ_1Q += spec_.succ_prob
                    max_succ_1Q = max(max_succ_1Q, spec_.succ_prob)
        avg_success0 /= len(specs)
        avg_success1 /= len(specs)
        avg_meas_succ /= len(specs)
        avg_diff_meas /= len(specs)
        avg_abs_diff_meas /= len(specs)
        avg_abs_diff_meas /= len(specs)
        num_hardware_specs = len(specs)
        avg_succ_1Q /= count_1Q
        if count_2Q != 0:
            avg_succ_2Q /= count_2Q
        summary_file.write(f"{algorithm_index},{min_success0},{avg_success0},{max_success0},{min_success1},{avg_success1},{max_success1},{min_meas_succ},{avg_meas_succ},{max_meas_succ},{min_diff_meas},{avg_diff_meas},{max_diff_meas},{min_abs_diff_meas},{avg_abs_diff_meas},{max_abs_diff_meas},{min_succ_1Q},{avg_succ_1Q},{max_succ_1Q},{min_succ_2Q},{avg_succ_2Q},{max_succ_2Q},{num_hardware_specs}\n")
    summary_file.close()

def dump_all_summary_specs():
    for experiment in Experiment:
        methods = get_methods_used(experiment)
        for method in methods:
            dump_summary_specs_per_algo(experiment, method)


def get_class(algorithm: int, d: Dict[int, Set[int]]):
    for (key, vals) in d.items():
        if key == algorithm:
            return key
        if algorithm in vals:
            return key

    return -1

def have_same_stats(stats: List[StatsFileLine], algorithm1: int, algorithm2: int) -> bool:
    stats1 = set()
    stats2 = set()
    for stat_line in stats:
        if stat_line.algorithm_index == algorithm1:
            stats1.add((stat_line.hardware, stat_line.embedding_index, stat_line.probability))
        elif stat_line.algorithm_index == algorithm2:
            stats2.add((stat_line.hardware, stat_line.embedding_index, stat_line.probability))
    assert len(stats1) == len(stats2)
    return stats1 == stats2

def find_witness(stats: List[StatsFileLine], algorithm1: int, algorithm2: int) -> Optional[Tuple[str, int, float]]:
    stats1 = set()
    for stat_line in stats:
        if stat_line.algorithm_index == algorithm1:
            stats1.add((stat_line.hardware, stat_line.embedding_index, stat_line.probability))

    for stat_line in stats:
        if stat_line.algorithm_index == algorithm2:
            if (stat_line.hardware, stat_line.embedding_index, stat_line.probability) not in stats1:
                return (stat_line.hardware, stat_line.embedding_index, stat_line.probability)
    return None

def truncate(x, n):
    factor = 10 ** n
    return math.trunc(x * factor) / factor

def get_algorithms_classes(experiment: Experiment, method: str) -> Dict[int, Set[int]]:
    # load stats
    p = get_verification_path(experiment)
    f = open(p)
    lines = f.readlines()[1:]
    f.close()

    all_stats = []
    for line in lines:
        stat_line = StatsFileLine(line, FileType.verification)
        stat_line.probability = truncate(stat_line.probability, 6)
        all_stats.append(stat_line)

    all_algorithms = get_algorithms(experiment, method)

    result = dict()
    for algorithm1 in all_algorithms:
        found = False
        for algorithm2 in result.keys():
            if have_same_stats(all_stats, algorithm1, algorithm2):
                found = True
                result[algorithm2].add(algorithm1)
                break
            else:
                witness = find_witness(all_stats, algorithm1, algorithm2)
                assert witness is not None
                print(f"{algorithm1} {algorithm2} are not equal because: {witness}")

        if not found:
            result[algorithm1]  = set()

    return result


def get_scatterplot(experiment: Experiment, method: str):
    baseline_color = "black"
    name = f"scatter_{method}_{get_nice_name(experiment)}.pdf"
    
    df = pd.read_csv(get_improvements_path(experiment))
    df = df[df.method == method]
    
    df["group"] = (
    df["hardware"].astype(str)
    + "_E" + df["embedding_index"].astype(str)
    + "_H" + df["horizon"].astype(str)
    )
    df["y_pos"] = pd.factorize(df["group"])[0]
    palette = sns.color_palette("tab10", df["algorithm_index"].nunique())
    g = sns.FacetGrid(df, col="horizon", sharey=False, height=5, aspect=0.8, col_wrap=3)
    def dumbbell(data, **kwargs):
        
        for i, (alg_idx, subdf) in enumerate(data.groupby("algorithm_index")):
            
            alg_color = palette[i]
            
            for _, row in subdf.iterrows():
                
                y = row["y_pos"]
                base = row["baseline_prob"]
                alg = row["algorithm_prob"]
                diff = row["diff"]

                # Baseline (always same color)
                plt.scatter(base, y, color=baseline_color, s=35, zorder=3, alpha=0.3)

                # Algorithm (colored)
                if diff > 0:
                    plt.scatter(alg, y, color=alg_color, s=35, zorder=3, alpha=0.8)
                    plt.plot([base, alg], [y, y], color=alg_color, linewidth=1)
    g.map_dataframe(dumbbell)
    for ax in g.axes.flat:
        ax.set_yticks([])
        ax.tick_params(left=False)        # remove little tick lines
        ax.set_ylabel("hardware / embedding")

    plt.tight_layout()
    plt.savefig(f"figs/{name}")
    
def get_scatterplots():
    experiments = [Experiment.basic_zero_plus_discr,
                   Experiment.bell_state_reach,
                   Experiment.bitflip_cxh,
                   Experiment.bitflip_ipma,
                   Experiment.reset,
                   Experiment.bitflip_ipma2]
    
    for experiment in experiments:
        methods = get_methods_used(experiment)
        for method in methods:
            get_scatterplot(experiment, method)
        


