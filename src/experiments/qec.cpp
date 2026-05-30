//
// Created by Stefanie Muroya Lei on 29.05.26.
//

#include "experiments.hpp"


class ThreeQubitCode : public QuantumExperiment {
    int c0 = 0;
    int c1 = 1;
    int c3 = 2;

    // We are goind to use Choi States, hence the copies
    int q0 = 0;
    int q1 = 1;
    int q2 = 2;

    int qmeas0 = 3;
    int qmeas1 = 4;

    // int q0_copy = 5;
    // int q1_copy = 6;
    // int q2_copy = 7;

    const int NoError = 0;
    const int X0Error = 1;
    const int X1Error = 2;
    const int X2Error = 3;

    vector<int> ALL_ERRORS{NoError, X0Error, X1Error, X2Error};

    vector<Instruction> MeasZ1Seq;
    vector<Instruction> MeasZ2Seq;

    bool is_repeated_embedding(const vector<unordered_map<int, int>> &all_embeddings, const unordered_map<int, int> &current) const {
        unordered_set<int> current_set;
        for (auto it : current) {
            current_set.insert(it.second);
        }

        for (auto embedding : all_embeddings) {
            unordered_set<int> temp_s{embedding[q0], embedding[q1], embedding[q2]};
            if (temp_s == current_set)
                return true;
        }
        return false;
    }

    vector<pair<int, int>> get_meas_qubits(const HardwareSpecification &hs, const vector<int> &current_set) const {
        int current_q0 = current_set[0];
        int current_q1 = current_set[1];
        int current_q2 = current_set[2];

        set<int> succs0 = hs.get_ctargets(current_q0);
        set<int> succs1 = hs.get_ctargets(current_q1);
        set<int> succs2 = hs.get_ctargets(current_q2);


        set<int> intersection01;

        set_intersection(
            succs0.begin(), succs0.end(),
            succs1.begin(), succs1.end(),
            inserter(intersection01, intersection01.begin())
        );

        set<int> intersection12;

        set_intersection(
            succs1.begin(), succs1.end(),
            succs2.begin(), succs2.end(),
            inserter(intersection12, intersection12.begin())
        );

        vector<pair<int, int>> result;

        for (auto m0 : intersection01) {
            for (auto m1 : intersection12) {
                result.push_back({m0, m1});
            }
        }

        return result;
    }
protected:
    void set_experiment_name() override {
        this->name = "three_qubit_code";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 8;
    }

    void set_qubits_used() override {
        for (auto q : {q0, q1, q2, qmeas0, qmeas1}) {
            this->qubits_used.push_back(q);
        }
    }

    void set_hardware_specs() override {
        this->hw_list.clear();
        for(int i = 0; i < QuantumHardware::HardwareCount; i++)  {
            QuantumHardware qw = static_cast<QuantumHardware>(i);
            HardwareSpecification hs(qw, this->with_thermalization, this->optimize);
            if (hs.basis_gates_type != BasisGates::TYPE5 && hs.basis_gates_type != BasisGates::TYPE2 && hs.num_qubits >= 5) {
                this->hw_list.push_back(hs);
            }
        }
    }

    vector<shared_ptr<HybridState>> get_initial_states() override {
        vector<shared_ptr<HybridState>> result;
        auto classical_state = make_shared<ClassicalState>();

        for (int i = 0; i < 9; i++) {

            auto qs = make_shared<QuantumState>(this->qubits_used);
            qs->sparse_vector.clear();
            qs->insert_amplitude(i, complex<double>(1, 0));
            result.push_back(make_shared<HybridState>(qs, classical_state));
        }

        return result;
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        vector<Embedding> result;
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            unordered_map<int, int> embedding;
            for (auto q : qubits_used) {
                embedding.insert({q, q});
            }
            return {embedding};
        }

