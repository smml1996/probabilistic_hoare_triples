//
// Created by Stefanie Muroya Lei on 01.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "instruction.hpp"

// ---------- Construction ----------
TEST(InstructionTest, SingleQubitConstructor) {
    Instruction instr(X, 0);
    EXPECT_EQ(instr.target, 0);
    EXPECT_TRUE(instr.controls.empty());
    EXPECT_EQ(instr.gate_name, X);
    EXPECT_EQ(instr.get_instruction_type(), UnitarySingleQubit);
}

TEST(InstructionTest, SingleQubitParamConstructor) {
    vector<double> params{3.14};
    Instruction instr(Rz, 1, params);
    EXPECT_EQ(instr.target, 1);
    EXPECT_EQ(instr.params, params);
    EXPECT_EQ(instr.params.size(), 1);
    EXPECT_TRUE(instr.controls.empty());
    EXPECT_EQ(instr.get_instruction_type(), UnitarySingleQubit);
}

TEST(InstructionTest, MultiQubitConstructor) {
    vector<int> controls{0,2};
    Instruction instr(Cnot, controls, 1);
    EXPECT_EQ(instr.controls, controls);
    EXPECT_EQ(instr.target, 1);
    EXPECT_EQ(instr.get_instruction_type(), UnitaryMultiQubit);
}

TEST(InstructionTest, MultiQubitParamConstructor) {
    vector<int> controls{0,2};
    vector<double> params{0.5, 1.0};
    Instruction instr(Rzx, controls, 1, params);
    EXPECT_EQ(instr.controls, controls);
    EXPECT_EQ(instr.target, 1);
    EXPECT_EQ(instr.params, params);
    EXPECT_EQ(instr.get_instruction_type(), UnitaryMultiQubit);
}

TEST(InstructionTest, MeasurementConstructor) {
    Instruction instr(Meas, 0, 1);
    EXPECT_EQ(instr.target, 0);
    EXPECT_EQ(instr.c_target, 1);
    EXPECT_EQ(instr.get_instruction_type(), Measurement);
}

TEST(InstructionTest, ClassicalConstructor) {
    Instruction instr(Write0, 1);
    EXPECT_EQ(instr.c_target, 1);
    EXPECT_EQ(instr.get_instruction_type(), Classical);

    Instruction instr2(Write1, 100);
    EXPECT_EQ(instr2.c_target, 100);
    EXPECT_EQ(instr2.get_instruction_type(), Classical);
}

TEST(InstructionTest, ProjectorConstructor) {
    Instruction instr(P0, 1);
    EXPECT_EQ(instr.target, 1);
    EXPECT_EQ(instr.get_instruction_type(), Projector);

    Instruction instr2(P1, 0);
    EXPECT_EQ(instr2.target, 0);
    EXPECT_EQ(instr2.get_instruction_type(), Projector);
}

TEST(InstructionTest, QubitsUsed) {
    Instruction XIns(X, 0);
    vector<int> controls{0};
    Instruction CXIns(Cnot, controls, 1);

    Embedding embedding{{0,0}, {1,1}};

    assert(XIns.is_used(embedding));
    assert(CXIns.is_used(embedding));

    Instruction XIns2(X, 3);
    Instruction CXIns2(Cnot, controls, 2);
    assert(!XIns2.is_used(embedding));
    assert(!CXIns2.is_used(embedding));
}

TEST(InstructionTest, Rename) {
    vector<int> controls{0};
    Instruction CXIns(Cnot, controls, 1);
    Instruction XIns(X, 0);
    vector<Instruction> original;
    original.push_back(XIns);
    original.push_back(CXIns);
    {
        Embedding embedding{{0,0}, {1,1}};
        for (auto o : original) {
            assert(o.rename(embedding) == o);
        }
    }

    {
        Embedding embedding{{0,1}, {1,0}};
        Embedding rev_embedding{{1,0}, {0,1}};
        for (auto o : original) {
            assert(o.rename(rev_embedding).rename(embedding) == o);
        }
    }
}
