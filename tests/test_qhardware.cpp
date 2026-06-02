//
// Created by Stefanie Muroya Lei on 01.06.26.
//
#ifndef TEST_QHARDWARE_H
#define TEST_QHARDWARE_H

#include <gtest/gtest.h>
#include "testing_utils.cpp"
#include "hardware_specification.hpp"

// TEST(QuantumHardwareTest, NormalizedChannels) {
//     for (int i = 0; i < QuantumHardware::HardwareCount-1; i++) {
//         QuantumHardware hw = static_cast<QuantumHardware>(i);
//         HardwareSpecification hs = HardwareSpecification(hw, false, false);
//         for (auto it : hs.instructions_to_channels) {
//             assert (it.second->is_normalized());
//         }
//     }
// }
//
// TEST(QuantumHardwareTest, NormalizedChannelsOptimized) {
//     for (int i = 0; i < QuantumHardware::HardwareCount-1; i++) {
//         QuantumHardware hw = static_cast<QuantumHardware>(i);
//         HardwareSpecification hs = HardwareSpecification(hw, false, false);
//         for (auto it : hs.instructions_to_channels) {
//             assert (it.second->is_normalized());
//         }
//     }
//
// }

// ---------- Enum conversion tests ----------
TEST(QuantumHardwareTest, StringConversionRoundTrip) {
    for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
        QuantumHardware hw = static_cast<QuantumHardware>(i);
        std::string name = to_string(hw);
        QuantumHardware hw_back = to_quantum_hardware(name);
        EXPECT_EQ(hw, hw_back) << "Mismatch for hardware: " << name;
    }
}

TEST(QuantumHardwareTest, GetHardwareStringsContainsAll) {
    std::set<std::string> names = get_hardware_strings();
    EXPECT_EQ(names.size(), QuantumHardware::HardwareCount);

    for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
        QuantumHardware hw = static_cast<QuantumHardware>(i);
        EXPECT_TRUE(names.count(to_string(hw)) > 0);
    }
}

// ---------- HardwareSpecification tests (with Lima hardware) ----------
TEST(HardwareSpecificationTest, ConstructorSetsName) {
    HardwareSpecification spec(QuantumHardware::Lima, false, false);
    EXPECT_EQ(spec.get_hardware_name(), "lima");
}

TEST(HardwareSpecificationTest, ConstructorSetsNumQubits) {
    HardwareSpecification spec(QuantumHardware::Lima, false, false);
    EXPECT_EQ(spec.num_qubits, 5);
}

TEST(HardwareSpecificationTest, ConstructorSetsBasisGatesType) {
    HardwareSpecification spec(Lima, false, false);
    EXPECT_EQ(spec.basis_gates_type, BasisGates::TYPE1);
}

TEST(HardwareSpecificationTest, Digraph) {
    HardwareSpecification spec(Lima, false, false);

    for (int source = 0; source < spec.digraph.size(); source ++) {
        EXPECT_TRUE(spec.digraph.find(source) != spec.digraph.end());
    }

    map<int, set<int>> digraph;
    digraph[0] = set<int>({1});
    digraph[1] = set<int>({0, 2, 3});
    digraph[2] = set<int>({1});
    digraph[3] = set<int>({1,4});
    digraph[4] = set<int>({3});

    EXPECT_EQ(spec.digraph, digraph);
}

TEST(HardwareSpecificationTest, qubitInDegree) {
    HardwareSpecification spec(Lima, false, false);

    EXPECT_EQ(spec.get_qubit_indegree(0), 1);
    EXPECT_EQ(spec.get_qubit_indegree(1), 3);
    EXPECT_EQ(spec.get_qubit_indegree(2), 1);
    EXPECT_EQ(spec.get_qubit_indegree(3), 2);
    EXPECT_EQ(spec.get_qubit_indegree(4), 1);
}

