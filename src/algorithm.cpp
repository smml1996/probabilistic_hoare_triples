#include "algorithm.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include "utils.hpp"

Algorithm::Algorithm(const shared_ptr<POMDPAction> &action, const cpp_int &classical_state, int precision, int depth) {
    assert(action != nullptr);
    assert(action->name.size() > 0);
    this->action = action;
    this->classical_state = classical_state;
    this->depth = depth;
    this->children_probs = unordered_map<int, double>();
    this->precision = precision;
}

Algorithm::Algorithm(json &data) {
    this->action = make_shared<POMDPAction>(data["action"]);
    this->classical_state.assign(data["classical_state"].get<string>());
    this->depth = data["depth"].get<int>();
    this->precision = data["precision"].get<int>();

    for (auto j_child : data["children"]) {
        this->children.push_back(make_shared<Algorithm>(j_child));
    }

    for (auto p : data["children_probs"]) {
        int index = p[0].get<int>();
        this->children_probs[index] = p[1].get<double>();
    }
}


bool Algorithm::exist_child_with_cstate(const cpp_int &cstate) const {
    for (auto child : this->children) {
        if(child->classical_state == cstate) {
            return true;
        }
    }
    return false;
}

bool Algorithm::operator==(const Algorithm &other) const {
    if (!(*other.action == *this->action)) {
        // cout << "action is different: " << this->action->name << " " << other.action->name << endl;
        return false;
    }
    if (this->classical_state != other.classical_state) {
        // cout << "classical state is different: " << this->classical_state << " " << other.classical_state << endl;
        return false;
    }

    if (this->children.size() != other.children.size()) {
        // cout << "children size is different: " << this->children.size() << " " << other.children.size() << endl;
        return false;
    }

    // check children are the same

    int c = 0;

    for (auto child : this->children) {
        double prob_child;
        if (this->children_probs.find(c) != this->children_probs.end()) {
            prob_child = this->children_probs.at(c);
        } else {
            prob_child = 0;
        }
        bool found = false;
        int c2 = 0;
        for (auto other_child : other.children) {
            double prob_other;
            if (other.children_probs.find(c2) != other.children_probs.end()) {
                prob_other = other.children_probs.at(c2);
            } else {
                prob_other = 0;
            }


            if (is_close(prob_other, prob_child, this->precision)) {
                if (*other_child == *child) {
                    found = true;
                    break;
                }
            }
            c2 += 1;
        }
        c += 1;
        if (!found) {
            return false;
        }
    }
    return true;
}

string to_string(shared_ptr<Algorithm> algorithm, const string& tabs) {
    if (algorithm == nullptr) return "";

    string result = "";
    if (*algorithm->action == random_branch) {
        if (algorithm->children.size() == 1) {
            return to_string(algorithm->children.at(0));
        }
        if (algorithm->children.size() == 2) {
            result += tabs + "{\n";
            string current_tabs = tabs + "\t";

            double condition_prob = algorithm->children_probs.at(1);

            result += to_string(algorithm->children.at(0), tabs + "\t");
            result += "\n} ⊕_" + to_string(condition_prob) + " {\n";
            result += to_string(algorithm->children.at(1), tabs+ "\t");
            result += tabs + "}\n";
            return result;

        }
        assert(algorithm->children.size() > 2);
        result += tabs + "{\n";
        string current_tabs = tabs + "\t";

        auto temp_algorithm = make_shared<Algorithm>(algorithm->action, algorithm->classical_state, algorithm->precision, algorithm->depth);
        double condition_prob = 0.0;
        vector<shared_ptr<Algorithm>> new_children;
        for (size_t i = 1; i < algorithm->children.size(); ++i) {
            condition_prob += algorithm->children_probs.at(i);
            temp_algorithm->children.push_back(algorithm->children.at(i));
            temp_algorithm->children_probs[i-1] = algorithm->children_probs.at(i);
        }

        result += to_string(algorithm->children.at(0), tabs + "\t");
        result += "\n } ⊕_" + to_string(condition_prob) + " {\n";
        result += to_string(temp_algorithm, tabs+ "\t");
        result += tabs + "}\n";
    } else {
        result = tabs + to_string(algorithm->action) + "\n";
        for(auto child : algorithm->children) {
            string child_alg;
            {
                if (algorithm->children.size() > 1) {
                    result += tabs + "if c = " + child->classical_state.str() + ":\n" ;
                    child_alg = to_string(child, tabs+"\t");
                } else {
                    child_alg = to_string(child, tabs);
                }
            }
            result += child_alg;
        }
    }
    return result;
}
string to_string_if_else(const vector<shared_ptr<Algorithm>> &children, const int &current_index, const string &tabs) {
    auto classical_val = children.at(current_index)->classical_state;
    string result = tabs + "if (" + to_string(classical_val) + ") {\n";
    result += v_to_string(children.at(current_index), tabs+"\t");
    result += "\n";
    result += tabs + "} else {\n";
    if (current_index == children.size() -1) {
        result += tabs + "skip\n";
    } else {
        result += to_string_if_else(children, current_index+1, tabs + "\t");
    }
    result += tabs + "}\n";
    return result;
}

