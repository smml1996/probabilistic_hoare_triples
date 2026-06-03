
#ifndef CHANNELS_H
#define CHANNELS_H
#include <vector>
#include "instruction.hpp"
#include <nlohmann/json.hpp>
using namespace std;

using json = nlohmann::json;

// ---- Base Class ----
class Channel {
public:
    double estimated_success_prob = 0;
    virtual ~Channel() = default;
    virtual bool is_normalized() const;
    virtual void normalize()  = 0;
    virtual shared_ptr<Channel> rename(const unordered_map<int, int> &rev_embedding) = 0;
};


class QuantumChannel : public Channel {
    static vector<Instruction> optimize_error_seq(const vector<Instruction> &old_seq);
    void optimize_error_seqs();
    void merge_same_errors();
    vector<Instruction> rename_error_seq(const vector<Instruction> &old_seq, const unordered_map<int, int> &rev_embedding) const;
public:
    vector<pair<vector<Instruction>, double>>errors_to_probs;
    QuantumChannel(json &data);
    QuantumChannel();

    void optimize();
    bool is_normalized() const override;
    void normalize() override;
    shared_ptr<Channel> rename(const unordered_map<int, int> &rev_embedding) override;
    bool operator==(const QuantumChannel &other) const;
    void print_channel() const;
};

class MeasurementChannel : public Channel {
public:
    double correct_0;   // probability of receiving 0 and that it is actually 0
    double correct_1;   // probability of receiving 1 and that it is actually 1
    double incorrect_0; // probability of receiving 0 and that it is actually 1
    double incorrect_1; // probability of receiving 1 and that it is actually 0
    MeasurementChannel(json &data);
    MeasurementChannel(const double &correct0, const double &correct1);
    MyFloat get_ind_probability(const int &ideal_outcome, const int &noisy_outcome) const;
    bool is_normalized() const override;
    void normalize() override;
    shared_ptr<Channel> rename(const unordered_map<int, int> &rev_embedding) override;
    bool operator==(const MeasurementChannel &other) const;
};
#endif
