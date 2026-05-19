#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "quantum_gates.hpp"
#include "utils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum InstructionType {
    Measurement,
    Classical,
    UnitarySingleQubit,
    UnitaryMultiQubit,
    Projector
};

class Instruction {
    public:
        int target;
        int c_target; // for writes we need to specify a classical address
        vector<int> controls;
        GateName gate_name;
        vector<double> params;
        InstructionType instruction_type;
        vector<double> params_;
        vector<vector<complex<double>>> matrix;

        Instruction() = default;
        Instruction(GateName gate_name, int target);
        Instruction(int target, vector<vector<complex<double>>> matrix);
        Instruction(GateName gate_name, int target, const vector<double> &params); // for single-qubit parametric gates
        Instruction(GateName gate_name, vector<int> controls, int target); // for multiqubit gates
        Instruction(GateName gate_name, const vector<int> &controls, int target, const vector<double> &params); // for multiqubit gates with parameters
        Instruction(GateName gate_name, int target, int c_target); // for measurements
        explicit Instruction(const json &json_val);
        explicit Instruction(const json &data, int dummy);
        
        [[nodiscard]] InstructionType get_instruction_type() const;
        bool operator==(const Instruction& other) const;
        friend std::ostream &operator<<(ostream& os, const Instruction&);
        Instruction rename(const unordered_map<int, int> &embedding);
};

inline json to_json(const std::vector<std::vector<std::complex<double>>>& matrix) {
    json j_matrix = json::array();
    for (const auto& row : matrix) {
        json j_row = json::array();
        for (const auto& elem : row) {
            j_row.push_back({elem.real(), elem.imag()});  // store as [real, imag]
        }
        j_matrix.push_back(j_row);
    }
    return j_matrix;
}

inline json to_json(const Instruction &i) {
    return json{
    {"target", i.target},
    {"c_target", i.c_target},
    {"controls", i.controls},
        {"gate_name", i.gate_name},
        {"params", i.params},
        {"instruction_type", i.instruction_type},
        {"params_", i.params_},
        {"matrix", to_json(i.matrix)}
    };
}

string to_string(const Instruction &instruction);

// Custom hash
struct InstructionHash {
    std::size_t operator()(const std::shared_ptr<Instruction>& instruction) const;
};

struct InstructionPtrEqual {
    bool operator()(const std::shared_ptr<Instruction>& a, const std::shared_ptr<Instruction>& b) const;
};

bool is_identity(const vector<Instruction> &seq);

bool are_instruction_seqs_equal(const vector<Instruction> &seq1, const vector<Instruction> &seq2);

Instruction to_custom(const Instruction &instruction);
#endif