        for (int qubit1 = 0; qubit1 < hw.num_qubits; qubit1++) {
            for (int qubit2 = 0; qubit2 < hw.num_qubits; qubit2++) {
                for (int qubit3 = 0; qubit3 < hw.num_qubits; qubit3++) {
                    if (qubit1 == qubit2) continue;
                    if (qubit1 == qubit3) continue;
                    if (qubit2 == qubit3) continue;
                    vector<int> current_set{qubit1, qubit2, qubit3};
                    unordered_map<int, int> d_temp;

                    d_temp[q0] = current_set[0];
                    d_temp[q1] = current_set[1];
                    d_temp[q2] = current_set[2];

                    if (!is_repeated_embedding(result, d_temp)) {
                        auto all_meas_qubits = this->get_meas_qubits(hw, current_set);
                        for (auto meas_qubits : all_meas_qubits) {
                            unordered_map<int, int> d_final;
                            d_final[q0] = current_set[0];
                            d_final[q1] = current_set[1];
                            d_final[q2] = current_set[2];

                            d_final[qmeas0] = meas_qubits.first;
                            d_final[qmeas1] = meas_qubits.second;

                            // vector<int> unused = this->get_unused(d_temp);
                            // assert(unused.size() == 3);
                            // d_final[q0_copy] = unused[0];
                            // d_final[q1_copy] = unused[1];
                            // d_final[q2_copy] = unused[2];
                            result.push_back(d_final);
                        }

                    }

                }
            }
        }
        return result;
    }


    shared_ptr<ClassicalState> get_correct_cs(const shared_ptr<QVertex> &v) const {
        shared_ptr<HybridState> temp_hs = make_shared<HybridState>(*v->hybrid_state);

        for (auto i : this->MeasZ1Seq) {
            temp_hs = temp_hs->apply_instruction(i);
        }

        for (auto i : this->MeasZ2Seq) {
            temp_hs = temp_hs->apply_instruction(i);
        }

        return temp_hs->classical_state;
    }

    MyFloat get_reward(shared_ptr<QVertex> &v) const override {
        shared_ptr<ClassicalState> correct_cs = this->get_correct_cs(v);
        return MyFloat(correct_cs->read(c0) == v->classical_state()->read(c0) && correct_cs->read(c1) == v->classical_state()->read(c1));
    }

    bool guard(const shared_ptr<QVertex> &v, const shared_ptr<QAction> &a) const override{
        return v->classical_state()->read(c3) == 0;
    }


    vector<shared_ptr<QAction>> get_actions(HardwareSpecification &hardware_specification) const override {
        auto Z1Action = make_shared<QAction>(hardware_specification, this->MeasZ1Seq);

        auto Z2Action = make_shared<QAction>(hardware_specification, this->MeasZ2Seq);

        vector<shared_ptr<QAction>> result = {Z1Action, Z2Action};
        for (int i = 0; i < 2; i++) {

            GateName write_ins0;
            if (i == 0) {
                write_ins0 = GateName::Write0;
            } else {
                write_ins0 = Write1;
            }

            for (int j = 0; j < 2; j++) {
                GateName write_ins1;
                if (j == 0) {
                    write_ins1 = GateName::Write0;
                } else {
                    write_ins1 = GateName::Write1;
                }
                result.push_back(
                    make_shared<QAction>(hardware_specification,
                        vector<Instruction>({
                            Instruction(write_ins0, c0),
                            Instruction(write_ins1, c1),
                            Instruction(GateName::Write1, c3)
                            })
                        )
                );

            }

        }

        return result;
    }

public:
    ThreeQubitCode() : QuantumExperiment() {
        Instruction CX0M0 = Instruction(GateName::Cnot, vector<int>{q0}, qmeas0);
        Instruction CX1M0 = Instruction(GateName::Cnot, vector<int>{q1}, qmeas0);

        Instruction CX1M1 = Instruction(GateName::Cnot, vector<int>{q1}, qmeas1);
        Instruction CX2M1 = Instruction(GateName::Cnot, vector<int>{q2}, qmeas1);

        Instruction M0 = Instruction(GateName::Meas, qmeas0, c0);
        Instruction M1 = Instruction(GateName::Meas, qmeas1, c1);

        this->MeasZ1Seq = vector<Instruction>{CX0M0, CX1M0, M0};
        this->MeasZ2Seq = vector<Instruction>{CX1M1, CX2M1, M1};
    }
};
