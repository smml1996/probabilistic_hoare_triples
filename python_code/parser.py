from typing import Dict, List, Set


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

    def __init__(self, name: str):
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

    def get_connected_vertices(self, start_v: int):
        queue = [start_v]
        visited = set()
        visited.add(start_v)

        while len(queue) > 0:
            current = queue.pop(0)

            for succ in self.states:
                if succ not in visited:
                    for action in self.actions:
                        prob = self.get_trans(current, action, succ)
                        if prob > 0.0:
                            visited.add(succ)
                            queue.append(succ)

        return visited