string v_to_string(shared_ptr<Algorithm> algorithm, const string& tabs) {
    if (algorithm == nullptr) return "";

    string result = "";
    if (*algorithm->action == random_branch) {
        if (algorithm->children.size() == 1) {
            return v_to_string(algorithm->children.at(0));
        }
        if (algorithm->children.size() == 2) {
            result += tabs + "{\n";
            string current_tabs = tabs + "\t";

            double condition_prob = algorithm->children_probs.at(1);

            result += v_to_string(algorithm->children.at(0), tabs + "\t");
            result += "\n} %" + to_string(condition_prob) +" %" + " {\n";
            result += v_to_string(algorithm->children.at(1), tabs+ "\t");
            result += tabs + "}\n";
            return result;

        }
        assert(algorithm->children.size() > 2);
        result += tabs + "{\n";
        string current_tabs = tabs + "\t";

        auto temp_algorithm = make_shared<Algorithm>(algorithm->action, algorithm->classical_state, algorithm->precision, algorithm->depth);
        double condition_prob = 0.0;
        vector<shared_ptr<Algorithm>> new_children;
        for (size_t i = 1; i < algorithm->children.size(); ++i) {
            condition_prob += algorithm->children_probs.at(i);
            temp_algorithm->children.push_back(algorithm->children.at(i));
            temp_algorithm->children_probs[i-1] = algorithm->children_probs.at(i);
        }

        result += v_to_string(algorithm->children.at(0), tabs + "\t");
        result += "\n } % " + to_string(condition_prob) + " % {\n";
        result += v_to_string(temp_algorithm, tabs+ "\t");
        result += tabs + "}\n";
    } else {
        result = tabs + v_to_string(algorithm->action) + "\n";
        if (algorithm->children.size() > 0) {
            if (algorithm->children.size() == 1) {
                result += v_to_string(algorithm->children.at(0), tabs);
            } else {
                result += to_string_if_else(algorithm->children, 0, tabs);
            }
        }
    }
    return result;
}

bool dump_to_file(const fs::path &path, const shared_ptr<Algorithm> &algorithm) {
    // Open file for writing
    std::ofstream out(path);  // creates the file or overwrites if it exists
    if (!out) {
        std::cerr << "Failed to open file: " << path << "\n";
        return false;
    }
    out << to_string(algorithm);

    out.close();
    return true;
}

bool dump_raw_algorithm(const fs::path &p, const shared_ptr<Algorithm> &a) {

    // Dump into a file
    std::ofstream file(p);
    if (!file) {
        std::cerr << "Error opening file for writing\n";
        return false;
    }
    auto j = to_json(*a);
    file << j.dump(4);  // "4" = pretty print with indentation
    file.close();
    return true;

}

int get_algorithm_from_list(const vector<shared_ptr<Algorithm>> &algorithms, const shared_ptr<Algorithm> &new_algorithm) {
    int index = 0;
    for (auto algorithm : algorithms) {
        if (*algorithm == *new_algorithm) {
            return index;
        }
        index++;
    }
    return -1;
}

int algorithm_exists(const unordered_map<int, shared_ptr<Algorithm>> &mapping_index_algorithm, const shared_ptr<Algorithm> &algorithm) {
    for (auto it : mapping_index_algorithm) {
        if (it.second == nullptr) {
            if (algorithm == nullptr) {
                return it.first;
            }
        } else if (algorithm != nullptr && *it.second == *algorithm) {
            return it.first;
        }
    }

    return -1;
}

shared_ptr<Algorithm> deep_copy_algorithm(shared_ptr<Algorithm> algorithm)  {
    if (algorithm == nullptr) return algorithm;
    string action = algorithm->action->name;
    auto classical_state = algorithm-> classical_state;
    int depth = algorithm->depth;

    auto algorithm_copy = make_shared<Algorithm>(algorithm->action, classical_state, algorithm->precision, depth);

    for (auto it : algorithm->reachable_states) {
        algorithm_copy->reachable_states.push_back(it);
    }

    for (auto child : algorithm->children) {
        algorithm_copy->children.push_back(deep_copy_algorithm(child));
    }

    return algorithm_copy;
}

