#include "quantum_gates.hpp"
#include <cassert>

using namespace std;

unordered_set<GateName> get_value(const BasisGates& basis_gates) {
    unordered_set<GateName> result;

    switch (basis_gates) {
        case BasisGates::TYPE1:
            return unordered_set<GateName>({
                GateName::Cnot, GateName::Meas, GateName::Reset, GateName::Rz, GateName::Sx, GateName::X
            });
        case BasisGates::TYPE2:
            return unordered_set({
                GateName::Cz, GateName::Meas, GateName::Reset, GateName::Rz, GateName::Sx, GateName::X
            });
        case BasisGates::TYPE3:
            return unordered_set({
                GateName::U1, GateName::Reset, GateName::U3, GateName::Meas, GateName::U2, GateName::Cnot
            });
        case BasisGates::TYPE4:
            return unordered_set({
                GateName::Reset, GateName::Meas, GateName::Rz, GateName::Sx, GateName::X
            });
        case BasisGates::TYPE5:
            return unordered_set({
                GateName::Rz, GateName::Meas, GateName::Reset, GateName::Sx, GateName::Ecr, GateName::X
            });
        case BasisGates::TYPE6:
            return unordered_set({
                GateName::Rz, GateName::Meas, GateName::Reset, GateName::Sx, GateName::Ecr, GateName::X, GateName::Cnot
            });
        default:
            assert(false);
    }
}

BasisGates get_basis_gates_type(const unordered_set<GateName> &basis_gates) {
    for(int i = 0; i < BasisGates::BasisGatesCount; i++) {
        BasisGates current_bg = static_cast<BasisGates>(i);
        auto val = get_value(current_bg);
        if (val == basis_gates) {
            return current_bg;
        }
    }

    throw std::invalid_argument("Unknown basis_gates set");
}

std::string gate_to_string(GateName gate) {
    switch (gate) {
        case I:        return "I";
        case X:        return "X";
        case Y:        return "Y";
        case Z:        return "Z";
        case H:        return "H";
        case S:        return "S";
        case Sd:       return "SD";
        case Sx:       return "SX";
        case Sxd:      return "SXD";
        case U1:       return "U1";
        case U2:       return "U2";
        case U3:       return "U3";
        case T:        return "T";
        case Td:       return "TD";
        case Rz:       return "RZ";
        case Ry:       return "RY";
        case Rx:       return "RX";
        case Reset:    return "RESET";
        case Meas:     return "MEAS";
        case Custom:   return "CUSTOM";

        case Cnot:     return "CNOT";
        case Ecr:      return "ECR";
        case Rzx:      return "RZX";
        case Cz:       return "CZ";
        case Ch:       return "CH";
        case Swap:     return "SWAP";
        case Toffoli:  return "TOFFOLI";

        case Write0:   return "WRITE0";
        case Write1:   return "WRITE1";
        case P0:       return "P0";
        case P1:       return "P1";
        default: return "UNKNOWN";
    }
}

GateName get_enum_obj(const string &raw_gate) {
    for (int i = 0; i < GateName::count; i++) {
        GateName g = static_cast<GateName>(i);
        if(gate_to_string(g) == raw_gate) {
            return g;
        }
    }
    throw std::invalid_argument("Invalid raw gate name " + raw_gate);
}