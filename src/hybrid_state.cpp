#include "hybrid_state.hpp"
#include <complex>
#include <algorithm>
#include <cassert>
#include "utils.hpp"
#include <cmath>
#include <iostream>


using namespace std;

// Quantum states
QuantumState::QuantumState(const vector<int> &qubits_used) {
    this->qubits_used = qubits_used;
    sort(this->qubits_used.begin(), this->qubits_used.end());

    // initialize to state |0>
    this->sparse_vector[0] = complex<double>(1.0, 0.0);
}

QuantumState::QuantumState(const QuantumState &qs) {
    for (auto it : qs.qubits_used) {
        this->qubits_used.push_back(it);
    }
    for (auto it : qs.sparse_vector) {
        this->sparse_vector[it.first] = it.second;
    }
}

shared_ptr<QuantumState> QuantumState::eval_qubit_unitary(const Instruction &instruction) const {
    assert (this->is_qubit());
    assert (instruction.instruction_type == InstructionType::UnitarySingleQubit || instruction.instruction_type == InstructionType::Projector);

    auto result = make_shared<QuantumState>(this->qubits_used);
    result->sparse_vector.clear();

    auto qubit_amplitudes = this->get_qubit_amplitudes();
    auto a0 = qubit_amplitudes.first;
    auto a1 = qubit_amplitudes.second;


    complex<double> half_prob = complex<double>(1/std::sqrt(2), 0.0);
    complex<double> I(0, 1);
    auto E_CONST = exp(1.0);

    if (instruction.gate_name == GateName::I){
        result->insert_amplitude(0, a0);
        result->insert_amplitude(1, a1);
    } else if( instruction.gate_name == GateName::X){
        result->insert_amplitude(0, a1);
        result->insert_amplitude(1, a0);
    } else if ( instruction.gate_name == GateName::Y){
        result->insert_amplitude(0, I*a1);
        result->insert_amplitude(1, -I*a0);
    } else if ( instruction.gate_name == GateName::Z){
        result->insert_amplitude(0, a0);
        result->insert_amplitude(1, -a1);
    } else if ( instruction.gate_name == GateName::H){
        assert((half_prob * (a0 + a1)).imag() == 0);
        assert((half_prob * (a0 - a1)).imag() == 0);
        result->insert_amplitude(0, half_prob * (a0 + a1));
        result->insert_amplitude(1, half_prob * (a0 - a1));
    } else if ( instruction.gate_name == GateName::S){
        result->insert_amplitude(0, a0);
        result->insert_amplitude(1, I*a1);
    } else if ( instruction.gate_name == GateName::Sd){
        result->insert_amplitude(0, a0);
        result->insert_amplitude(1, -I*a1);
    } else if ( instruction.gate_name == GateName::Sx){
        result->insert_amplitude(0, half_prob* (a0 - I*a1));
        result->insert_amplitude(1, half_prob*(-I*a0  + a1));
    } else if (instruction.gate_name == GateName::V3) {
        result->insert_amplitude(0,a0*(complex<double>(1)+complex<double>(2)*I)/sqrt(5));
        result->insert_amplitude(1, a1*(complex<double>(1)-complex<double>(2)*I)/sqrt(5));
    }else if ( instruction.gate_name == GateName::Sxd) {
        result->insert_amplitude(0, half_prob * (a0 + a1*I));
        result->insert_amplitude(1, half_prob * (a0*I + a1));
    } else if ( instruction.gate_name == GateName::Td){
        result->insert_amplitude(0, a0);
        // we have to calculate e^(-i*pi/4) = cos(-pi/4) + isin(-pi/4) = 1/sqrt{2} - i/sqrt{2}
        result->insert_amplitude(1, a1 * ( half_prob - (I * half_prob)));
    } else if ( instruction.gate_name == GateName::T){
        // assert not is_inverse
        result->insert_amplitude(0, a0);
        // we have to calculate e^(i*pi/4) = cos(pi/4) + isin(pi/4) = 1/sqrt{2} + i/sqrt{2}
        result->insert_amplitude(1, a1 * ( half_prob + (I * half_prob)));
    } else if ( instruction.gate_name == GateName::U3){
        assert(instruction.params.size() == 3);
        auto cos_result = cos(instruction.params[0]/2);
        auto sin_result = sin(instruction.params[0]/2);
        auto pow2 = pow(E_CONST,(I*instruction.params[2]));
        auto pow1 = pow(E_CONST,(I*instruction.params[1]));
        auto pow12 = pow(E_CONST,(I*(instruction.params[1]+instruction.params[2])));
        result->insert_amplitude(0, (a0*cos_result)-(a1*pow2*sin_result));
        result->insert_amplitude(1, a0*pow1*sin_result+a1*pow12*cos_result);
    } else if ( instruction.gate_name == GateName::Custom){
        auto temp_res = get_kraus_matrix_probability(instruction.matrix, a0, a1);
        if (is_close(temp_res.first, 0)) {return nullptr;}
        result->insert_amplitude(0, temp_res.second.first);
        result->insert_amplitude(1, temp_res.second.second);
    } else if ( instruction.gate_name == GateName::Rx){
        assert (instruction.params.size() == 1);
        auto temp_v = vector<double>({instruction.params[0], 3*my_pi/2, my_pi/2});
        Instruction u3_instruction(GateName::U3,
            instruction.target,
            temp_v);
        return this->eval_qubit_unitary(u3_instruction);
    } else if ( instruction.gate_name == GateName::Ry){
        assert (instruction.params.size() == 1);
        auto temp_v = vector<double>({instruction.params[0], 0, 0});
        Instruction u3_instruction(GateName::U3,
            instruction.target,
            temp_v);
        return this->eval_qubit_unitary(u3_instruction);
    } else if ( instruction.gate_name == GateName::Rz){
        assert (instruction.params.size() == 1);
        Instruction u1_instruction (GateName::U1, instruction.target, instruction.params);
        return this->eval_qubit_unitary(u1_instruction);
    } else if ( instruction.gate_name == GateName::U2){
        assert (instruction.params.size() == 2);
        return this->eval_qubit_unitary(Instruction(GateName::U3, instruction.target, vector<double>({my_pi/2.0, instruction.params[0], instruction.params[1]})));
    } else if ( instruction.gate_name == GateName::U1){
        // if is_inverse:
        //     raise Exception("Missing implementation of reverse of op U1")
        // assert len(params) == 1
        return this->eval_qubit_unitary(Instruction(GateName::U3, instruction.target, vector<double>({0,0,instruction.params[0]})));
    } else if ( instruction.gate_name == GateName::Reset){
        assert(a0 == 1.0 or a1 == 1.0);
        result->insert_amplitude(0, 1.0);
    } else if (instruction.instruction_type == InstructionType::Projector){
        if (instruction.gate_name == GateName::P0) {
            if (a0 == complex<double>(0, 0)) {return nullptr;}
            result->insert_amplitude(0, complex<double>(1.0, 0.0));
        } else {
            assert (instruction.gate_name == GateName::P1);
            if (a1 == complex<double>(0,0)) {return nullptr;}
            result->insert_amplitude(1, complex<double>(1, 0.0));
        }
    } else {
        throw invalid_argument("Not implemented (eval gate): " + to_string(instruction.gate_name));
    }
    return result;
}


