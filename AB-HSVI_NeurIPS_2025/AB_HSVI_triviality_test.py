import numpy as np
import scipy as sp
from dataclasses import dataclass
from typing import List
import gurobipy as gp
import Parser
from gurobipy import GRB
import time

@dataclass
class AlphaVec:
    action: int = -1
    env_values: List[float] = sp.sparse.csr_matrix(0, dtype=float)
    identifier: str = "v0"
    id_num: int = 0

    def __str__(self):
        return f"Alpha-vector v{self.id_num} ({self.identifier}): rewards for action {action_names[self.action]}:\n{self.env_values}."

@dataclass
class Belief:
    values: List[float] = sp.sparse.csr_matrix(0, dtype=float)

    def __str__(self):
        return f"Belief:\n{self.values}."
    
    def __hash__(self):
        return hash(tuple([val for s_val in self.values for val in s_val]))
    
    def __eq__(self,other):
        return (self.values != other.values).nnz()==0

def mc_comp(a,R_low,disc):
    values = np.full((N,S),R_low, dtype=float)
    new_values = np.zeros((N,S), dtype=float)
    done = False
    while not done:
        for n in range(N):
            for s in range(S):
                new_values[n][s] = rewards[n][s,a] + disc*sum([transitions[a][n][s,ss]*values[n][ss] for ss in range(S)])
        if np.sum(np.absolute(values-new_values)) < 0.0001:
            done = True
        else:
            values = new_values
            new_values = np.zeros((N,S), dtype=float)
    return new_values

def initialize_gamma(disc):
    global a_i
    gamma = []
    R_low = max([min([rewards[n][s,a] for n in range(N) for s in range(S)]) for a in range(A)])/(1-disc)
    for a in range(A):
        a_values = sp.sparse.csr_matrix(mc_comp(a,R_low,disc))
        gamma.append(AlphaVec(a,a_values,f"v{a_i}_a{a}",a_i))
        a_i += 1
    return gamma

def mdp_comp(disc):
    values = np.zeros((N,S), dtype=float)
    new_values = np.zeros((N,S), dtype=float)
    done = False
    while not done:
        for n in range(N):
            for s in range(S):
                new_values[n][s] = max([rewards[n][s,a] + disc*sum([transitions[a][n][s,ss]*values[n][ss] for ss in range(S)]) for a in range(A)])
        if np.sum(np.absolute(values-new_values)) < 0.0001:
            done = True
        else:
            values = new_values
            new_values = np.zeros((N,S), dtype=float)
    return new_values

def FIB_comp(values,disc):
    a_values = [values]*A
    new_a_values = []
    done = False
    while not done:
        done = True
        for a in range(A):
            new_values = np.zeros((N,S), dtype=float)
            for n in range(N):
                for s in range(S):
                    new_values[n][s] = rewards[n][s,a] + disc*sum([max([sum([transitions[a][n][s,ss]*observations[a][n][ss,o]*values[n][ss] for ss in range(S)]) for values in a_values]) for o in range(O)])
            if np.sum(np.absolute(a_values[a]-new_values)) > 0.0001:
                new_a_values.append(new_values)
                new_values = np.zeros((N,S), dtype=float)
                done = False
            else:
                new_a_values.append(a_values[a])
        a_values = new_a_values
        new_a_values = []
    return [sp.sparse.csr_matrix(values) for values in a_values]

def initialize_upsilon(disc):
    global a_i
    mdp_values = mdp_comp(disc)
    a_values = FIB_comp(mdp_values,disc)
    upsilon = []
    
    for n in range(N):
        for s in range(S):
            val = max([values[n,s] for values in a_values])
            upsilon.append((Belief(sp.sparse.csr_matrix(([1.0],([n],[s])),shape=(N,S))), val))
    return(upsilon)

def sawtooth(upsilon_det,upsilon_nondet, bel):
    val_zero = 0
    for (b,v) in upsilon_det:
        val_zero += np.sum(b.values.multiply(bel.values)*v)
    
    vals = [0]
    for(bb,vv) in upsilon_nondet:
        bb_val_zero = 0
        min_bb = 1
        for(b,v) in upsilon_det:
            bb_val_zero += np.sum(b.values.multiply(bb.values))*v
        for n in range(N):
            for s in range(S):
                if bb.values[n,s] > 0:
                    min_bel = bel.values[n,s]/bb.values[n,s]
                    if min_bel <= min_bb:
                        min_bb = min_bel
        vals.append((vv-bb_val_zero)*min_bb)
    val = min(vals)
    return val+val_zero

