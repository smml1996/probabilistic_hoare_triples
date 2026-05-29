
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
    inline virtual bool is_normalized() {return false;};
    inline virtual void normalize() {};
};


class QuantumChannel : public Channel {
    static vector<Instruction> optimize_error_seq(const vector<Instruction> &old_seq);
    void optimize_error_seqs();
    void merge_same_errors();
public:
    vector<pair<vector<Instruction>, double>>errors_to_probs;
    explicit QuantumChannel(json &data);
    QuantumChannel();

    void optimize();
    bool is_normalized() override;
    void normalize() override;
};

class MeasurementChannel : public Channel {
public:
    double correct_0;   // probability of receiving 0 and that it is actually 0
    double correct_1;   // probability of receiving 1 and that it is actually 1
    double incorrect_0; // probability of receiving 0 and that it is actually 1
    double incorrect_1; // probability of receiving 1 and that it is actually 0
    MeasurementChannel(json &data);
    MeasurementChannel(double correct0, double correct1);
    MyFloat get_ind_probability(int ideal_outcome, int noisy_outcome) const;
    bool is_normalized() override;
    void normalize() override;
};
#endif