complex<double> QuantumState::get_amplitude(const int &basis) const {
    auto it = this->sparse_vector.find(basis);
    if ( it != this->sparse_vector.end()) {
        return it->second;
    }

    return {0.0, 0.0};
}

bool QuantumState::is_qubit() const {
    if (this->sparse_vector.size() > 2) {
        return false;
    }

    for (auto it : this->sparse_vector) {
        if (it.first > 1) {
            return false;
        }
    }

    return true;
}

bool QuantumState::is_qubit_0() const {
    assert(this->is_qubit());

    complex<double> amplitude0 = this->get_amplitude(0);
    return is_close(norm(amplitude0), 1.0);
}

pair<complex<double>, complex<double>> QuantumState::get_qubit_amplitudes() const {
    return make_pair(this->get_amplitude(0), this->get_amplitude(1));
}

bool QuantumState::insert_amplitude(const int &basis, const complex<double> &amplitude) {
    if (is_close(amplitude.real(), 0.0) && is_close(amplitude.imag(), 0.0)) {
        return false;
    }
    double temp_real = 0.0;
    double temp_imag = 0.0;
    if (!is_close(amplitude.real(), 0.0)) {
        temp_real = amplitude.real();
    }

    if (!is_close(amplitude.imag(), 0.0)) {
        temp_imag = amplitude.imag();
    }

    this->sparse_vector[basis] = complex<double>(temp_real, temp_imag);
    return true;
}

