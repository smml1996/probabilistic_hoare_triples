//
// Created by Stefanie Muroya Lei on 05.06.26.
//

#include <gtest/gtest.h>
#include "testing_utils.cpp"

#include "hybrid_state.hpp"
#include "instruction.hpp"

TEST(HStateTest, ToggleTest) {
    Instruction instr(GateName::Toggle, 1);

    ClassicalState cs;

    cout << cs.get_memory_val() << endl;
    auto p_cs = cs.apply_instruction(instr);

    cout << p_cs->get_memory_val() << endl;

    p_cs = p_cs->apply_instruction(instr);

    cout << p_cs->get_memory_val() << endl;

}
