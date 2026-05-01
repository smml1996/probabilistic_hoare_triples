import random
from itertools import product
import itertools
import numpy as np
import math

def create_dict(N,K,goodRocks):
    state_name_dict = dict()

    for i in range(N):
        for j in range(N):
            for gr in range(len(goodRocks)):
                gr_list = goodRocks[gr]
                gr_bin = "".join(["1" if k in gr_list else "0" for k in range(K)])
                state_name_dict[len(state_name_dict)] = f"s{i}.{j}_{gr_bin}"
    state_name_dict[len(state_name_dict)] = "exit"
    
    name_state_dict = {v: k for k, v in state_name_dict.items()}
    return state_name_dict, name_state_dict

def state(si,sj,gr,N,GR):
    return si * N*GR + sj*GR + gr

def Generate(filename,N,G,K,RSeed):
    random.seed(RSeed)
    rockPositions = dict()
    while len(rockPositions) < K:
        rock = (random.randint(0,N-1),random.randint(0,N-1))
        if rock != (0,0) and rock not in rockPositions.values():
            rockPositions[len(rockPositions)] = rock

    goodRocks = []
    for g in range(G+1):
        for comb in itertools.combinations([i for i in range(K)], g):
            goodRocks.append(list(comb))
    len_gr = len(goodRocks)
    
    state_name_dict, name_state_dict = create_dict(N,K,goodRocks)

    states = ", ".join(state_name_dict.values())
    state_string = f"{len(state_name_dict)}, [{states}]\n"
    S = len(state_name_dict)
    
    env_string = "1, [e1]\n"
    A = K+5
    check_actions = ", ".join([f"Check{k}" for k in range(K)])
    action_string = f"{A}, [North, South, East, West, Sample, {check_actions}]\n"
    obs_string = "3, [Bad, Good, Nothing]\n\n"
    par_string = "0, []\n\n"

    transition_strings = ["# Transition function (s,a,s -> p)"]
    for si in range(N):
        for sj in range(N):
            for gr in range(len_gr):
                if si < N-1:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},0,{state(si+1,sj,gr,N,len_gr)} -> 1")
                else:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},0,{state(si,sj,gr,N,len_gr)} -> 1")
                if si > 0:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},1,{state(si-1,sj,gr,N,len_gr)} -> 1")
                else:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},1,{state(si,sj,gr,N,len_gr)} -> 1")
                if sj < N-1:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},2,{state(si,sj+1,gr,N,len_gr)} -> 1")
                else:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},2,{S-1} -> 1")
                if sj > 0:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},3,{state(si,sj-1,gr,N,len_gr)} -> 1")
                else:
                    transition_strings.append(f"{state(si,sj,gr,N,len_gr)},3,{state(si,sj,gr,N,len_gr)} -> 1")
            if (si,sj) in rockPositions.values():
                k = [key for key, value in rockPositions.items() if value == (si,sj)][0]
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    state_name = state_name_dict[state_num]
                    gr_bin = state_name[-K:]
                    if gr_bin[k] == '1':
                        gr_bin2 = gr_bin[0:k]+"0"+gr_bin[k+1:]
                        state2_name = state_name[:-K]+gr_bin2
                    else:
                        state2_name = state_name
                    transition_strings.append(f"{name_state_dict[state_name]},4,{name_state_dict[state2_name]} -> 1")
            else:
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    transition_strings.append(f"{state_num},4,{state_num} -> 1")
            for k in range(K):
                a = k+5
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    transition_strings.append(f"{state_num},{a},{state_num} -> 1")
    transition_strings.append(f"{S-1},_,{S-1} -> 1")

    half_dist = math.floor(N/2)
    observation_strings = ["# Observation function (a,s,o -> p)"]
    for a in range(5):
        for si in range(N):
            for sj in range(N):
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    observation_strings.append(f"{a},{state_num},2 -> 1")
    for k in range(K):
        a = k+5
        for si in range(N):
            for sj in range(N):
                (sii,sjj) = rockPositions[k]
                dist = np.linalg.norm([si-sii,sj-sjj])
                eff = pow(2,-dist/half_dist)
                p_correct = round(0.5 + 0.5*eff,6)
                p_incorrect = round(1-p_correct,6)
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    state_name = state_name_dict[state_num]
                    gr_bin = state_name[-K:]
                    rock_state = int(gr_bin[k])
                    observation_strings.append(f"{a},{state_num},{rock_state} -> {p_correct}")
                    observation_strings.append(f"{a},{state_num},{abs(rock_state-1)} -> {p_incorrect}")
    observation_strings.append(f"_,{S-1},2 -> 1")

    reward_strings = ["# Reward function (s,a -> r)"]
    for si in range(N):
        for sj in range(N):
            if (si,sj) in rockPositions.values():
                k = [key for key, value in rockPositions.items() if value == (si,sj)][0]
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    state_name = state_name_dict[state_num]
                    gr_bin = state_name[-K:]
                    if gr_bin[k] == '1':
                        reward_strings.append(f"{state_num},4 -> 10")
                    else:
                        reward_strings.append(f"{state_num},4 -> -10")
            if sj == N-1:
                for gr in range(len_gr):
                    state_num = state(si,sj,gr,N,len_gr)
                    reward_strings.append(f"{state_num},2 -> 10")
    
    G_goodRocks = []
    for comb in itertools.combinations([i for i in range(K)], G):
        G_goodRocks.append(list(comb))

    belief_strings = ["# Initial tuples (n,s)"]
    for gr in range(len(G_goodRocks)):
        gr_list = G_goodRocks[gr]
        gr_bin = "".join(["1" if k in gr_list else "0" for k in range(K)])
        state_num = name_state_dict[f"s{0}.{0}_{gr_bin}"]
        belief_strings.append(f"0,{state_num}")
    
    with open(filename, "w") as file:
        file.write(state_string)
        file.write(env_string)
        file.write(action_string)
        file.write(obs_string)
        file.write(par_string)
        file.write("\n".join(transition_strings)+"\n\n")
        file.write("\n".join(observation_strings)+"\n\n")
        file.write("\n".join(reward_strings)+"\n\n")
        file.write("\n".join(belief_strings))

# Generate a POMDP for the RockSample problem with an n x n grid, g good rocks, and k rocks in total, using random seed r to create the rock positions.
if __name__ == "__main__":
    n = 3
    density_total_rocks = [0.3, 0.5, 0.75]
    density_good_rocks = [0.25, 0.5, 0.75]
    for dt in density_total_rocks:
        k = math.ceil(n*n * dt)
        for dg in density_good_rocks:
            g = math.ceil(dg*k)
            r = k + g
            Generate(f"Models/RockSample_POMDP_N{n}_G{g}_K{k}_R{r}_.txt",n,g,k,r)