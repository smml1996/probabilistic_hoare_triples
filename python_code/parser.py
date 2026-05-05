from typing import Dict, List, Set
from scipy.spatial.distance import jensenshannon
import os
import math

class POMDP:
    states: Set[int]
    actions: Set[int]
    observations: Set[int]
    trans_f: Dict[int, Dict[int, Dict[int, float]]] # v_from, action, v_to -> prob
    obs_f: Dict[int, Dict[int, Dict[int, float]]] # action, v_to, obs -> prob
    rewards: Dict[int, Dict[int, float]] # vertex, action -> reward
    enemy_states_d: Dict[float, Set[int]]
    friend_states_d: Dict[float, Set[int]]

    def get_reward(self, v: int, action: int) -> float:
        if v not in self.rewards.keys():
            return 0.0
        if action not in self.rewards[v].keys():
            return 0.0
        return self.rewards[v][action]

    def __init__(self, name: str="", is_navigation=False, is_empty=False):
        self.enemy_states_d = {}
        self.friend_states_d = {}
        self.states = set()
        self.actions = set()
        self.observations = set()
        self.trans_f = {}
        self.obs_f = {}
        self.rewards = {}
        if not is_empty:
            self.name = name
            f = open(name)
            lines = f.readlines()
            num_states = int(lines[0])
            num_actions = int(lines[1])
            num_observations = int(lines[2])

            for i in range(num_states):
                self.states.add(i)
            for i in range(num_actions):
                self.actions.add(i)
            for i in range(num_observations):
                self.observations.add(i)

            for line in lines[3:]:
                if len(line) < 3:
                    continue
                tokens = line.split(",")
                if tokens[0] == "T":
                    v_from = int(tokens[1])
                    action = int(tokens[2])
                    v_to = int(tokens[3])
                    prob = float(tokens[4])
                    if v_from not in self.trans_f.keys():
                        self.trans_f[v_from] = {}
                    if action not in self.trans_f[v_from].keys():
                        self.trans_f[v_from][action] = {}
                    self.trans_f[v_from][action][v_to] = prob
                elif tokens[0] == "R":
                    v = int(tokens[1])
                    action = int(tokens[2])
                    reward = float(tokens[3])

                    if v not in self.rewards.keys():
                        self.rewards[v] = {}
                    self.rewards[v][action] = reward
                elif tokens[0] == "O":
                    action = int(tokens[1])
                    v = int(tokens[2])
                    obs = int(tokens[3])
                    prob = float(tokens[4])

                    if action not in self.obs_f.keys():
                        self.obs_f[action] = {}
                    if v not in self.obs_f[action].keys():
                        self.obs_f[action][v] = {}
                    self.obs_f[action][v][obs] = prob
                else:
                    raise Exception("cannot parse line: " + line)
            f.close()

            if not is_navigation:
                self.get_enemy_states()
                self.get_friend_states()
                self.check_type_states(self.enemy_states_d)
                self.check_type_states(self.friend_states_d)

    def get_trans(self, state0, action, state1):
        if state0 not in self.trans_f.keys():
            return 0.0
        if action not in self.trans_f[state0].keys():
            return 0.0
        if state1 not in self.trans_f[state0][action].keys():
            return 0.0
        return self.trans_f[state0][action][state1]

    def get_obs_prob(self, action, v, obs):
        if action not in self.obs_f.keys():
            return 0.0
        if v not in self.obs_f[action].keys():
            return 0.0
        if obs not in self.obs_f[action][v].keys():
            return 0.0
        return self.obs_f[action][v][obs]

    def is_state_absorbing(self, state: int) -> bool:
        for action in self.actions:
            for succ in self.states:
                if succ != state:
                    prob = self.get_trans(state, action, succ)
                    if prob > 0.0:
                        return False
        return True

    def find_absorbing_states(self):
        result = set()
        for state in self.states:
            if self.is_state_absorbing(state):
                result.add(state)
        return result

    def is_d_to_absorbing1(self, state: int) -> bool:
        absorbing_states = self.find_absorbing_states()
        for action in self.actions:
            if state in self.trans_f[state].keys():
                if action in self.trans_f[state][action].keys():
                    for ab_state in absorbing_states:
                        if ab_state in self.trans_f[state][action].keys():
                            if self.trans_f[state][action][ab_state] > 0:
                                return True
        return True

    def get_enemy_states(self):
        action = 3
        for state in self.states:
            reward = self.get_reward(state, action)
            if reward > 0:
                if reward not in self.enemy_states_d.keys():
                    self.enemy_states_d[reward] = set()
                self.enemy_states_d[reward].add(state)

    def get_friend_states(self):
        action = 3
        for state in self.states:
            reward = self.get_reward(state, action)
            if reward < 0:
                if reward not in self.friend_states_d.keys():
                    self.friend_states_d[reward] = set()
                self.friend_states_d[reward].add(state)

    def check_type_states(self, d: Dict[float, Set[int]]):
        for (key, values) in d.items():
            assert(len(values) == 5)

    def get_num_distances(self):
        assert len(self.friend_states_d.keys()) == len(self.enemy_states_d.keys())
        return len(self.friend_states_d.keys())

    def get_connected_vertices(self, start_v: int, reverse=False):
        queue = [start_v]
        visited = set()
        visited.add(start_v)

        while len(queue) > 0:
            current = queue.pop(0)

            for succ in self.states:
                if succ not in visited:
                    for action in self.actions:
                        if not reverse:
                            prob = self.get_trans(current, action, succ)
                        else:
                            prob = self.get_trans(succ, action, current)
                        if prob > 0.0:
                            visited.add(succ)
                            queue.append(succ)

        return visited

    def get_bfs_distances(self, start_vs: Set[int], is_reverse=True):
        queue = []
        visited = set()
        result = dict()
        result[0] = set()

        for start_v in start_vs:
            queue.append((start_v, 0))
            visited.add(start_v)
            result[0].add(start_v)

        while len(queue) > 0:
            current, bfs_d = queue.pop(0)

            for succ in self.states:
                if succ not in visited:
                    for action in self.actions:
                        if is_reverse:
                            prob = self.get_trans(succ, action, current)
                        else:
                            prob = self.get_trans(current, action, succ)
                        if prob > 0.0:
                            visited.add(succ)
                            new_d = bfs_d + 1
                            queue.append((succ, new_d))
                            if new_d not in result.keys():
                                result[new_d] = set()
                            result[new_d].add(succ)

        return result

    def get_goal_states(self):
        result = set()
        actions = set()
        for state in self.states:
            for action in self.actions:
                if self.get_reward(state, action) > 0:
                    result.add(state)
                    actions.add(action)
        return (result, actions)

    def get_action_similarity(self, state1, state2, action1):
        v1 = [0.0 for _ in range(len(self.observations))]
        v2 = [0.0 for _ in range(len(self.observations))]

        for succ in self.states:
            t_prob1 = self.get_trans(state1, action1, succ)
            t_prob2 = self.get_trans(state2, action1, succ)
            for obs in self.observations:
                v1[obs] += t_prob1 * self.get_obs_prob(action1, succ, obs)
                v2[obs] += t_prob2 * self.get_obs_prob(action1, succ, obs)
        return jensenshannon(v1, v2)

    def get_state_similarity(self, state1, state2):
        val = 0.0

        for action in self.actions:
            val = self.get_action_similarity(state1, state2, action)

        return val/len(self.actions)

    def re_index(self):
        old_to_new_states = {}

        last_index = 0
        states = list(self.states)
        sorted_states = sorted(states)
        for state in sorted_states:
            old_to_new_states[state] = last_index
            last_index += 1

        self.states = set([i for i in range(0, last_index)])

        new_trans_f = {}
        for (state, d_state) in self.trans_f.items():
            new_state = old_to_new_states[state]
            if new_state not in new_trans_f.keys():
                new_trans_f[new_state] = {}
            for (action , d_action) in d_state.items():
                if action not in new_trans_f[new_state].keys():
                    new_trans_f[new_state][action] = {}
                for (ss, prob) in d_action.items():
                    new_ss = old_to_new_states[ss]
                    new_trans_f[new_state][action][new_ss] = round(prob, 7)

        self.trans_f = new_trans_f

        new_obs_f = {}
        for (action, action_d) in self.obs_f.items():
            if action not in new_obs_f.keys():
                new_obs_f[action] = {}

            for (v, v_d) in action_d.items():
                new_v = old_to_new_states[v]
                if new_v not in new_obs_f[action].keys():
                    new_obs_f[action][new_v] = {}

                for (obs, prob) in v_d.items():
                    new_obs_f[action][new_v][obs] = round(prob, 7)

        self.obs_f = new_obs_f

        new_reward_f = {}

        for (v, v_dict) in self.rewards.items():
            new_v = old_to_new_states[v]
            if new_v not in new_reward_f.keys():
                new_reward_f[new_v] = {}
            for (action, reward) in v_dict.items():
                new_reward_f[new_v][action] = round(reward, 7)

        self.rewards = new_reward_f
        return old_to_new_states


    def normalize_obs_f(self):
        for (action, action_d) in self.obs_f.items():
            for (v, v_d) in action_d.items():
                total_prob = 0.0
                for (obs, prob) in v_d.items():
                    total_prob += prob

                total_prob = round(total_prob, 7)
                for (obs, prob) in v_d.items():
                    self.obs_f[action][v][obs] = round(v_d[obs]/total_prob, 7)

    def normalize_trans_f(self):
        for (state, d_state) in self.trans_f.items():
            for (action , d_action) in d_state.items():
                total_prob = 0.0
                for (ss, prob) in d_action.items():
                    total_prob += prob
                total_prob = round(total_prob, 7)
                for (ss, prob) in d_action.items():
                    self.trans_f[state][action][ss] = round(prob / total_prob, 7)

    def check(self):
        for (state, d_state) in self.trans_f.items():
            for (action , d_action) in d_state.items():
                total_prob = 0.0
                for (ss, prob) in d_action.items():
                    total_prob += prob
                total_prob = round(total_prob, 7)
                assert(math.isclose(total_prob, 1, rel_tol=1e-7, abs_tol=1e-7))

        for (action, action_d) in self.obs_f.items():
            for (v, v_d) in action_d.items():
                total_prob = 0.0
                for (obs, prob) in v_d.items():
                    total_prob += prob

                total_prob = round(total_prob, 7)
                assert (math.isclose(total_prob, 1, rel_tol=1e-7, abs_tol=1e-7))


    def normalize(self):
        d = self.re_index()
        self.normalize_trans_f()
        self.normalize_obs_f()
        self.check()
        return d

    def write_test_case(self, initial_states: Set[int], name: str):
        file_path = os.path.join("..", "AB-HSVI_NeurIPS_2025", "Models", f"{name}" )
        f = open(file_path, "w")

        states_names = []

        for state in self.states:
            states_names.append(f"s{state}")

        actions_names = []
        for action in self.actions:
            actions_names.append(f"a{action}")

        observations_names = []
        for obs in self.observations:
            observations_names.append(f"o{obs}")

        str_states_names = ",".join(states_names)
        str_actions_names = ",".join(actions_names)
        str_observations_names = ",".join(observations_names)
        f.write(f"{len(self.states)}, [{str_states_names}]\n")
        f.write("1, [e1]\n")
        f.write(f"{len(self.actions)}, [{str_actions_names}]\n")
        f.write(f"{len(self.observations)}, [{str_observations_names}]\n")
        f.write("\n")
        f.write("0, []\n")
        f.write("\n")

        f.write("# Transition function (s,a,s -> p)\n")

        for v in self.states:
            for a in self.actions:
                for vv in self.states:
                    prob = self.get_trans(v, a, vv)
                    if prob > 0:
                        f.write(f"{v},{a},{vv} -> {prob:.7f}\n")
        f.write("\n")

        f.write("# Observation function (a,s,o -> p)\n")
        for action in self.actions:
            for v in self.states:
                for obs in self.observations:
                    prob = self.get_obs_prob(action, v, obs)
                    if prob > 0:
                        f.write(f"{action},{v},{obs} -> {prob:.7f}\n")

        f.write("\n")

        f.write("# Reward function (s,a -> r)\n")
        for v in self.states:
            for a in self.actions:
                reward = self.get_reward(v, a)
                if reward != 0:
                    f.write(f"{v},{a} -> {reward:.7f}\n")

        f.write("\n")

        f.write("# Initial tuples (n,s)\n")
        for state in initial_states:
            f.write(f"0,{state}\n")
        f.close()