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

    def max_improvement_at_k(self, k: int) -> float:
        assert k >= self.min_horizon and k <= self.max_horizon
        temp = [r.diff  for r in self.rows if ( r.diff  > 0 and r.horizon == k)]
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

    def get_row_max_improv(self) -> Row:
        answer = None
        current_improv = 0

        for r in self.rows:
            if answer is None:
                answer = r
                current_improv = r.diff
            elif r.diff > current_improv:
                answer = r
                current_improv = r.diff

        return answer

    def get_row_robust_improv(self):
        hardware = None
        embedding_index = None
        horizon = None

        if self.method_type == MethodType.singular:
            if self.exp_type == ExpType.ipma2:
                hardware = "cambridge"
                embedding_index = 1
                horizon = 8
            if self.exp_type == ExpType.cxh:
                hardware = "kolkata"
                embedding_index = 0
                horizon = 7
            if self.exp_type == ExpType.lphase:
                hardware = "brisbane"
                embedding_index = 0
                horizon = 7
            if self.exp_type == ExpType.reset:
                hardware = "brisbane"
                embedding_index = 0
                horizon = 9
        else:
            assert self.method_type == MethodType.linear
            if self.exp_type == ExpType.ipma2:
                hardware = "geneva"
                embedding_index = 0
                horizon = 4
            if self.exp_type == ExpType.lphase:
                hardware = "almaden"
                embedding_index = 1
                horizon = 7
            if self.exp_type == ExpType.reset:
                hardware = "montreal"
                embedding_index = 1
                horizon = 3


        for row in self.rows:
            if row.hardware == hardware and row.embedding_index == embedding_index and row.horizon == horizon:
                return row

        assert False

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
               "\\#emb.",
               "\\makecell[c]{min. $\\horizon$}",
               "\\makecell[c]{max. $\\horizon$}",
               "\\#progs.",
               "\\#tot. improv.",
               "\\makecell[c]{avg. synth.\\\\ time}",
               "\\makecell[c]{avg. verif.\\\\ time}"]

    table = Texttable()
    table.set_cols_align(["c" for _ in range(len(columns))])
    table.add_row(columns)

    for exp in ALL_EXPERIMENTS:
        columns = [exp.tex_short_name,
                   exp.tex_instruction_set,
                   exp.method_type.name,
                   exp.tex_initial_states,
                   exp.tex_target_states,
                   exp.tot_embeddings,
                   exp.min_horizon,
                   exp.max_horizon,
                   exp.tot_progs,
                   exp.tot_improvement,
                   exp.avg_method_time,
                   exp.avg_verify_time]
        table.add_row(columns)

    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="Experiments summary.", label="tab:exp_summary",position="t"))
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
        "avg. synth. time": avgs_method,
        "avg. verif. time": avgs_verification
    })
    df.to_csv(tab_path, index=False)


def get_results_table_singular():
    tab_path = os.path.join(abs_path, "tables", "singular_summary.tex")
    min_horizon = 1000
    max_horizon = -1
    header = [""]
    second_column = ["horizon"]
    for experiment in ALL_EXPERIMENTS:
        if experiment.method_type == MethodType.singular and experiment.exp_type != ExpType.ghz:
            max_horizon = max(experiment.max_horizon, max_horizon)
            min_horizon = min(experiment.min_horizon, min_horizon)
            header.append(experiment.tex_instruction_set)
            header.append("")
            second_column.append("max. improv. $(\\%)$")
            second_column.append("\\#improv.")
    table = Texttable()
    table.set_cols_align(["c" for _ in range((len(header)))])
    table.add_rows([header, second_column], header=False)


    for horizon in range(min_horizon, max_horizon + 1):
        current_col = [horizon]
        for experiment in ALL_EXPERIMENTS:
            if experiment.method_type == MethodType.singular and experiment.exp_type != ExpType.ghz:
                if experiment.min_horizon <= horizon and experiment.max_horizon >= horizon:
                    current_col.append(experiment.max_improvement_at_k(horizon))
                    current_col.append(experiment.improvement_at_k(horizon))
                else:
                    current_col.append("-")
                    current_col.append("-")
        table.add_row(current_col)
    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="Singular summary.", label="tab:singular_summary", position="t"))
    f.close()