bool QuantumState::add_amplitude(const int &basis, const complex<double> &amplitude) {

    if (is_close(amplitude.real(), 0.0) && is_close(amplitude.imag(), 0.0)){
        // if both the real and the imaginary part of the amplitude are 0 then we return False because we do not add any amplitude
        return false;
    }

    complex<double> prev_amplitude = this->get_amplitude(basis);
    complex<double>current_amplitude = prev_amplitude + amplitude;
    if (is_close(current_amplitude.real(), 0.0) and is_close(current_amplitude.imag(), 0.0)){
        // if the new amplitude is 0 in both the real and imaginary part we delete this key
        this->sparse_vector.erase(basis);
        return false;
    }
    double temp_real = 0.0;
    double temp_imag = 0.0;
    if (!is_close(current_amplitude.real(), 0.0)) {
        temp_real = current_amplitude.real();
    }

    if (!is_close(current_amplitude.imag(), 0.0)) {
        temp_imag = current_amplitude.imag();
    }
    this->sparse_vector[basis] = complex<double>(temp_real, temp_imag);
    return true;
}

void QuantumState::normalize() {
    double sum_ = 0;
    for (auto it : this->sparse_vector) {
        sum_ += norm(it.second);
    }

    double norm = sqrt(sum_);

    for (auto it : this->sparse_vector) {
        this->sparse_vector[it.first] = it.second/norm;
    }
}

bool QuantumState::operator==(const QuantumState& other) const {
    if (this->sparse_vector.size() != other.sparse_vector.size()) return false;

    // here we check for global phases: two states are equal if they only differ by a global factor
    double inner_product = get_fidelity(*this, other);
    assert(inner_product >= 0);
    return is_close(inner_product, 1);
}

string to_string(const QuantumState &quantum_state) {
    string result;
    bool is_first = true;
    for (auto it : quantum_state.sparse_vector) {
        if (!is_first) {
            result+=" + ";
        }
        result += "("+ to_string(it.second.real()) + ","+ to_string(it.second.imag()) + "i)|" + to_binary(it.first) + ">";
        is_first = false;
    }

    return result;
}

pair<shared_ptr<QuantumState>, MyFloat> get_sequence_probability(shared_ptr<QuantumState> const &quantum_state0, const vector<Instruction> &seq) {
    assert(quantum_state0 != nullptr);
    int count_meas = 0;
    auto quantum_state = quantum_state0;
    int index = 0;
    for (const auto& instruction : seq) {
        assert(instruction.gate_name != GateName::Meas);
        if (instruction.gate_name== GateName::P0 or instruction.gate_name== GateName::P1){
            count_meas += 1;
            if (count_meas > 1) throw invalid_argument("Invalid Measurement instruction");
        }
        quantum_state = quantum_state->apply_instruction(instruction, false);

        index++;
        if (quantum_state == nullptr) {
            return make_pair(nullptr, 0.0);
        }
    }

    auto prob = get_inner_product(*quantum_state, *quantum_state);

    assert(is_close(prob.imag(), 0.0));
    // round down was here
    auto prob_answer = round_to(prob.real());
    quantum_state->normalize();
    return make_pair(quantum_state, prob_answer);
}

