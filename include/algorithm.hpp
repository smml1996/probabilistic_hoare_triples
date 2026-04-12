#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <string>
#include <vector>
#include <unordered_map>
#include "pomdp.hpp"
#include <filesystem>
#include "instruction.hpp"
#include <nlohmann/json.hpp>

using namespace std;

namespace fs = std::filesystem;

inline POMDPAction random_branch("RANDOM_BRANCH", {}, 1, {});
inline POMDPAction HALT_ACTION("HALT__", {}, -1, {});

class Algorithm {
public:
    shared_ptr<POMDPAction> action;
    vector<shared_ptr<Algorithm>> children;
    cpp_int classical_state;
    int depth;
    int precision;
    unordered_map<int, double> children_probs;

    Algorithm(const shared_ptr<POMDPAction> &action, const cpp_int &classical_state, int precision, int depth=-1);

    explicit Algorithm(json &data);
    bool exist_child_with_cstate(const cpp_int &cstate) const;
    bool operator==(const Algorithm &algorithm) const;
    bool has_meas() const;
    bool has_classical_instruction() const;
    bool is_unitary() const; // returns true if all instructions used by the action of the current node use only unitary instructions (children are not checked)
    void get_successor_classical_states(const cpp_int &current_classical_state, unordered_set<cpp_int> &result) const; // returns a set containing all reachable classical states by executing the current acction (children are not checked)
};

string to_string(shared_ptr<Algorithm> algorithm, const string& tabs="");
string v_to_string(shared_ptr<Algorithm> algorithm, const string& tabs ="");

bool dump_to_file(const fs::path &, const shared_ptr<Algorithm> &);

bool dump_raw_algorithm(const fs::path &, const shared_ptr<Algorithm> &);

int get_algorithm_from_list(const vector<shared_ptr<Algorithm>> &algorithms, const shared_ptr<Algorithm> &new_algorithm);

int algorithm_exists(const unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm, const shared_ptr<Algorithm> &algorithm);

shared_ptr<Algorithm> deep_copy_algorithm(shared_ptr<Algorithm> algorithm);

void get_algorithm_end_nodes(const shared_ptr<Algorithm> &algorithm, vector<shared_ptr<Algorithm>> &end_nodes);

inline json to_json(const Algorithm &a) {
    vector<json> j_children;
    for (const auto& child : a.children) {
        j_children.push_back(to_json(*child));
    }

    return json{
                {"action", to_json(*a.action)},
                {"children", j_children},
                {"classical_state", a.classical_state.str()},
                {"depth", a.depth},
                {"precision", a.precision},
                {"children_probs", a.children_probs}
    };
}

shared_ptr<Algorithm> normalize_algorithm(const shared_ptr<Algorithm> &algorithm);

#endif
