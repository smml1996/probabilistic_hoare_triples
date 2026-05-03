from typing import Dict, List, Set
from scipy.spatial.distance import jensenshannon


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

    def __init__(self, name: str, is_navigation=False):
        self.enemy_states_d = {}
        self.friend_states_d = {}
        self.states = set()
        self.actions = set()
        self.observations = set()
        self.trans_f = {}
        self.obs_f = {}
        self.rewards = {}
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

    def get_bfs_distances(self, start_vs: Set[int]):
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
                        prob = self.get_trans(succ, action, current)
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