shared_ptr<QuantumState> QuantumState::apply_instruction(const Instruction &instruction, bool normalize) const {
    assert(!this->sparse_vector.empty());
    shared_ptr<QuantumState> result;
    if (instruction.instruction_type == InstructionType::UnitaryMultiQubit){
        if( instruction.gate_name == GateName::Swap) {
            assert(instruction.controls.size() == 1);
            auto index1 = instruction.target;
            auto index2 = instruction.controls[0];

            auto result0 = this->apply_instruction(Instruction(GateName::Cnot, vector<int>({index1}), index2), normalize=false);
            auto result1 = result0->apply_instruction(Instruction(GateName::Cnot, vector<int>({index2}), index1), normalize=false);
            result = result1->apply_instruction(Instruction(GateName::Cnot, vector<int>({index1}), index2), normalize=false);
        } else if (instruction.gate_name == GateName::Rzx){
            assert (instruction.params.size() == 1);
            auto angle = instruction.params[0];
            auto H1 = Instruction(GateName::H, instruction.target);
            auto CX01 = Instruction(GateName::Cnot, instruction.controls, instruction.target);
            auto RZ1 = Instruction(GateName::Rz, instruction.target, vector<double>({angle}));
            auto result0 = this->apply_instruction(H1, normalize=false);
            auto result1 = result0->apply_instruction(CX01, normalize=false);
            auto result2 = result1->apply_instruction(RZ1, normalize=false);
            auto result3 = result2->apply_instruction(CX01, normalize=false);
            result = result3->apply_instruction(H1, normalize=false);
        } else if (instruction.gate_name == GateName::Ecr) {
            assert (instruction.controls.size() == 1);
            auto angle = my_pi / 4;
            auto RZX = Instruction(GateName::Rzx,  instruction.controls, instruction.target, vector<double>({angle}));
            auto RZX_dagger = Instruction(GateName::Rzx,  instruction.controls, instruction.target, vector<double>({-angle}));
            auto X0 = Instruction(GateName::X, instruction.controls[0]);
            auto result0 = this->apply_instruction(RZX, normalize=false);
            auto result1 = result0->apply_instruction(X0, normalize=false);
            result = result1->apply_instruction(RZX_dagger, normalize=false);
        } else {
            assert (instruction.gate_name == GateName::Cnot || instruction.gate_name == GateName::Cz);
            result = this->eval_multiqubit_gate(instruction);
        }
    } else {
        if (instruction.gate_name == GateName::Reset){
            auto meas_instruction = Instruction(GateName::Meas, instruction.target, instruction.target);
            shared_ptr<QuantumState> quantum_state = make_shared<QuantumState>(this->qubits_used);
            auto pair0 = get_sequence_probability(quantum_state, vector<Instruction>({Instruction(GateName::P0, instruction.target)}));
            auto q0 = pair0.first;
            auto prob0 = pair0.second;

            auto pair1 = get_sequence_probability(quantum_state, vector<Instruction>({Instruction(GateName::P1, instruction.target)}));
            auto q1 = pair1.first;
            auto prob1 = pair1.second;
            auto result0 = weighted_choice(vector<shared_ptr<QuantumState>>({q0, q1}), vector<double>({prob0.value, prob1.value}));
            if (q1 != nullptr && *result0 == *q1) {
                auto x_instruction = Instruction(GateName::X, instruction.target);
                result = result0->apply_instruction(x_instruction);
            } else {
                result = result0;
            }
        } else {
            result = this->eval_single_qubit_gate(instruction);

        }
    }
    if (result){
        if (normalize) {
            result->normalize();
            assert(is_close(get_fidelity(*result, *result), 1));
        }
        assert(!result->sparse_vector.empty());
    }

    return result;
}

shared_ptr<QuantumState> QuantumState::get_qubit_from_basis(const int &basis, int target) {
    auto result = make_shared<QuantumState>(vector<int>({target}));
    result->sparse_vector.clear();
    std::string bin_number = to_binary(basis);  // reversed already
    if (target >= static_cast<int>(bin_number.size())) {
        result->insert_amplitude(0, complex<double>(1.0, 0.0));
    } else {
        int value = bin_number[target] - '0';  // char -> int
        result->insert_amplitude(value, complex(1.0, 0.0));
    }
    return result;
}

int QuantumState::glue_qubit_in_basis(const int &basis, int address, int value) {
    // convert basis to binary string (reversed = least significant bit first)
    std::string bin_number;
    int temp = basis;
    if (temp == 0) bin_number.push_back('0');
    while (temp > 0) {
        int mod = temp % 2;
        bin_number.push_back(mod + '0');
        temp /= 2;
    }

    // append zeros until length > address
    while (static_cast<int>(bin_number.size()) <= address) {
        bin_number.push_back('0');
    }

    // set qubit at address
    bin_number[address] = static_cast<char>('0' + value);

    // compute integer result (reverse order back)
    int result = 0;
    for (auto it = bin_number.rbegin(); it != bin_number.rend(); ++it) {
        result = (result << 1) + (*it - '0');
    }
    return result;
}

