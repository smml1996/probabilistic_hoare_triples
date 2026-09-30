import os
folders = [
    "reset", "bell_state_reach", "basic_zero_plus_discr"
]

for folder in folders:
    p = os.path.join(folder, "stats.csv")
    f = open(p, "r")
    lines = f.readlines()[1:]
    print(lines)
    current_avg = 0.0
    count = 0.0
    for line in lines:
        elements = line.split(",")
        horizon = int(elements[2])
        method_name = elements[5]
        method_time = float(elements[6])
        if method_name == "convex":
            if horizon == 3:
                current_avg += method_time
                count += 1
        else:
            assert(method_name == "bellman")

    print(folder, current_avg / count)