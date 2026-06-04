experiments = [
    "bv",
    "bv0",
    "discr",
    "reset",
    "superdense_cod"
]

sp_message = ["00", "01", "02", "03"]

for (i0, m0) in enumerate(sp_message):
    for (i1, m1) in enumerate(sp_message):
        if i0 < i1:
            experiments.append(f"superdense_cod{m0}{m1}")

output_file = open("slurm_helper.sh", "w")
for experiment in experiments:
    output_file.write(f"sbatch slurm_script.sh {experiment}\n")

output_file.close()