shared_ptr<QuantumState> QuantumState::eval_single_qubit_gate(const Instruction &instruction) const {
    auto result = make_shared<QuantumState>(this->qubits_used);
    result->sparse_vector.clear();
    bool at_least_one_perform_op = false;
    auto op = instruction.gate_name;
    int address = instruction.target;
    for (auto it : this->sparse_vector) {
        auto basis = it.first;
        auto value = it.second;
        auto old_qubit = this->get_qubit_from_basis(basis, address);
        bool should_perform_op = true;
        if (instruction.instruction_type == InstructionType::Projector) {
            if (op == GateName::P0 && (!old_qubit->is_qubit_0())) {
                // we cannot apply a projection to \0> if the qubit is 1 already
                should_perform_op = false;
            } else if ( op == GateName::P1 && old_qubit->is_qubit_0()) {
                should_perform_op = false;
            }
        }

        if (should_perform_op) {
            auto new_qubit = old_qubit->eval_qubit_unitary(instruction);
            if (new_qubit != nullptr){
                auto a = new_qubit->get_qubit_amplitudes();
                auto a0 = a.first;
                auto a1 = a.second;
                a0 *= value;
                a1 *= value;
                auto basis0 = QuantumState::glue_qubit_in_basis(basis, address, 0);
                auto basis1 = QuantumState::glue_qubit_in_basis(basis, address, 1);
                result->add_amplitude(basis0, a0);
                result->add_amplitude(basis1, a1);
                at_least_one_perform_op = true;
            }
        }
    }
    if (not at_least_one_perform_op) {return nullptr;}
    if (result->sparse_vector.empty()){return nullptr;}
    return result;
}

bool QuantumState::are_controls_true(const int &basis, const vector<int> &controls) {
    for (int a : controls) {
        // if the bit is at position an is 0
        if (((basis >> a) & 1) == 0) {
            return false;
        }
    }
    return true;
}

shared_ptr<QuantumState> QuantumState::eval_multiqubit_gate(const Instruction &instruction) const {
    auto op = instruction.gate_name;
    auto controls = instruction.controls;
    auto address = instruction.target;
    auto result = make_shared<QuantumState>(this->qubits_used);
    result->sparse_vector.clear();

    for (auto it : this->sparse_vector) {
        auto basis = it.first;
        auto value = it.second;
        if (QuantumState::are_controls_true(basis, controls)) {
            auto basis_state = new QuantumState(this->qubits_used);
            basis_state->sparse_vector.clear();
            basis_state->insert_amplitude(basis, value);
            Instruction new_instruction;
            if (op == GateName::Cnot || op == GateName::Toffoli) {
                new_instruction = Instruction(GateName::X, address);
            }else if (op == GateName::Ch){
                new_instruction = Instruction(GateName::H, address);
            // } else if( op == GateName::Cu3) {
            //     new_instruction = Instruction(GateName::U3, address, None, gate_data.params)
            } else if (op == GateName::Cz) {
                new_instruction = Instruction(GateName::Z, address);
            } else {
                throw invalid_argument("Multiqubit gate name not defined " + to_string(op));
            }

            auto written_basis = basis_state->eval_single_qubit_gate(new_instruction);
            if (written_basis != nullptr) {
                for (auto it2 : written_basis->sparse_vector) {
                    auto b = it2.first;
                    auto v = it2.second;
                    result->add_amplitude(b, v);
                }
            } else {
                throw invalid_argument("written basis is none");
            }

        } else {
            result->add_amplitude(basis, value);
        }
    }

    assert (!result->sparse_vector.empty());
    return result;
}

int  _get_real_index(const vector<int> &qubits_used,const int&index) {
    for(int index_ = 0; index_ < qubits_used.size(); index_++) {
        auto q = qubits_used[index_];
        if (q == index) {
            return index_;
        }
    }

    throw invalid_argument("Could not get real index");
}

string int_to_bin(int n, int zero_padding=-1) {
    assert(n >= 0);
    string result;
    while (n > 0) {
        if ((n % 2) == 1) result += "1";
        else
            result += "0";
        n = n >> 1;
    }

    if (zero_padding > -1) {
        while (result.size() < zero_padding)
            result += "0";
    }
    if (result.empty())
        return "0";
    return result;
}

string remove_unused(const string &bin_string, const vector<int> &used_qubits, int padding) {
    string answer;
    for (int index = 0; index < bin_string.size(); index++) {
        char c = bin_string.at(index);
        if (std::find(used_qubits.begin(), used_qubits.end(), index) == used_qubits.end()) {
            assert (c == '0');
        } else {
            answer += c;
        }
    }

    if (padding > -1) {
        while (answer.size() < padding) answer += "0";
    }

    return answer;
}

