//
// Created by Stefanie Muroya Lei on 31.05.26.
//

// Only one classical action: determine Done

#ifndef RUS_EXPR_H
#define RUS_EXPR_H

#include "experiments.hpp"

class RUS : public QuantumExperiment {
    // https://arxiv.org/pdf/1311.1074
    // We seek to implement gate V3 = (I + 2iZ)/sqrt(5) on qubit q0
    const int c0 = 0;

    // there are 5 stages (subprocedures to synthesize) for which we need 3 bits
    const int b0 = 1;
    const int b1 = 2;
    const int b2 = 3;
    const int b3 = 4;
    const int b4 = 5;
    vector<int> stages_bits{b0, b1, b2, b3, b4};

    // stages
    const int STAGE_AncU0 = 0;
    const int STAGE_QU0 = 1;
    const int STAGE_CX = 2;
    const int STAGE_AncU1 = 3;
    const int STAGE_QU1 = 4;
    const int STAGE_FINAL = 5;

    unordered_map<int, unordered_set<int>> stage_to_actions;


    // we are using Choi states (hence we use "copies")
    const int q0 = 0;
    const int ancilla = 1;
    const int q0_copy = 2;

    vector<pair<int, int>> choi_pairs{{q0, q0_copy}};

    // target channel
    ComplexMatrix V3_pt;

    int get_current_stage(const shared_ptr<const QVertex> &v) const {
        int result = 0;
        for (int i = 0; i < this->stages_bits.size(); i++) {
            result += v->classical_state()->read(this->stages_bits[i]) * (1 << i);
        }
        return result;
    }

    vector<Instruction> get_stage_write_sequence(int s) const {
        vector<Instruction> result;
        for (int i = 0; i < this->stages_bits.size(); i++) {
            int curr_bit_val = s % 2;
            if (curr_bit_val == 1) {
                result.push_back(Instruction(GateName::Write1, stages_bits[i]));
            } else {
                result.push_back(Instruction(GateName::Write0, stages_bits[i]));
            }
            s /= 2;
        }

        return result;
    }
protected:
    vector<shared_ptr<const QAction>> get_actions_(HardwareSpecification &hardware_specification) override {
        vector<shared_ptr<const QAction>> result;
        {
            // ancilla
            auto H = Instruction(GateName::H, ancilla);
            auto HAction = make_shared<QAction>(hardware_specification, vector<Instruction>{H});
            result.push_back(HAction);
            stage_to_actions[STAGE_AncU0].insert(HAction->id);
            stage_to_actions[STAGE_AncU1].insert(HAction->id);

            auto X = Instruction(GateName::X, ancilla);
            auto XAction = make_shared<QAction>(hardware_specification, vector<Instruction>{X});
            result.push_back(XAction);
            stage_to_actions[STAGE_AncU0].insert(XAction->id);
            stage_to_actions[STAGE_AncU1].insert(XAction->id);

            auto Z = Instruction(GateName::Z, ancilla);
            auto ZAction = make_shared<QAction>(hardware_specification, vector<Instruction>{Z});
            result.push_back(ZAction);
            stage_to_actions[STAGE_AncU0].insert(ZAction->id);
            stage_to_actions[STAGE_AncU1].insert(ZAction->id);

            auto T = Instruction(GateName::T, ancilla);
            auto TAction = make_shared<QAction>(hardware_specification, vector<Instruction>{T});
            result.push_back(TAction);
            stage_to_actions[STAGE_AncU0].insert(TAction->id);
            stage_to_actions[STAGE_AncU1].insert(TAction->id);

            auto TD = Instruction(GateName::Td, ancilla);
            auto TDAction = make_shared<QAction>(hardware_specification, vector<Instruction>{TD});
            result.push_back(TDAction);
            stage_to_actions[STAGE_AncU0].insert(TDAction->id);
            stage_to_actions[STAGE_AncU1].insert(TDAction->id);

            auto NextStageAction = make_shared<QAction>(hardware_specification, get_stage_write_sequence(STAGE_QU0));
            result.push_back(NextStageAction);
            stage_to_actions[STAGE_AncU0].insert(NextStageAction->id);

            NextStageAction = make_shared<QAction>(hardware_specification, get_stage_write_sequence(STAGE_QU1));
            result.push_back(NextStageAction);
            stage_to_actions[STAGE_AncU1].insert(NextStageAction->id);
        }

        {
            // q0
            auto H = Instruction(GateName::H, q0);
            auto HAction = make_shared<QAction>(hardware_specification, vector<Instruction>{H});
            result.push_back(HAction);
            stage_to_actions[STAGE_QU0].insert(HAction->id);
            stage_to_actions[STAGE_QU1].insert(HAction->id);

            auto X = Instruction(GateName::X, q0);
            auto XAction = make_shared<QAction>(hardware_specification, vector<Instruction>{X});
            result.push_back(XAction);
            stage_to_actions[STAGE_QU0].insert(XAction->id);
            stage_to_actions[STAGE_QU1].insert(XAction->id);
            stage_to_actions[STAGE_FINAL].insert(XAction->id);

            auto Z = Instruction(GateName::Z, q0);
            auto ZAction = make_shared<QAction>(hardware_specification, vector<Instruction>{Z});
            result.push_back(ZAction);
            stage_to_actions[STAGE_QU0].insert(ZAction->id);
            stage_to_actions[STAGE_QU1].insert(ZAction->id);

            auto NextStageAction = make_shared<QAction>(hardware_specification, get_stage_write_sequence(STAGE_CX));
            result.push_back(NextStageAction);
            stage_to_actions[STAGE_QU0].insert(NextStageAction->id);

            NextStageAction = make_shared<QAction>(hardware_specification, get_stage_write_sequence(STAGE_FINAL));
            result.push_back(NextStageAction);
            stage_to_actions[STAGE_QU1].insert(NextStageAction->id);
        }

        {
            // CX stage
            if (hardware_specification.digraph[q0].find(ancilla) != hardware_specification.digraph[q0].end()) {
                auto CX = Instruction(GateName::Cnot, vector<int>{q0}, ancilla);
                auto CXAction = make_shared<QAction>(hardware_specification, vector<Instruction>{CX});
                result.push_back(CXAction);
                stage_to_actions[STAGE_CX].insert(CXAction->id);
            }

            if (hardware_specification.digraph[ancilla].find(q0) != hardware_specification.digraph[ancilla].end()) {
                auto CX = Instruction(GateName::Cnot, vector<int>{ancilla}, q0);
                auto CXAction = make_shared<QAction>(hardware_specification, vector<Instruction>{CX});
                result.push_back(CXAction);
                stage_to_actions[STAGE_CX].insert(CXAction->id);
            }


            auto NextStageAction = make_shared<QAction>(hardware_specification, get_stage_write_sequence(STAGE_AncU1));
            result.push_back(NextStageAction);
            stage_to_actions[STAGE_CX].insert(NextStageAction->id);
        }


        {
            // final stage
            auto Toggle = Instruction(GateName::Toggle, c0);
            auto ToggleAction = make_shared<QAction>(hardware_specification, vector<Instruction>{Toggle});
            result.push_back(ToggleAction);
            stage_to_actions[STAGE_FINAL].insert(ToggleAction->id);


            auto Meas = Instruction(GateName::Meas, q0, c0);
            auto MeasAction = make_shared<QAction>(hardware_specification, vector<Instruction>{Meas});
            result.push_back(MeasAction);
            stage_to_actions[STAGE_FINAL].insert(MeasAction->id);

        }

        return result;
    }
public:
    void set_experiment_name() override {
        this->name = "rus_v3";
    }