def get_results_table_singular2():
    tab_path = os.path.join(abs_path, "tables", "singular_summary2.tex")
    header = [""]
    second_column = ["max. improv. $(\\%)$"]
    col3 = ["\\#improv."]
    for experiment in ALL_EXPERIMENTS:
        if experiment.method_type == MethodType.singular and experiment.exp_type != ExpType.ghz:
            header.append(experiment.tex_instruction_set)
            second_column.append(str(experiment.max_improvement))
            col3.append(str(experiment.tot_improvement))
    table = Texttable()
    table.set_cols_align(["c" for _ in range(len(header))])
    table.add_rows([header, second_column, col3], header=False)

    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="Singular summary 2.", label="tab:singular_summary2", position="t"))
    f.close()

def get_all_improv_tab():
    tab_path = os.path.join(abs_path, "tables", "all_improv.tex")
    min_horizon = 1000
    max_horizon = -1
    for experiment in ALL_EXPERIMENTS:
        if experiment.exp_type != ExpType.ghz and experiment.exp_type != ExpType.cxh:
            max_horizon = max(experiment.max_horizon, max_horizon)
            min_horizon = min(experiment.min_horizon, min_horizon)
    table = Texttable()

    header = ["", "", ""]
    for horizon in range(min_horizon, max_horizon + 1):
        header.append(horizon)
    table.set_cols_align(["c" for _ in range((len(header)))])
    all_rows = [header]
    for experiment in ALL_EXPERIMENTS:
        if experiment.exp_type != ExpType.ghz and experiment.exp_type != ExpType.cxh:
            tot_improv_col = [experiment.tex_instruction_set, experiment.method_type.name, "\\# improv"]
            max_improv_col = ["", "", "max. improv. $(\\%)$"]
            for horizon in range(min_horizon, max_horizon + 1):
                if experiment.max_horizon >= horizon >= experiment.min_horizon:
                    tot_improv_col.append(str(experiment.improvement_at_k(horizon)))
                    max_improv_col.append(str(experiment.max_improvement_at_k(horizon)))
                else:
                    tot_improv_col.append("-")
                    max_improv_col.append("-")
            all_rows.append(tot_improv_col)
            all_rows.append(max_improv_col)



    table.add_rows(all_rows, header=True)
    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="All improvements.", label="tab:all_improv", position="t"))
    f.close()


def get_example_improv_tab():
    tab_path = os.path.join(abs_path, "tables", "ex_improv.tex")
    table = Texttable()

    header = ["","from", "to", "diff", "Fig.", "from", "to", "diff", "Fig."]
    table.set_cols_align(["c" for _ in range((len(header)))])
    all_rows = [header]
    for experiment in ALL_EXPERIMENTS:
        if experiment.exp_type != ExpType.ghz:
            max_improv_row = experiment.get_row_max_improv()
            robust_row = experiment.get_row_robust_improv()
            r = [f"{experiment.tex_instruction_set}~({experiment.method_type.name})",
                 f"{round(max_improv_row.baseline_prob*100,f_round_to)}\\%",
                 f"{round(max_improv_row.prob*100, f_round_to)}\\%",
                 f"{round(max_improv_row.diff * 100, f_round_to)}\\%",
                 "",
                 f"{round(robust_row.baseline_prob*100,f_round_to)}\\%",
                 f"{round(robust_row.prob*100, f_round_to)}\\%",
                 f"{round(robust_row.diff * 100, f_round_to)}\\%",
                 ""]
            all_rows.append(r)

    table.add_rows(all_rows, header=True)
    f = open(tab_path, "w")
    f.write(latextable.draw_latex(table, caption="Example improvements.", label="tab:ex_improv", position="t"))
    f.close()

if __name__ == "__main__":
    summary_table()
    get_other_stats_csv()
    get_results_table_singular()
    get_results_table_singular2()
    get_all_improv_tab()
    get_example_improv_tab()