string remove_char_at_indices(const string &s, const vector<int> &remove_indices) {
    string answer;
    for (int index = 0; index < s.size(); index++) {
        char c = s.at(index);
        if (std::find(remove_indices.begin(), remove_indices.end(), index) == remove_indices.end())
            answer += c;
    }

    return answer;
}

int bin_to_int(const string &bin) {
    int result = 0;
    for (int power = 0; power < bin.size(); power++) {
        char b = bin.at(power);
        assert (b=='0' || b == '1');
        if (b == '1') {
            result += pow(2, power);
        }
    }

    return result;
}

bool are_all_indices_equal(const string &s1, const string &s2, const vector<int> &indices) {
    for (auto index : indices) {
        if (s1.at(index) != s2.at(index))
            return false;
    }
    return true;
}

vector<vector<complex<double>>> QuantumState::multi_partial_trace(const vector<int> &remove_indices) const {
    vector<vector<complex<double>>> result;

    vector<int> local_qubits_used;
    for (auto q : this->qubits_used) {
        local_qubits_used.push_back(q);
    }


    auto final_dim = pow(2 ,(local_qubits_used.size() - remove_indices.size()));

    vector<int> real_indices;

    for (int index : remove_indices) {
        real_indices.push_back(_get_real_index(local_qubits_used, index));
    }

    for (int i = 0; i < final_dim; i++) {
        vector<complex<double>> temp;
        temp.reserve(final_dim);
        for (int j = 0; j < final_dim; j++) {
            temp.emplace_back(0);
        }
        result.push_back(temp);
    }
    for (auto it : this->sparse_vector) {
        int ket = it.first;

        auto bin_ket = int_to_bin(ket, local_qubits_used.size());
        auto bin_ket_ = remove_unused(bin_ket, local_qubits_used, local_qubits_used.size());
        auto bin_new_ket = remove_char_at_indices(bin_ket_, real_indices);
        auto index_new_ket = bin_to_int(bin_new_ket); // index of the row in the result(-ing density matrix)
        for (auto it2 : this->sparse_vector) {
            int bra = it2.first;
            auto current_val = real(this->get_amplitude(ket) * conj(this->get_amplitude(bra)));
            auto bin_bra = int_to_bin(bra, local_qubits_used.size()); // original bra
            auto bin_bra_ = remove_unused(bin_bra, local_qubits_used, local_qubits_used.size());
            auto bin_new_bra = remove_char_at_indices(bin_bra_, real_indices);
            auto index_new_bra = bin_to_int(bin_new_bra);
            if (are_all_indices_equal(bin_ket_, bin_bra_, real_indices))
                result[index_new_ket][index_new_bra] += current_val;
        }
    }
    return result;
}

complex<double> get_inner_product(const QuantumState &qs1, const QuantumState &qs2) {
    complex<double> inner_product(0,0);

    for (const auto &it : qs1.sparse_vector) {
        const auto &val1 = it.second;
        const auto &val2 = qs2.get_amplitude(it.first);
        inner_product += val1 * std::conj(val2);

    }
    return inner_product;
}

double get_fidelity(const QuantumState &qs1, const QuantumState &qs2) {
    auto inner_product = get_inner_product(qs1, qs2);
    return norm(inner_product);
}

ClassicalState::ClassicalState(const ClassicalState & cs) {
    for (auto it : cs.sparse_vector) {
        this->sparse_vector[it.first] = it.second;
    }
}
bool ClassicalState::operator==(const ClassicalState&other) const {
    return this->get_memory_val() == other.get_memory_val();
}

int ClassicalState::get_memory_val() const {
    int answer = 0;

    for(auto it : this->sparse_vector) {
        answer += pow(2, it.first) * it.second;
    }

    return answer;
}
bool ClassicalState::read(const int &address) const {
    auto it = this->sparse_vector.find(address);

    if (it != this->sparse_vector.end()) {
        return it->second;
    }
    return false;
}

shared_ptr<ClassicalState> ClassicalState::write(const int &address, bool value) const {
    shared_ptr<ClassicalState> classical_state = make_shared<ClassicalState>(*this);
    classical_state->sparse_vector[address] = value;
    return classical_state;
}

