f = open("helper_f1_slurm.sh", "w")
convexify_file = open("helper_f1_convexify.sh", "w")
bfs_distances = [i for i in range(4, 8)]
pomdp_names = ["cit.POMDP",
               "fourth.POMDP",
               "mit.POMDP",
               "pentagon.POMDP",
               "sunysb.POMDP"]
for n_states in range(1, 5):
    for bfs_distance in bfs_distances:
        for pomdp_name in pomdp_names:
            f.write(f"sbatch f1_slurm.sh {pomdp_name}_{n_states}_{bfs_distance}\n")
            convexify_file.write(f"sbatch convex_slurm.sh {pomdp_name}_{n_states}_{bfs_distance}\n")
f.close()

convexify_file.close()