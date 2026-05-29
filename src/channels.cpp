#include <set>
#include "utils.hpp"
#include "channels.hpp"
#include <map>
#include <cassert>

vector<Instruction> QuantumChannel::optimize_error_seq(const vector<Instruction> &old_seq) {
    map<int, Instruction> target_to_custom_ins;

    Instruction IDENTITY(GateName::I, 0);

    for (auto it : old_seq) {
        auto current = to_custom(it);
        assert(it.instruction_type == InstructionType::UnitarySingleQubit);
        int target = it.target;
        if (target_to_custom_ins.find(target) == target_to_custom_ins.end()) {
            target_to_custom_ins[target] = to_custom(IDENTITY);
        }
        target_to_custom_ins[target] = Instruction(target, multiply_matrices(current.matrix, target_to_custom_ins[target].matrix));
    }

    vector<Instruction> new_seq;

    new_seq.reserve(target_to_custom_ins.size());

    for (const auto& it : target_to_custom_ins) {
        if (!are_matrices_equal(IDENTITY.matrix, it.second.matrix)) {
            new_seq.push_back(it.second);
        }
    }

    std::sort(new_seq.begin(), new_seq.end(),
      [](const Instruction &a, const Instruction &b) {
          return a.target < b.target;
         });
    return new_seq;
}

void QuantumChannel::optimize_error_seqs() {
    vector<pair<vector<Instruction>, double>> new_errors_to_probs;
    new_errors_to_probs.reserve(this->errors_to_probs.size());
    for (const auto& e : this->errors_to_probs) {
        if (!is_close(e.second, 0.0))
            new_errors_to_probs.emplace_back(this->optimize_error_seq(e.first), e.second);
    }
    this->errors_to_probs = new_errors_to_probs;
}

void QuantumChannel::merge_same_errors() {
    map<int, set<int>> indices_to_errors;

    for (int index = 0;  index < this->errors_to_probs.size(); ++index) {
        assert(indices_to_errors.find(index) == indices_to_errors.end());
        bool found = false;
        for (int index2 = 0; index2 < index; ++index2) {
            if (are_instruction_seqs_equal(errors_to_probs[index2].first, this->errors_to_probs[index].first)) {
                found = true;
                indices_to_errors[index2].insert(index);
                break;
            }
        }
        if (!found) {
            indices_to_errors[index] = {index};
        }
    }

    vector<pair<vector<Instruction>, double>> new_errors_to_probs;

    for (auto it :indices_to_errors) {
        double new_prob = 0.0;
        for (int index : it.second) {
            new_prob += this->errors_to_probs[index].second;
        }
        new_errors_to_probs.emplace_back(this->errors_to_probs[it.first].first, new_prob);
    }

    this->errors_to_probs = new_errors_to_probs;
}

void QuantumChannel::optimize() {
    this->optimize_error_seqs();
    this->merge_same_errors();
}

bool QuantumChannel::is_normalized() {
    double total = 0;
    for (auto it : this->errors_to_probs) {
        total += it.second;
    }

    return total == 1;
}

void QuantumChannel::normalize() {
    double total_prob = 0.0;
    int current_index = 1;
    int total_entries = this->errors_to_probs.size();
    for (auto &it : this->errors_to_probs) {
        it.second = round_to(it.second, 10);
        total_prob += it.second;
    }

    for (auto &it : this->errors_to_probs) {
        it.second /= total_prob;
    }

    total_prob = 0.0;
    for (auto &it : this->errors_to_probs) {
        if (total_entries == current_index) {
            it.second = 1-total_prob;
        }
        total_prob += it.second;
        current_index++;
    }
}

bool MeasurementChannel::is_normalized() {
    return (this->correct_0 + this->incorrect_0 == 1.0) && (this->correct_1 + this->incorrect_1 == 1.0);
}

void MeasurementChannel::normalize() {
    this->incorrect_0 = 1-this->correct_0;
    this->incorrect_1 = 1-this->correct_1;
}

QuantumChannel::QuantumChannel(json &data) {

    vector<double> probabilities = data["probabilities"];
    vector<json> all_errors = data["errors"];

    assert(probabilities.size() == all_errors.size());

    for (int i = 0; i < probabilities.size(); i++) {
        vector<Instruction> final_errors;
        double probability = probabilities[i];

        
        for (const auto & j : all_errors[i]){
            Instruction instruction = Instruction(j);
            final_errors.push_back(instruction);
        }
        
        this->errors_to_probs.emplace_back(final_errors, probability);

    }

    //estimating success probability
    this->estimated_success_prob = 0.5;
    for (const auto& it : this->errors_to_probs) {
        auto instruction_seq = it.first;
        auto prob = it.second;
        if (is_identity(instruction_seq) or prob > 0.5) {
                this->estimated_success_prob = max(this->estimated_success_prob, prob);
        }
    }
}

QuantumChannel::QuantumChannel() {
    // initialize to a quantum channel with no error
    vector<Instruction> errors = {Instruction(GateName::I, 0)};
    this->errors_to_probs.emplace_back(errors, 1.0);
}

MeasurementChannel::MeasurementChannel(json &json_val) {
    this->correct_0 = json_val["0"]["0"];
    this->incorrect_0 = json_val["0"]["1"];
    this->correct_1 = json_val["1"]["1"];
    this->incorrect_1 = json_val["1"]["0"];
}

MeasurementChannel::MeasurementChannel(double correct0, double correct1) {
    this->correct_0 = correct0;
    this->correct_1 = correct1;
    this->incorrect_0 = 1 - correct_0;
    this->incorrect_1 = 1 - correct_1;
}

MyFloat MeasurementChannel::get_ind_probability(int ideal_outcome, int noisy_outcome) const{
    if (ideal_outcome ==  noisy_outcome) {
        if (noisy_outcome == 0) {
            return this->correct_0;
        } else {
            return this->correct_1;
        }
    } else {
        if (ideal_outcome == 0) {
            return this->incorrect_0;
        } else {
            return this->incorrect_1;
        }
    }
}
