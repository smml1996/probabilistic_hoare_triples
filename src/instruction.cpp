#include "instruction.hpp"
#include <iostream>
#include <complex>
#include <cassert>

using namespace std;

Instruction::Instruction(GateName gate_name, int target, bool with_noise_) {
    assert (gate_name != GateName::Meas);
    assert (target > -1);
    this->c_target = -1;
    this->gate_name = gate_name;
    this->with_noise = with_noise_;
    if (gate_name == GateName::Write0 || gate_name == GateName::Write1 || gate_name == GateName::Toggle) {
        this->instruction_type = InstructionType::Classical;
        this->c_target = target;
    } else if(gate_name == GateName::P0 || gate_name == GateName::P1) {
        this->instruction_type = InstructionType::Projector;
        this->target = target;
    } else {
        this->instruction_type = InstructionType::UnitarySingleQubit;
        this->target = target;
    }
}

Instruction::Instruction(int target, vector<vector<complex<double>>> matrix_, bool with_noise_) {
    assert(matrix_.size() == matrix_[0].size()); // only square matrices
    assert(matrix_.size() == 2);
    this->target = target;
    this->c_target = -1;
    this->gate_name = GateName::Custom;
    this->instruction_type = InstructionType::UnitarySingleQubit;
    this->matrix = std::move(matrix_);
    this->with_noise = with_noise_;

    const complex<double> ZERO = complex<double>(0, 0);
    const complex<double> ONE = complex<double>(1, 0);
    const complex<double> ONEJ = complex<double>(0, 1);
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[0].size(); j++) {
            const complex<double> curr = this->matrix[i][j];
            if (is_close(curr, ZERO)) {
                this->matrix[i][j] = complex<double>(0,0);
            } else if (is_close(curr, ONE)) {
                this->matrix[i][j] = complex<double>(1,0);
            } else if (is_close(curr, ONEJ)) {
                this->matrix[i][j] = complex<double>(0,1);
            }
        }
    }
}

Instruction::Instruction(GateName gate_name, int target, const vector<double> &params, bool with_noise_) {
    this->c_target = -1;
    assert (target > -1);
    this->gate_name = gate_name;
    this->target = target;
    this->params = params;
    this->instruction_type = InstructionType::UnitarySingleQubit;
    this->c_target = -1;
    this->with_noise = with_noise_;
}

Instruction::Instruction(GateName gate_name, const vector<int> &controls, int target,
    const vector<double> &params, bool with_noise_) {
    for (auto control : controls) {
        assert (control > -1);
    }
    assert (target > -1);
    this->c_target = -1;
    this->gate_name = gate_name;
    this->controls = controls;
    this->target = target;
    this->params = params;
    this->instruction_type = InstructionType::UnitaryMultiQubit;
    this->c_target = -1;
    this->with_noise = with_noise_;
}

Instruction::Instruction(GateName gate_name, vector<int> controls, int target, bool with_noise_) {
    for (auto control : controls) {
        assert (control > -1);
    }
    assert (target > -1);
    this->gate_name = gate_name;
    this->controls = std::move(controls);
    this->target = target;
    this->instruction_type = InstructionType::UnitaryMultiQubit;
    this->c_target = -1;
    this->with_noise = with_noise_;
}

Instruction::Instruction(GateName gate_name, int target, int c_target, bool with_noise_) {
    this->gate_name = gate_name;
    this->target = target;
    this->c_target= c_target;
    this->instruction_type = InstructionType::Measurement;
    this->with_noise = with_noise_;
}

Instruction::Instruction(const json &json_val) {
    string raw_gate_name = json_val["op"];
    this->gate_name = get_enum_obj(raw_gate_name);

    this->target = json_val["target"];

    this->c_target = -1;
    
    this->instruction_type = InstructionType::UnitarySingleQubit;

    this->with_noise = json_val["with_noise"];

    
    switch (gate_name) {
        // One-qubit gates
        case I:break;
        case X: break;
        case Y:break;
        case Z:break;
        case H:break;
        case S:break;
        case Sd:break;
        case Sx:break;
        case Sxd:break;
        case U1:break;
        case U2:break;
        case U3:break;
        case V3:break;
        case T:break;
        case Td:break;
        case Rz:break;
        case Ry:break;
        case Rx:break;
        case Reset:break;
        case GateName::Meas:
            this->instruction_type = InstructionType::Measurement;
            this->c_target = target;
            break;
        case GateName::Custom:
            this->matrix = json_to_matrix(json_val["params"]);
            break;
        case GateName::Cnot:
            this->controls.push_back(json_val["control"]);
            this->instruction_type = InstructionType::UnitaryMultiQubit;
            break;
        case GateName::Ecr:
            this->controls.push_back(json_val["control"]);
            this->instruction_type = InstructionType::UnitaryMultiQubit;
            break;
        case GateName::Rzx:
            assert(false);
            break;
        case GateName::Cz:
            this->controls.push_back(json_val["control"]);
            this->instruction_type = InstructionType::UnitaryMultiQubit;
            break;
        case Ch:
            assert(false);
            break;
        case Swap:
            assert(false);
            break;
        case Toffoli:
            for (auto c : json_val["control"]) {
                this->controls.push_back(c);
            }
            this->instruction_type = InstructionType::UnitaryMultiQubit;
            break;
        // Classical gates
        case Write0:
            assert(false);
            break;
        case Write1:
            assert(false);
            break;
        default:
            std::cerr << "Coult not get instruction for " + to_string(json_val) << endl;
            break;
    }
    
}

