import math

f = open("helper_f1_slurm.sh", "w")
convexify_file = open("helper_f1_convexify.sh", "w")
bfs_distances = [i for i in range(4, 8)]
pomdp_names = [
    "cit.POMDP_53_73_1",
    "mit.POMDP_174_190_1",
    "pentagon.POMDP_41_173_1",
    "sunysb.POMDP_183_227_1"] +[
    "iff.POMDP_54_56_1",
    "iff.POMDP_54_58_4",
    "iff.POMDP_54_59_1",
    "iff.POMDP_54_61_2",
    "iff.POMDP_54_63_5",
    "iff.POMDP_56_58_1",
    "iff.POMDP_56_61_1",
    "iff.POMDP_56_63_2",
    "iff.POMDP_58_63_1",
    "iff.POMDP_59_61_1",
    "iff.POMDP_59_63_4",
    "iff.POMDP_61_63_1"]
               # + [
               #  "RockSample_POMDP_N3_G1_K2_R18.txt",
               #   "RockSample_POMDP_N3_G1_K3_R13.txt",
               #   "RockSample_POMDP_N3_G1_K4_R83.txt",
               #   "RockSample_POMDP_N3_G2_K3_R4.txt",
               #   "RockSample_POMDP_N4_G1_K2_R33.txt",
               #   "RockSample_POMDP_N5_G1_K2_R96.txt",
               #   "RockSample_POMDP_N6_G1_K2_R77.txt"
               #  ]

# n = 3
# density_total_rocks = [0.3, 0.5, 0.75]
# density_good_rocks = [0.25, 0.5, 0.75]
# for dt in density_total_rocks:
#     k = math.ceil(n*n * dt)
#     for dg in density_good_rocks:
#         g = math.ceil(dg*k)
#         r = k + g
#         pomdp_names.append(f"RockSample_POMDP_N{n}_G{g}_K{k}_R{r}_.txt")

for pomdp_name in pomdp_names:
    f.write(f"sbatch run_test_slurm.sh {pomdp_name}\n")
    convexify_file.write(f"sbatch convex_slurm.sh {pomdp_name}\n")


f.close()

convexify_file.close()