TEST(HardwareSpecificationTest, NormalizeByEmbedding1) {
    HardwareSpecification spec(Lima, false, true);

    Embedding embedding{{{0,0}}};

    auto hs = spec.get_normalized(embedding);
    LOG.write_info_ln("instructions_to_channels: " + to_string(hs.instructions_to_channels.size()));

    for (auto e : hs.instructions_to_channels) {
        LOG.write_info_ln("instructions_to_channels: " + to_string(*e.first));
        assert(spec.instructions_to_channels.find(e.first) != spec.instructions_to_channels.end());

        if (auto b = std::dynamic_pointer_cast<QuantumChannel>(e.second)) {
            assert(*b == *std::dynamic_pointer_cast<QuantumChannel>(spec.instructions_to_channels.at(e.first)));
        }
        else if (auto c = std::dynamic_pointer_cast<MeasurementChannel>(e.second)) {
            assert(*c == *std::dynamic_pointer_cast<MeasurementChannel>(spec.instructions_to_channels.at(e.first)));
        }
    }
}

TEST(QuantumChannelTest, PrintChannel) {
    test_setup("print_channel");
    HardwareSpecification spec(QuantumHardware::Algiers, false, true);

    Embedding embedding{{{0,0}}};
    Embedding rev_embedding{{{0,0}}};

    shared_ptr<Instruction> cx_instruction = make_shared<Instruction>(GateName::X, 0);

    assert(spec.instructions_to_channels.find(cx_instruction) != spec.instructions_to_channels.end());

    shared_ptr<Instruction> renamed_instruction = make_shared<Instruction>(cx_instruction->rename(rev_embedding));


    LOG.write_info_ln("***********************");
    LOG.write_info_ln("Channel for: " + to_string(*cx_instruction));
    if (auto b = std::dynamic_pointer_cast<QuantumChannel>(spec.instructions_to_channels.at(cx_instruction))) {
        for (auto error : b->errors_to_probs) {
            LOG.write_info_ln(to_string(error.second) + ": " + to_string(error.first));
        }
    }


    // for (auto e : spec.instructions_to_channels) {
    //     LOG.write_info_ln("***********************");
    //     LOG.write_info_ln("Channel for: " + to_string(*e.first));
    //     if (auto b = std::dynamic_pointer_cast<QuantumChannel>(e.second)) {
    //         for (auto error : b->errors_to_probs) {
    //             LOG.write_info_ln(to_string(error.second) + ": " + to_string(error.first));
    //         }
    //     }
    //
    // }

    test_cleanup();
}

TEST(HardwareSpecificationTest, NormalizeByEmbedding2) {
    test_setup("normalized_embedding2");
    HardwareSpecification spec(Lima, false, true);

    Embedding embedding{{{0,1}, {1, 3}}};
    Embedding rev_embedding{{1,0}, {3,1}};

    auto hs = spec.get_normalized(embedding);
    LOG.write_info_ln("instructions_to_channels: " + to_string(hs.instructions_to_channels.size()));

    for (auto e : hs.instructions_to_channels) {
        LOG.write_info_ln("new instruction: " + to_string(*e.first));
        auto old_instruction = make_shared<Instruction>(e.first->rename(embedding));
        LOG.write_debug_ln("old instruction: " + to_string(*old_instruction));
        assert(spec.instructions_to_channels.find(old_instruction) != spec.instructions_to_channels.end());
        auto old_channel = spec.instructions_to_channels.find(old_instruction)->second->rename(rev_embedding);
        if (auto b = std::dynamic_pointer_cast<QuantumChannel>(e.second)) {
            assert(*b == *std::dynamic_pointer_cast<QuantumChannel>(old_channel));
        }
        else if (auto c = std::dynamic_pointer_cast<MeasurementChannel>(e.second)) {
            assert(*c == *std::dynamic_pointer_cast<MeasurementChannel>(old_channel));
        }
    }
    test_cleanup();
}

#endif