InstructionType Instruction::get_instruction_type() const {
    return this->instruction_type;
}

bool Instruction::operator==(const Instruction& other) const {
    return target == other.target
        // && c_target == other.c_target
        && controls == other.controls
        && gate_name == other.gate_name
        && are_matrices_equal(matrix, other.matrix);
        // && params == other.params;
}

Instruction Instruction::rename(const unordered_map<int, int> &embedding) const {
    Instruction instruction;
    instruction.c_target = this->c_target;
    if (embedding.find(this->target) != embedding.end()) {
        instruction.target = embedding.at(this->target);
    } else {
        assert(false);
    }
    assert(instruction.controls.size() == 0);
    for (auto c : this->controls) {
        instruction.controls.push_back(embedding.at(c));
    }

    instruction.gate_name = this->gate_name;
    instruction.params = this->params;
    instruction.instruction_type = this->instruction_type;
    instruction.params_ = this->params_;
    instruction.matrix = this->matrix;
    instruction.with_noise = this->with_noise;
    return instruction;
}

bool Instruction::is_used(const unordered_set<int> &qubits_used) const {
    if (qubits_used.find(this->target) == qubits_used.end()) {
        return false;
    }

    for (auto c : this->controls) {
        if (qubits_used.find(c) == qubits_used.end()) {
            return false;
        }
    }

    return true;
}

string to_string(const Instruction &instruction) {
    
    switch (instruction.instruction_type) {
        case Measurement: 
            return "x" + to_string(instruction.c_target) + " := measure(q"+ to_string(instruction.target) +"); ";
        case Classical:
            if (instruction.gate_name == GateName::Write0) {
                return "x" + to_string(instruction.c_target) + " := 0; ";
            } else {
                return "x" + to_string(instruction.c_target) + " := 1; ";
            }
        default:
            assert(instruction.instruction_type == InstructionType::UnitarySingleQubit || instruction.instruction_type == InstructionType::UnitaryMultiQubit);
            string str_qvars = "";

            for (auto control : instruction.controls) {
                if (!str_qvars.empty()) {
                    str_qvars+=", ";
                }
                str_qvars +=  "q"+ to_string(control);
            }

            if (!str_qvars.empty()) {
                str_qvars +=",";
            }
            str_qvars += "q" + to_string(instruction.target);

            return gate_to_string(instruction.gate_name) + "([" + str_qvars + "]); ";

    }
}


bool InstructionPtrEqual::operator()(const std::shared_ptr<Instruction>& a, const std::shared_ptr<Instruction>& b) const {
    return *a == *b;
}


std::size_t InstructionHash::operator()(const std::shared_ptr<Instruction>&instruction) const {
    std::size_t seed = 0;

    // Helper to combine hashes (boost style)
    auto hash_combine = [](std::size_t& s, std::size_t h) {
        s ^= h + 0x9e3779b9 + (s << 6) + (s >> 2);
    };

    hash_combine(seed, std::hash<int>{}(instruction->target));
    // hash_combine(seed, std::hash<int>{}(instruction->c_target));
    hash_combine(seed, std::hash<int>{}(static_cast<int>(instruction->gate_name)));

    for (auto c : instruction->controls)
        hash_combine(seed, std::hash<int>{}(c));

    // for (auto p : instruction->params)
    //     hash_combine(seed, std::hash<double>{}(p));

    return seed;
}

bool is_identity(const vector<Instruction> &seq) {
    for (auto s : seq) {
        // if isinstance(s, KrausOperator)
        //     return False
        if (s.gate_name != GateName::I) return false;
    }
        
    return true;
}

bool are_instruction_seqs_equal(const vector<Instruction> &seq1, const vector<Instruction> &seq2) {
    if (seq1.size() != seq2.size()) return false;

    for (int i = 0; i < seq1.size(); i++) {
        if (!(seq1[i] == seq2[i])) return false;
    }
    return true;
}

Instruction to_custom(const Instruction &instruction) {
    std::vector<std::vector<std::complex<double>>> matrix{
        {std::complex<double>(0.0, 0.0), std::complex<double>(0.0, 0.0)},
        {std::complex<double>(0.0, 0.0), std::complex<double>(0.0, 0.0)}
    };

    switch (instruction.gate_name) {
        case GateName::I:
            matrix[0][0] = 1;
            matrix[1][1] = 1;
            break;
        case GateName::X:
            matrix[0][1] = 1;
            matrix[1][0] = 1;
            break;
        case GateName::Z:
            matrix[0][0] = 1;
            matrix[1][1] = -1;
            break;
        case GateName::Y:
            matrix[0][1] = complex<double>(0, -1);
            matrix[1][0] = complex<double>(0, 1);
            break;
        case GateName::Custom:
            return instruction;
        default:
            throw std::invalid_argument("Unknown gate name at to_custom: " + gate_to_string(instruction.gate_name));

    }
    assert(matrix.size() == 2);
    assert(matrix[0].size() == matrix.size());
    return {instruction.target, matrix};
}

ostream & operator<<(ostream &os, const Instruction &instruction) {
    os << to_string(instruction);
    return os;
}
