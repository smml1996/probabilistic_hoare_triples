import latextable
from texttable import Texttable

from gen_tex_macros import _tex_name, _tex_name_abbr, ExpType, MethodType, _tex_instruction_set
from gen_tex_macros import _tex_initial_states, _tex_target_states
import os
from typing import List
import pandas as pd

abs_path = os.path.dirname(os.path.abspath(__file__))

f_round_to = 3


class Row:
    hardware: str
    embedding_index: int
    horizon: int
    prob: float
    baseline_prob: float
    diff: float
    method: MethodType
    method_time = float
    algorithm_index: int
    baseline_index: int
    num_strats: int
    tot_ins: int
    cstates: int
    verify_time: float

    def __init__(self, line):
        elements = line.split(",")
        self.hardware = elements[0]
        self.embedding_index = int(elements[1])
        self.horizon = int(elements[2])
        self.prob = float(elements[3])
        self.baseline_prob = float(elements[4])
        self.diff = float(elements[5])
        if elements[6] == "bellman":
            self.method = MethodType.singular
        else:
            assert elements[6] == "convex"
            self.method = MethodType.linear
        self.method_time = float(elements[7])
        self.algorithm_index = int(elements[8])
        self.baseline_index = int(elements[9])
        self.num_strats = int(elements[10])
        self.tot_ins = int(elements[11])
        self.cstates = int(elements[12])
        self.verify_time = float(elements[13])


class Experiment:
    exp_type: ExpType
    rows: List[Row]

    def __init__(self, exp_type: ExpType, method_type: MethodType):
        self.exp_type = exp_type
        self.method_type = method_type
        self.rows = []

        file_path = os.path.join(abs_path, exp_type.name, "stats.csv")
        f = open(file_path, "r")
        lines = f.readlines()[1:]

        for line in lines:
            row = Row(line)
            if row.method == self.method_type:
                self.rows.append(row)
        f.close()

    @property
    def tex_short_name(self):
        return _tex_name_abbr(self.exp_type, self.method_type)

    @property
    def tex_name(self):
        return _tex_name(self.exp_type, self.method_type)

    @property
    def tex_instruction_set(self):
        return _tex_instruction_set(self.exp_type)

    @property
    def tex_initial_states(self):
        return _tex_initial_states(self.exp_type)

    @property
    def tex_target_states(self):
        return _tex_target_states(self.exp_type)

    @property
    def tot_hardware(self):
        return len(set([r.hardware for r in self.rows]))

    @property
    def tot_embeddings(self):
        return len([r for r in self.rows if r.horizon == self.min_horizon])

    @property
    def max_horizon(self):
        return max([r.horizon for r in self.rows])

    @property
    def min_horizon(self):
        return min([r.horizon for r in self.rows])

    @property
    def avg_method_time(self):
        temp = [r.method_time for r in self.rows]
        temp = sum(temp) / len(temp)
        return round(temp, f_round_to)

    @property
    def max_method_time(self):
        return round(max([r.method_time for r in self.rows]), f_round_to)

    @property
    def avg_verify_time(self):
        temp = [r.verify_time for r in self.rows]
        temp = sum(temp) / len(temp)
        return round(temp, f_round_to)

    @property
    def max_verify_time(self):
        return round(max([r.verify_time for r in self.rows]), f_round_to)

    @property
    def tot_improvement(self):
        return len([r for r in self.rows if r.diff > 0])

    @property
    def max_improvement(self):
        return round(max([r.diff for r in self.rows]) * 100.0, f_round_to)

    @property
    def tot_progs(self):
        return len(set(r.algorithm_index for r in self.rows))

    @property
    def num_cvars(self):
        temp = set(r.cstates for r in self.rows)
        assert len(temp) == 1
        return list(temp)[0]

    def improvement_at_k(self, k: int) -> int:
        assert k >= self.min_horizon and k <= self.max_horizon
        return len([r for r in self.rows if( r.diff > 0 and r.horizon == k)])

    def max_improvement_at_k(self, k: int) -> int:
        assert k >= self.min_horizon and k <= self.max_horizon
        temp = [r.diff for r in self.rows if (r.diff > 0 and r.horizon == k)]
        if len(temp) == 0:
            return 0
        return round(max(temp)*100.0, f_round_to)

    def avg_method_at_k(self, k: int) -> float:
        temp = [r.method_time for r in self.rows if r.horizon == k]
        temp = sum(temp) / len(temp)
        return round(temp, f_round_to)

    def max_method_at_k(self, k: int) -> float:
        return round(max([r.method_time for r in self.rows if r.horizon == k]), f_round_to)

    def avg_verification_at_k(self, k: int) -> float:
        temp = [r.verify_time for r in self.rows if r.horizon == k]
        temp = sum(temp) / len(temp)
        return round(temp, f_round_to)

    def algos_at_k(self, k: int) -> int:
        return len(set([r.algorithm_index for r in self.rows if r.horizon == k]))

    def max_verification_at_k(self, k: int) -> float:
        return round(max([r.verify_time for r in self.rows if r.horizon == k]), f_round_to)