    void set_horizons() override {
        this->min_horizon = 2;
        this->max_horizon = 8;
    }

    void set_qubits_used() override {
        this->qubits_used.push_back(q0);
        this->qubits_used.push_back(ancilla);
        this->qubits_used.push_back(q0_copy);
    }

    vector<shared_ptr<const HybridState>> get_initial_states() override {
        // the ancilla is faulty might be flipped.
        // In the original algorithm the ancillae are initialized to |+>
        vector<shared_ptr<const HybridState>> result;
        auto classical_state = make_shared<ClassicalState>();

        auto H = Instruction(GateName::H, ancilla);


        // prepare first bell state
        auto state0 = make_shared<QuantumState>(this->qubits_used);
        state0 = state0->apply_instruction(H);
        result.emplace_back(new HybridState(state0, classical_state, 0));

        // prepare second bell state
        auto Z = Instruction(GateName::Z, ancilla);
        auto state1 = state0->apply_instruction(Z);
        result.emplace_back(make_shared<HybridState>(state1, classical_state, 0));
        return result;
    }

    MyFloat get_reward(shared_ptr<const QVertex> &v) const override {
        if (v->hybrid_state->classical_state->read(c0)) {
            auto current_pt = v->quantum_state()->multi_partial_trace({ancilla});
            return MyFloat(are_matrices_equal(current_pt, this->V3_pt));
        }
        return MyFloat(0);
    }

    bool guard(const shared_ptr<const QVertex> &v, const shared_ptr<const QAction> &a) const override {
        int current_stage = this->get_current_stage(v);
        return this->stage_to_actions.find(current_stage)->second.find(a->id) != this->stage_to_actions.find(current_stage)->second.end();
    }

    vector<Embedding> get_embeddings(const HardwareSpecification &hw) const override {
        if (hw.get_hardware() == QuantumHardware::PerfectHardware) {
            return {Embedding{{q0, 0}, {ancilla, 1}, {q0_copy, 2}}};
        }
        vector<Embedding> result;
        for (int c_qubit = 0; c_qubit < hw.num_qubits; c_qubit++) {
            for (auto t_qubit : hw.digraph.at(c_qubit)) {
                assert(c_qubit != t_qubit);
                auto unused_qubits = this->get_unused(unordered_set<int>{c_qubit, t_qubit}, 1);
                result.push_back(Embedding{{q0, c_qubit}, {ancilla, t_qubit}, {q0_copy, unused_qubits[0]}});
            }
        }

        return result;
    }

public:
    RUS() : QuantumExperiment() {
        auto temp = this->get_choi_id_state(choi_pairs);
        temp = temp->apply_instruction(Instruction(GateName::V3, q0));
        this->V3_pt = temp->multi_partial_trace({ancilla});
    }
};

#endif