def comp_V_lb(bel,gamma):
    vals = []
    for alpha in gamma:
        vals.append(np.sum(alpha.env_values.multiply(bel.values)))
    return np.max([vals])

def comp_Q_vals(bel,upsilon_det,upsilon_nondet,disc):
    bel_vals = bel.values
    q_vals = []
    for a in range(A):
        rew = 0
        for n in range(N):
            for s in range(S):
                rew += bel_vals[n,s]*rewards[n][s,a]
        fut_rew = 0
        for o in range(O):
            p_o = 0
            for n in range(N):
                trns = transitions[a][n]
                obs = observations[a][n]
                for s in range(S):
                    bel_ns = bel_vals[n,s]
                    for ss in range(S):
                        p_o += bel_ns*trns[s,ss]*obs[ss,o]
            if p_o > 0:
                new_bel, _ = belief_update(bel,a,o)
                fut_rew += p_o*sawtooth(upsilon_det,upsilon_nondet,new_bel)
        q_val = rew + disc*fut_rew
        q_vals.append(q_val)
    return q_vals

def update_gamma(bel, gamma, disc):
    global a_i
    alphas = []

    for a in range(A):
        a_alphas = []
        for o in range(O):
            new_bel, valid_belief = belief_update(bel,a,o)
            if valid_belief:
                vals = [np.sum(alpha.env_values.multiply(new_bel.values)) for alpha in gamma]
                a_alphas.append(gamma[np.argmax(vals)].env_values)
            else:
                a_alphas.append(sp.sparse.csr_matrix((N, S), dtype=float))
        alphas.append(a_alphas)
    
    row,col,data = [],[],[]
    a_vecs = []
    for a in range(A):
        for n in range(N):
            trns = transitions[a][n]
            obs = observations[a][n]
            for s in range(S):
                val = rewards[n][s,a] + disc*sum([trns[s,ss]*obs[ss,o]*alphas[a][o][n,ss] for ss in range(S) for o in range(O)])
                if val != 0:
                    row.append(n)
                    col.append(s)
                    data.append(val)
        a_vecs.append(sp.sparse.csr_matrix((data,(row,col)),shape=(N,S)))
        row,col,data = [],[],[]
    
    max_index = np.argmax([np.sum(vec.multiply(bel.values)) for vec in a_vecs])
    gamma.append(AlphaVec(max_index,a_vecs[max_index],f"v{a_i}_a{max_index}",a_i))
    a_i += 1

    return gamma

    
def comp_o_vals(bel, a_star, gamma, upsilon_det, upsilon_nondet, disc, epsilon, t):
    o_vals = []
    for o in range(O):
        new_bel, valid_belief = belief_update(bel,a_star,o)
        if valid_belief:
            V_ub = sawtooth(upsilon_det,upsilon_nondet,new_bel)
            V_lb = comp_V_lb(new_bel,gamma)
            excess = V_ub - V_lb - epsilon/pow(disc,t)
            p_o = 0
            for n in range(N):
                trns = transitions[a_star][n]
                obs = observations[a_star][n]
                for s in range(S):
                    bel_ns = bel.values[n,s]
                    for ss in range(S):
                        p_o += bel_ns*trns[s,ss]*obs[ss,o]
            o_vals.append(p_o*excess)
        else:
            o_vals.append(np.nan)
    return o_vals

def explore(bel, gamma, upsilon_det, upsilon_nondet, disc, epsilon, t):
    done = False
    while not done:
        q_vals = comp_Q_vals(bel,upsilon_det,upsilon_nondet,disc)
        a_star = np.nanargmax(q_vals)
        gamma = update_gamma(bel,gamma,disc)
        if (bel.values >= 1).getnnz() == 1:
            row, col, _ = sp.sparse.find(bel.values)
            index = row[0]*S+col[0]
            (_,val) = upsilon_det[index]
            if val > q_vals[a_star]:
                upsilon_det[index] = (bel,q_vals[a_star])
        else:
            upsilon_nondet.append((bel,q_vals[a_star]))
        
        o_star = np.nanargmax(comp_o_vals(bel, a_star, gamma, upsilon_det, upsilon_nondet, disc, epsilon, t))
        new_bel, _ = belief_update(bel,a_star,o_star)
        V_lb = comp_V_lb(new_bel,gamma)
        V_ub = sawtooth(upsilon_det,upsilon_nondet,new_bel)
        if (V_ub - V_lb) <= epsilon/pow(disc,t):
            done = True
        else:
            t += 1
            bel = new_bel
    
    return gamma, upsilon_det, upsilon_nondet