ALL_EXPERIMENTS = [
    Experiment(ExpType.ipma2, MethodType.singular),
    Experiment(ExpType.ipma2, MethodType.linear),
    Experiment(ExpType.cxh, MethodType.singular),
    Experiment(ExpType.ghz, MethodType.singular),
    Experiment(ExpType.reset, MethodType.singular),
    Experiment(ExpType.reset, MethodType.linear),
    Experiment(ExpType.lphase, MethodType.singular),
    Experiment(ExpType.lphase, MethodType.linear),
]

def summary_table():
    tab_path = os.path.join(abs_path, "tables", "summary.tex")
    columns = ["problem",
               "ins. set",
               "method",
               "\\makecell[c]{init.\\\\ states}",
               "\\makecell[c]{target.\\\\ states}",
               "$|\\cstates|$",
               "\\#embeddings",
               "\\makecell[c]{min.\\\\ horizon}",
               "\\makecell[c]{max.\\\\ horizon}",
               "\\#progs."]

    table = Texttable()
    table.set_cols_align(["c" for _ in range(len(columns))])
    table.add_row(columns)

    for exp in ALL_EXPERIMENTS:
        columns = [exp.tex_short_name,
                   exp.tex_instruction_set,
                   exp.method_type.name,
                   exp.tex_initial_states,
                   exp.tex_target_states,
                   exp.num_cvars,
                   exp.tot_embeddings,
                   exp.min_horizon,
                   exp.max_horizon,
                   exp.tot_progs]
        table.add_row(columns)

    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="Experiments summary.", label="tab:exp_summary"))
    f.close()

def get_other_stats_csv():
    tab_path = os.path.join(abs_path, "tables", "other_stats.csv")

    experiments = []
    methods = []
    horizons = []
    num_algorithms = []
    count_improvements = []
    max_improvements = []
    avgs_method = []
    avgs_verification = []

    for exp in ALL_EXPERIMENTS:
        for horizon in range(exp.min_horizon, exp.max_horizon + 1):
            experiments.append(exp.exp_type.name)
            methods.append(exp.method_type.name)
            horizons.append(horizon)
            num_algorithms.append(exp.algos_at_k(horizon))
            count_improvements.append(exp.improvement_at_k(horizon))
            max_improvements.append(exp.max_improvement_at_k(horizon))
            avgs_method.append(exp.avg_method_at_k(horizon))
            avgs_verification.append(exp.avg_verification_at_k(horizon))


    df = pd.DataFrame({
        "experiment": experiments,
        "method": methods,
        "horizon": horizons,
        "algos.": num_algorithms,
        "num. improv.": count_improvements,
        "max. improv.": max_improvements,
        "avg. method time": avgs_method,
        "avg. verif. time": avgs_verification
    })
    df.to_csv(tab_path, index=False)

if __name__ == "__main__":
    summary_table()
    get_other_stats_csv()