shared_ptr<ClassicalState> ClassicalState::apply_instruction(const Instruction &instruction) const {
    assert(instruction.instruction_type == InstructionType::Classical);
    assert(instruction.c_target > -1);
    
    if (instruction.gate_name == GateName::Write0) {
        return this->write(instruction.c_target, false);
    } else if (instruction.gate_name == GateName::Write1){
        return this->write(instruction.c_target, true);
    } else {
        assert(instruction.gate_name == GateName::Toggle);
        auto current_val = this->read(instruction.c_target);
        return this->write(instruction.c_target, !current_val);
    }
}

string to_string(const ClassicalState &classical_state) {
    return to_string(classical_state.get_memory_val());
}

HybridState::~HybridState() = default;

HybridState::HybridState(const shared_ptr<QuantumState> &quantum_state,
    const shared_ptr<ClassicalState> &classical_state,
    const int &hidden_index) {
    this->quantum_state = quantum_state;
    this->classical_state = classical_state;
    this->hidden_index = hidden_index;
}

shared_ptr<HybridState> HybridState::apply_instruction(const Instruction &instruction) const
{
    auto new_qs = this->quantum_state;
    auto new_cs = this->classical_state;

    if (instruction.instruction_type == InstructionType::Classical) {
        new_cs = new_cs->apply_instruction(instruction);
    } else{
        new_qs = new_qs->apply_instruction(instruction);
    }

    return make_shared<HybridState>(new_qs, new_cs, this->hidden_index);
}

bool HybridState::operator==(const HybridState &other) const {
    assert (other.classical_state != nullptr);
    assert (other.quantum_state!= nullptr);
    assert (this->quantum_state!= nullptr);
    assert (this->classical_state!= nullptr);
    return (*this->quantum_state  == *other.quantum_state)  && (*this->classical_state  == *other.classical_state) && (this->hidden_index == other.hidden_index);
}

string to_string(const HybridState &hybrid_state) {
    return to_string(*hybrid_state.quantum_state) + ", " + to_string(*hybrid_state.classical_state) + ", " + to_string(hybrid_state.hidden_index);
}

int QEnsemble::is_new_value(const shared_ptr<const HybridState> &hs) const {

    for (int i = 0; i < this->values.size(); i++) {
        if (*this->values[i].first == *hs) {
            return i;
        }
    }
    return -1;
}

void QEnsemble::add(const shared_ptr<const HybridState> &hs, const MyFloat &prob) {
    if (prob > zero) {
        auto index = this->is_new_value(hs);

        if (index < this->values.size()) {
            this->values[index].second += prob;
        } else {
            this->values.emplace_back(hs, prob);
        }
        return;
    }
    assert (prob == zero);
}

void QEnsemble::insert_new(const shared_ptr<const HybridState> &hs, const MyFloat &prob) {
    if (this->is_new_value(hs)) {
        if (prob > zero) {
            this->values.emplace_back(hs, prob);
        }
        return;
    }

    assert(false);
}

void QEnsemble::normalize() {
    MyFloat total_sum;
    for (const auto& e : this->values) {
        total_sum += e.second;
    }

    for (auto & value : this->values) {
        value.second /= total_sum;
    }

    this->check();
}

void QEnsemble::check() const {
    MyFloat total_sum;
    for (const auto& e : this->values) {
        total_sum += e.second;
    }

    if (Config::is_debug) {
        assert(one.value == 1);
    }
    assert(total_sum == one);
}

bool QEnsemble::empty() const {
    return this->values.empty();
}

void QEnsemble::print() const {
    for (const auto &[hs, prob] : this->values) {
        cout << *hs << ": " << prob << endl;
    }
}

std::ostream &operator<<(ostream& os, const QuantumState& quantum_state) {
    bool is_first = true;
    for (auto it : quantum_state.sparse_vector) {
        if (!is_first) {
            os <<" + ";
        }
        os << it.second << "|" << to_binary(it.first) << ">";
        is_first = false;
    }
    return os;
}

std::ostream &operator<<(ostream& os, const ClassicalState& classical_state) {
    os << classical_state.get_memory_val();
    return os;
}

std::ostream &operator<<(ostream& os, const HybridState& hybrid_state) {
    os << *hybrid_state.quantum_state << " --------- " << *hybrid_state.classical_state;
    return os;
}