def belief_update(bel, a, o):
    new_values = sp.sparse.dok_matrix((N,S),dtype=float)
    new_bel = 0
    valid_bel = True

    denom = sum([bel.values[n,s]*transitions[a][n][s,ss]*observations[a][n][ss,o] for n in range(N) for s in range(S) for ss in range(S)])
    if denom > 0:
        for n in range(N):
            trns = transitions[a][n]
            obs = observations[a][n]
            for ss in range(S):
                new_values[n,ss] = sum([bel.values[n,s]*trns[s,ss]*obs[ss,o] for s in range(S)])/denom
        new_bel = Belief(new_values.tocsr())
    else:
        valid_bel = False
    
    return new_bel, valid_bel

def prune_gamma(gamma):
    i = 0
    while i < len(gamma):
        j = 0
        while j < i:
            if (gamma[i].env_values < gamma[j].env_values).getnnz() == 0:
                gamma.pop(j)
                i -= 1
                j = i
            elif (gamma[i].env_values > gamma[j].env_values).getnnz() == 0:
                gamma.pop(i)
                i -= 1
                j = i
            else:
                j += 1
        i += 1
    return(gamma)

def prune_upsilon(upsilon_det,upsilon_nondet):
    i = 0
    while i < len(upsilon_nondet):
        bel,val = upsilon_nondet[i]
        if val > sawtooth(upsilon_det,upsilon_nondet,bel):
            upsilon_nondet.pop(i)
        else:
            i += 1
    return upsilon_nondet


def nature_policy(gamma,beliefs_given,initial_beliefs,initial_tuples, precision):
    try:
        # Create a new model
        m = gp.Model("nature")
        m.setParam('OutputFlag', 0)

        # Create variables
        if beliefs_given:
            x = m.addVars(N, vtype=GRB.CONTINUOUS,lb = 0.0, ub=1.0, name='x')
        else:
            x = m.addVars(len(initial_tuples), vtype=GRB.CONTINUOUS,lb = 0.0, ub=1.0, name='x')
        t = m.addVar(vtype=GRB.CONTINUOUS, name="t")

        # Set objective
        m.setObjective(t, GRB.MINIMIZE)

        # Add constraints
        if beliefs_given:
            for g in range(len(gamma)):
                alpha = gamma[g].env_values
                expr = gp.LinExpr()
                for n in range(N):
                    expr.add(sum([alpha[n,s]*initial_beliefs[n,s] for s in range(S)])*x[n])
                m.addConstr(expr <= t, f"c{n}")
        else:
            for g in range(len(gamma)):
                alpha = gamma[g].env_values
                expr = gp.LinExpr()
                for j in range(len(initial_tuples)):
                    (n,s) = initial_tuples[j]
                    expr.add(alpha[n,s]*x[j])
                m.addConstr(expr <= t, f"c{n}")
        m.addConstr(x.sum() == 1)

        # Optimize model
        m.optimize()

        # Contruct policy
        nat_pol = sp.sparse.dok_matrix((N,S),dtype=float)
        pol_strings = []
        for j in range(len(x)):
            v = m.getVarByName(f"x[{j}]")
            if beliefs_given:
                pol_strings.append(f"  Environment {j} --> {round(v.X,precision)}")
                for s in range(S):
                    nat_pol[j,s] = initial_beliefs[j,s]*round(v.X,precision)
            else:
                (n,s) = initial_tuples[j]
                nat_pol[n,s] = round(v.X,precision)
                pol_strings.append(f"  Environment {n}, State {s} --> {round(v.X,precision)}")

        return (round(m.ObjVal,precision),Belief(nat_pol.tocsr()), "\n".join(pol_strings))

    except gp.GurobiError as e:
        print(f"Error code {e.errno}: {e}")

    except AttributeError:
        print("Encountered an attribute error")

