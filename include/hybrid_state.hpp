#ifndef STATES_H
#define STATES_H

#include <unordered_map>
#include <complex>
#include <vector>
#include "instruction.hpp"
#define _USE_MATH_DEFINES
inline auto my_pi = M_PI;
using namespace std;

class QuantumState {
public:
    vector<int> qubits_used;
    int precision;
    unordered_map<int, complex<double>>  sparse_vector; // stores amplitudes for computational basis
    pair<complex<double>, complex<double>> get_qubit_amplitudes() const;
    shared_ptr<QuantumState> get_qubit_from_basis(const int &basis, int target) const;

    static int glue_qubit_in_basis(const int &basis, int address, int value);
    shared_ptr<QuantumState> eval_qubit_unitary(const Instruction &instruction) const;
    shared_ptr<QuantumState> eval_multiqubit_gate(const Instruction &instruction) const;
    shared_ptr<QuantumState> eval_single_qubit_gate(const Instruction &instruction) const;

    static bool are_controls_true(const int &basis, const vector<int> &controls);
    QuantumState(const vector<int> &qubits_used, int precision);
    QuantumState(const QuantumState &qs);
    complex<double> get_amplitude(const int &basis) const;
    bool is_qubit() const;
    bool is_qubit_0() const;
    bool insert_amplitude(const int &basis, const complex<double> &amplitude);
    bool add_amplitude(const int &basis, const complex<double> &amplitude);
    void normalize();
    bool operator==(const QuantumState& other) const;
    shared_ptr<QuantumState> apply_instruction(const Instruction &instruction, bool normalize=true) const;
    vector<vector<complex<double>>> multi_partial_trace(const vector<int> &remove_indices) const;
    friend std::ostream &operator<<(ostream& os, const QuantumState& quantum_state);
};

pair<shared_ptr<QuantumState>, double> get_sequence_probability(shared_ptr<QuantumState> const &quantum_state0, const vector<Instruction> &seq, int precision);
complex<double> get_inner_product(const QuantumState &qs1, const QuantumState &qs2);
double get_fidelity(const QuantumState &qs1, const QuantumState &qs2);


class ClassicalState {
    unordered_map<int, bool> sparse_vector;
    public:
        ClassicalState() = default;
        ClassicalState(const ClassicalState &cs);
        bool operator==(const ClassicalState &other) const;
        int get_memory_val() const;
        bool read(const int &address) const;
        shared_ptr<ClassicalState> write(const int &address, bool value) const;
        shared_ptr<ClassicalState> apply_instruction(const Instruction &instruction) const;
        friend std::ostream &operator<<(ostream& os, const ClassicalState& classical_state);
};

class HybridState {
    public:
        shared_ptr<QuantumState> quantum_state;
        shared_ptr<ClassicalState> classical_state;

        ~HybridState();

        HybridState(const shared_ptr<QuantumState> &quantum_state, const shared_ptr<ClassicalState> &classical_state);
        shared_ptr<HybridState> apply_instruction(const Instruction &instruction) const;
        bool operator==(const HybridState &other) const;
        friend std::ostream &operator<<(ostream& os, const HybridState& hybrid_state);
};

#endif