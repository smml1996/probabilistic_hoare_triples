#ifndef QGATES_H
#define QGATES_H

#include <unordered_set>
#include <string>

using namespace std;

enum GateName {
    // this enum just helps to keep track of the basis gates of the quantum hardware
    // one-qubit ops
    I,
    X,
    Y,
    Z,
    H,
    S,
    Sd,
    Sx,
    Sxd,
    U1,
    U2,
    U3,
    T,
    Td,
    Rz,
    Ry,
    Rx,
    Reset,
    Meas,
    Custom,

    // projector
    P0,
    P1,

    // MULTI-QUBIT GATES
    Cnot,
    Ecr,
    Rzx,
    Cz,
    Ch,
    Swap,
    Toffoli,

    // classical gates
    Write0,
    Write1,

    count
};

enum BasisGates {
    TYPE1,
    TYPE2,
    TYPE3,
    TYPE4,
    TYPE5,
    TYPE6,

    BasisGatesCount
};

std::string gate_to_string(GateName gate);

unordered_set<GateName> get_value(const BasisGates& basis_gates);

BasisGates get_basis_gates_type(const unordered_set<GateName> &basis_gates);

GateName get_enum_obj(const string &raw_gate);



#endif