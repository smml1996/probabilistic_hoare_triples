import numpy as np
import scipy as sp
import re

def parse_basic(string):
    units = list(filter(None,re.split('\,|\[|\]|\s', string))) #Remove all syntax
    Num = int(units[0])
    return Num, units[1:Num+1]

def parse_parameters(strings,N):
    P, parameter_names = parse_basic(strings[0])
    parameter_dict = dict()
    parameter_matrix = np.zeros((P,N),float)
    for n in range(N):
        units = list(filter(None,re.split('\,|\[|\]|\s', strings[n+1]))) #Remove all syntax
        for p in range(P):
            parameter_matrix[p][n] = float(units[p])
    for p in range(P):
        parameter_dict.update({parameter_names[p]: parameter_matrix[p]})
    return P, parameter_dict

def parse_transitions(strings, parameter_dict,S,N,A):
    transitions = []
    for a in range(A):
        transitions_A = []
        for n in range(N):
            transitions_A.append(sp.sparse.dok_matrix((S,S),dtype=float))
        transitions.append(transitions_A)    
    for string in strings:
        s,a,ss,p = list(filter(None,re.split('\,|->|\s', string.rstrip()))) #Remove all syntax
        s,ss = int(s),int(ss)
        try:
            a = int(a)
            act = True
        except:
            a = 0
            act = False
        try:
            p_val = float(p)
            val = True
        except:
            p_formula = list(filter(None,re.split('\-', p.rstrip())))
            val = False
        
        if val:
            for n in range(N):
                transitions[a][n][s,ss] = p_val
        else:
            if len(p_formula) > 1:
                for n in range(N):
                    p_val = 1 - sum([parameter_dict[term][n] for term in p_formula[1:]])
                    transitions[a][n][s,ss] = p_val
            else:
                for n in range(N):
                    transitions[a][n][s,ss] = parameter_dict[p_formula[0]][n]
        if not(act):
            for aa in range(A):
                for n in range(N):
                    transitions[aa][n][s,ss] = transitions[a][n][s,ss]
    for a in range(A):
        for n in range(N):
            transitions[a][n] = transitions[a][n].tocsr()
            for i in range(S):
                if np.sum(transitions[a][n][i]) != 1 and transitions[a][n][i].getnnz() > 0:
                    print(f"Error: not a valid probability distribution from state {i} with action {a} in environment {n}: {transitions[a][n][i].A}")

    return transitions

def parse_observations(strings,parameter_dict,S,N,A,O):
    observations = []
    for a in range(A):
        observations_A = []
        for n in range(N):
            observations_A.append(sp.sparse.dok_matrix((S,O),dtype=float))
        observations.append(observations_A)    
    for string in strings:
        a,ss,o,p = list(filter(None,re.split('\,|->|\s', string.rstrip()))) #Remove all syntax
        try:
            a = int(a)
            act = True
        except:
            a = 0
            act = False
        ss,o = int(ss),int(o)
        try:
            p_val = float(p)
            val = True
        except:
            p_formula = list(filter(None,re.split('\-', p.rstrip())))
            val = False
        
        if val:
            for n in range(N):
                    observations[a][n][ss,o] = p_val
        else:
            if len(p_formula) > 1:
                for n in range(N):
                    p_val = 1 - sum([parameter_dict[term][n] for term in p_formula[1:]])
                    observations[a][n][ss,o] = p_val
            else:
                for n in range(N):
                    observations[a][n][ss,o] = parameter_dict[p_formula[0]][n]
        if not(act):
            for aa in range(A):
                for n in range(N):
                    observations[aa][n][ss,o] = observations[a][n][ss,o]

    for a in range(A):
        for n in range(N):
            observations[a][n] = observations[a][n].tocsr()
            for i in range(S):
                if np.sum(observations[a][n][i]) != 1:
                    print(f"Error: not a valid observation probability distribution in state {i} after action {a} in environment {n}: {observations[a][n][i].A}")

    return observations

