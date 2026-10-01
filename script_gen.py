from paper_utils import *

all_tests = get_all_f1_tests() + [t + ".txt" for t in get_all_rs_tests()]

f = open("helper.sh", "w")

for t in all_tests:
    f.write(f"sbatch f1_slurm.sh {t}\n")
f.close()