bool Algorithm::has_meas() const {
    for (auto instruction : this->action->instruction_sequence) {
        if (instruction.instruction_type == InstructionType::Measurement) {
            return true;
        }
    }

    return false;
}

bool Algorithm::has_classical_instruction() const {
    for (auto instruction : this->action->instruction_sequence) {
        if (instruction.instruction_type == InstructionType::Classical) {
            return true;
        }
    }

    return false;

}
bool Algorithm::is_unitary() const {
    for (auto instruction : this->action->instruction_sequence) {
        if ((instruction.instruction_type != InstructionType::UnitarySingleQubit) && (instruction.instruction_type != InstructionType::UnitaryMultiQubit)) {
            return false;
        }
    }

    return true;
}

void Algorithm::get_successor_classical_states(const cpp_int &current_classical_state,
    unordered_set<cpp_int> &result) const {
    // get all bits that might change
    unordered_set<int> bits;
    auto copy_current_classical_state = current_classical_state;

    for (const auto& instruction : this->action->instruction_sequence) {
        if (instruction.instruction_type == InstructionType::Measurement) {
            bits.insert(instruction.c_target);
        } else {
            auto t = instruction.c_target;
            assert(bits.find(t) == bits.end());
            if (instruction.gate_name == GateName::Write0) {
                copy_current_classical_state = copy_current_classical_state & ~(1 << instruction.c_target);
            } else if (instruction.gate_name == GateName::Write1) {
                copy_current_classical_state = copy_current_classical_state | (1 << instruction.c_target);
            }
        }
    }

    result.insert(copy_current_classical_state);

    for (auto bit : bits) {
        unordered_set<cpp_int> new_states;
        for (auto c_state : result) {
            new_states.insert(c_state ^ (1 << bit)); // toggle bit
        }
        for (const auto& n : new_states) {
            result.insert(n);
        }
    }

}



void get_algorithm_end_nodes(const shared_ptr<Algorithm> &algorithm, vector<shared_ptr<Algorithm>> &end_nodes) {
    if (algorithm->children.empty()) {
        if (!(*algorithm->action == HALT_ACTION)) {
            end_nodes.push_back(algorithm);
        }

        return;
    }

   if (algorithm->has_meas()) {
        unordered_set<cpp_int> all_c_succs;
        algorithm->get_successor_classical_states(algorithm->classical_state, all_c_succs);
        if (algorithm->children.size() < all_c_succs.size()) {
            // TODO: this can be better (some classical states cannot happen)
            // assert(algorithm->children.size() == 1);
            end_nodes.push_back(algorithm);

        }
   }
    

    for (const auto& child : algorithm->children) {
        get_algorithm_end_nodes(child, end_nodes);
    }
}

unordered_set<cpp_int> get_possible_next_cstates(const shared_ptr<Algorithm> &node) {
    assert (node->has_meas());

    unordered_set<cpp_int> all_next_cstates;
    node->get_successor_classical_states(node->classical_state, all_next_cstates);
    for (const auto& child : node->children) {
        assert(all_next_cstates.find(child->classical_state) != all_next_cstates.end());
        all_next_cstates.erase(child->classical_state);
    }
    return all_next_cstates;
}


shared_ptr<Algorithm> normalize_algorithm(const shared_ptr<Algorithm> &algorithm) {
    // normalize algorithm
    if (algorithm == nullptr) {
        return make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), 0, -1, 0);
    }
    shared_ptr<Algorithm> current_algorithm = deep_copy_algorithm(algorithm);
    if (*algorithm->action == HALT_ACTION) {
        return current_algorithm;
    }
    vector<shared_ptr<Algorithm>> end_nodes;
    get_algorithm_end_nodes(current_algorithm, end_nodes);
    for (const auto& end_node : end_nodes) {

        if (end_node->has_meas()) {
            unordered_set<cpp_int> all_c_succs = get_possible_next_cstates(end_node);

            for (const auto& c : all_c_succs) {
                shared_ptr<Algorithm> halt_node = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), c, end_node->precision, end_node->depth+1);
                end_node->children.push_back(halt_node);
            }

        } else {
            assert(end_node->children.empty());
            if (!(*end_node->action == HALT_ACTION)) {
                shared_ptr<Algorithm> halt_node = make_shared<Algorithm>(make_shared<POMDPAction>(HALT_ACTION), end_node->classical_state, end_node->precision, end_node->depth+1);
                end_node->children.push_back(halt_node);
            }
        }
    }
    return current_algorithm;
}