def parse_rewards(strings,parameter_dict,S,N,A):
    rewards = []
    for n in range(N):
        rewards.append(sp.sparse.dok_matrix((S,A),dtype=float))
    for string in strings:
        s,a,r = list(filter(None,re.split('\,|->|\s', string.rstrip()))) #Remove all syntax
        s = int(s)
        try:
            a = int(a)
            act = True
        except:
            a = 0
            act = False
        try:
            r_val = float(r)
            val = True
        except:
            val = False
        
        if val:
            for n in range(N):
                rewards[n][s,a] = r_val
        else:
            for n in range(N):
                rewards[n][s,a] = parameter_dict[r][n]
        if not(act):
            for aa in range(A):
                for n in range(N):
                    rewards[n][s,aa] = rewards[n][s,a]
    for n in range(N):
        rewards[n] = rewards[n].tocsr()

    return rewards

def parse_beliefs(strings,parameter_dict,S,N):
    initial_beliefs = sp.sparse.dok_matrix((N,S),dtype=float)
    for string in strings:
        s,p = list(filter(None,re.split('\,|->|\s', string.rstrip()))) #Remove all syntax
        s = int(s)
        try:
            p_val = float(p)
            val = True
        except:
            p_formula = list(filter(None,re.split('\-', p.rstrip())))
            val = False
        
        if val:
            for n in range(N):
                initial_beliefs[n,s] = p_val
        else:
            if len(p_formula) > 1:
                for n in range(N):
                    p_val = 1 - sum([parameter_dict[term][n] for term in p_formula[1:]])
                    initial_beliefs[n,s] = p_val
            else:
                for n in range(N):
                    initial_beliefs[n,s] = parameter_dict[p_formula[0]][n]
    initial_beliefs = initial_beliefs.tocsr()
    for n in range(N):
        if np.sum(initial_beliefs[n]) != 1:
            print(f"Error: not a valid initial belief in environment {n}: {initial_beliefs[n].A}")

    return initial_beliefs

def parse_tuples(strings,S,N):
    initial_tuples = []
    for string in strings:
        n,s = list(filter(None,re.split('\,|\s', string.rstrip()))) #Remove all syntax
        initial_tuples.append((int(n),int(s)))

    return initial_tuples


def parse_model(file_name):
    file = open(file_name, "r")
    data = file.read().split("\n")
    file.close()
    lines = list(filter(None, (line.lstrip() for line in data)))
    S, state_names = parse_basic(lines[0])
    N, environment_names = parse_basic(lines[1])
    A, action_names = parse_basic(lines[2])
    O, observation_names = parse_basic(lines[3])
    _, parameter_dict = parse_parameters(lines[4:4+N+1],N)
    lines = lines[4+N+2:]
    trns_lines = []
    obs_lines = []
    rwrd_lines = []
    bel_lines = []
    beliefs_given = True

    i = 0
    while i < len(lines):
        if lines[i] == "# Observation function (a,s,o -> p)":
            trns_lines = lines[0:i]
            lines = lines[i+1:]
            i = 0
        elif lines[i] == "# Reward function (s,a -> r)":
            obs_lines = lines[0:i]
            lines = lines[i+1:]
            i = 0
        elif lines[i] == "# Initial beliefs (s -> p)":
            rwrd_lines = lines[0:i]
            bel_lines = lines[i+1:]
            break
        elif lines[i] == "# Initial tuples (n,s)":
            rwrd_lines = lines[0:i]
            bel_lines = lines[i+1:]
            beliefs_given = False
            break
        else:
            i += 1
    transitions = parse_transitions(trns_lines,parameter_dict,S,N,A)
    observations = parse_observations(obs_lines,parameter_dict,S,N,A,O)
    rewards = parse_rewards(rwrd_lines,parameter_dict,S,N,A)
    if beliefs_given:
        initial_beliefs = parse_beliefs(bel_lines,parameter_dict,S,N)
        initial_tuples = []
    else:
        initial_beliefs = []
        initial_tuples = parse_tuples(bel_lines,S,N)


    return (S,state_names,N,environment_names,A,action_names,O,observation_names,transitions,observations,rewards,beliefs_given,initial_beliefs,initial_tuples)

# Parse the ME-POMDP, PO-MEMDP, MO-POMDP, and POMDP files into the data structures used by the AB-HSVI implementation.
if __name__ == "__main__":
    parse_model("Models/RockSample_MEPOMDP_N3_G3_K4_R29.txt")