def agent_policy(gamma,beliefs_given,initial_beliefs,initial_tuples, precision):
    try:
        # Create a new model
        m = gp.Model("agent")
        m.setParam('OutputFlag', 0)

        # Create variables
        y = m.addVars(len(gamma), vtype=GRB.CONTINUOUS,lb = 0.0, ub=1.0, name='y')
        t = m.addVar(vtype=GRB.CONTINUOUS, name="t")

        # Set objective
        m.setObjective(t, GRB.MAXIMIZE)

        # Add constraints
        if beliefs_given:
            for n in range(N):
                expr = gp.LinExpr()
                for g in range(len(gamma)):
                    alpha = gamma[g].env_values
                    expr.add(sum([alpha[n,s]*initial_beliefs[n,s] for s in range(S)])*y[g])
                m.addConstr(expr >= t, f"c{g}")
        else:
            for j in range(len(initial_tuples)):
                (n,s) = initial_tuples[j]
                expr = gp.LinExpr()
                for g in range(len(gamma)):
                    alpha = gamma[g].env_values
                    expr.add(alpha[n,s]*y[g])
                m.addConstr(expr >= t, f"c{g}")
        m.addConstr(y.sum() == 1)

        # Optimize model
        m.optimize()

        # Contruct policy
        ag_pol = []
        for n in range(len(gamma)):
            v = m.getVarByName(f"y[{n}]")
            if v.X > 0:
                ag_pol.append(f"  Deterministic policy {gamma[n].identifier} --> {round(v.X,precision)}")

        return (round(m.ObjVal,precision),"\n".join(ag_pol))

    except gp.GurobiError as e:
        print(f"Error code {e.errno}: {e}")

    except AttributeError:
        print("Encountered an attribute error")

def AB_HSVI(model,disc,epsilon,results_file):
    global S,state_names,N,environment_names,A,action_names,O,observation_names,transitions,observations,rewards,beliefs_given,initial_beliefs,initial_tuples,a_i
    S,state_names,N,environment_names,A,action_names,O,observation_names,transitions,observations,rewards,beliefs_given,initial_beliefs,initial_tuples = Parser.parse_model(model)
    start_time = time.time()

    a_i = 0
    precision = 6 # Gurobi gives a precion up to 1e-6 (https://docs.gurobi.com/projects/optimizer/en/current/reference/parameters.html#parameteroptimalitytol)
    gamma = initialize_gamma(disc)

    upsilon_det = initialize_upsilon(disc)
    upsilon_nondet = []
    len_g = len(gamma)
    len_u = len(upsilon_nondet)
    i=0
    lower_val, current_bel, _ = nature_policy(gamma,beliefs_given,initial_beliefs,initial_tuples,precision)
    upper_val = sawtooth(upsilon_det,upsilon_nondet,current_bel)
    c_time = time.time() - start_time
    while upper_val-lower_val > epsilon and c_time < 30:
        gamma, upsilon_det, upsilon_nondet = explore(current_bel,gamma, upsilon_det, upsilon_nondet,disc,epsilon, 1)
        if len(gamma) >= 1.1*len_g:
            gamma = prune_gamma(gamma)
            len_g = len(gamma)
        if len(upsilon_nondet) >= 1.1*len_u:
            upsilon_nondet = prune_upsilon(upsilon_det,upsilon_nondet)
            len_u = len(upsilon_nondet)
        lower_val, current_bel, _ = nature_policy(gamma,beliefs_given,initial_beliefs,initial_tuples,precision)
        upper_val = sawtooth(upsilon_det,upsilon_nondet,current_bel)
        c_time = time.time() - start_time
        i+=1

    if c_time < 30:
        return True
    else:
        return False
    
if __name__ == "__main__":
    results = []
    for r in range(100):
        test = f"Birds_MEPOMDP_S3_N3_A3_R{r}"
        print(f"\nStarting test {test}:")
        results.append(AB_HSVI(f"Models/Triviality_test/{test}.txt",0.95,0.5,f"Results/{test}.txt"))
    for r in range(100):
        print(f"MEPOMDP {r}: {results[r]}")
    print(f"Number of trival models: {sum(results)}")