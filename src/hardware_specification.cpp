#include "hardware_specification.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "channels.hpp"
#define _USE_MATH_DEFINES
auto pi = M_PI;

using namespace std;
using json = nlohmann::json;

MeasurementChannel PERFECT_MEAS_CHANNEL = MeasurementChannel(1.0, 1.0);

int HardwareSpecification::get_qubit_indegree(int qubit) const {
    if (this->qubit_to_indegree.find(qubit) != this->qubit_to_indegree.end()) {
        return this->qubit_to_indegree.at(qubit);
    }
    return 0;
}

int HardwareSpecification::get_qubit_outdegree(int qubit) const {
    if (this->qubit_to_outdegree.find(qubit) != this->qubit_to_outdegree.end()) {
        return this->qubit_to_outdegree.at(qubit);
    }
    return 0;
}

vector<pair<int, double>> HardwareSpecification::get_sorted_qubit_couplers(int target) const {
    assert (target >= 0);
    vector<pair<int, double>> result;

    for (auto it : this->instructions_to_channels) {
        auto instruction = it.first;
        auto channel = it.second;

        if(instruction->gate_name == GateName::Cnot || instruction->gate_name == GateName::Cz) {
            assert (instruction->controls.size() == 1);
            if (target == instruction->target) {
                result.emplace_back(instruction->controls[0], channel->estimated_success_prob);
            }
        }
    }

    sort(result.begin(), result.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first > b.first;
    });
    return result;
}

vector<pair<pair<int,int>, double>> HardwareSpecification::get_sorted_qubit_couplers() const {
    vector<pair<pair<int, int>, double>> result;

    for (auto it : this->instructions_to_channels) {
        auto instruction = it.first;
        auto channel = it.second;

        if(instruction->gate_name == GateName::Cnot || instruction->gate_name == GateName::Cz) {
            assert (instruction->controls.size() == 1);

            result.emplace_back(make_pair(instruction->controls[0], instruction->target), channel->estimated_success_prob);

        }
    }

    sort(result.begin(), result.end(), [](const pair<pair<int, int>, double> &a, const pair<pair<int, int>, double> &b) {
        if (a.second != b.second) {
            return a.second > b.second;
        }
        return a.first > b.first;
    });
    return result;
}

shared_ptr<Channel> HardwareSpecification::get_channel(const shared_ptr<Instruction> &instruction) const {
    assert (instruction->instruction_type != InstructionType::Classical);
    assert (instruction->instruction_type != InstructionType::Projector);
    if (this->quantum_hardware == PerfectHardware) {
        if (instruction->instruction_type == InstructionType::Measurement) {
            return make_shared<MeasurementChannel>(PERFECT_MEAS_CHANNEL);
        } else {
            return make_shared<QuantumChannel>(QuantumChannel());
        }
    } else {
        return this->instructions_to_channels.at(instruction);
    }
}

QuantumHardware HardwareSpecification::get_hardware() const {
    return this->quantum_hardware;
}

HardwareSpecification HardwareSpecification::get_normalized(const unordered_map<int, int> &embedding) const {
    HardwareSpecification result(this->quantum_hardware, embedding.size(), this->basis_gates_type, 1);
    result.basis_gates = this->basis_gates;

    unordered_set<int> used_qubits;
    unordered_map<int, int> rev_embedding;
    // qubits indegree and outdegree
    for (auto e : embedding) {
        if (this->qubit_to_indegree.find(e.second) != this->qubit_to_indegree.end()) {
            result.qubit_to_indegree[e.first] = this->qubit_to_indegree.at(e.second);
        }

        if (this->qubit_to_outdegree.find(e.second) != this->qubit_to_outdegree.end()) {
            result.qubit_to_outdegree[e.first] = this->qubit_to_outdegree.at(e.second);
        }
        used_qubits.insert(e.second);
        result.digraph[e.first] = {};
        result.rev_digraph[e.first] = {};
        rev_embedding[e.second] = e.first;
    }


    // digraphs
    for (auto e : embedding) {
        if (this->digraph.find(e.second) != this->digraph.end()) {
            for (auto succ : this->digraph.at(e.second)) {
                if (used_qubits.find(succ) != used_qubits.end()) {
                    result.digraph[e.first].insert(rev_embedding.at(succ));
                }
            }
        }

        if (this->rev_digraph.find(e.second) != this->rev_digraph.end()) {
            for (auto succ : this->rev_digraph.at(e.second)) {
                if (used_qubits.find(succ) != used_qubits.end()) {
                    result.rev_digraph[e.first].insert(rev_embedding.at(succ));
                }
            }
        }
    }

    // channels
    for (auto e : this->instructions_to_channels) {
        if (e.first->is_used(used_qubits)) {
            result.instructions_to_channels[make_shared<Instruction>(e.first->rename(rev_embedding))] = e.second->rename(rev_embedding);
        }
    }

    return result;

}

bool HardwareSpecification::are_adjacent(const int &q0, const int &q1) const {

    if (this->digraph.find(q0) != this->digraph.end()) {
        if (this->digraph.at(q0).find(q1) != this->digraph.at(q0).end()) {
            return true;
        }
    }

    if (this->digraph.find(q1) != this->digraph.end()) {
        if (this->digraph.at(q1).find(q0) != this->digraph.at(q1).end()) {
            return true;
        }
    }


    return false;
}

set<int> HardwareSpecification::get_ctargets(const int &q) const {
    return this->digraph.at(q);
}

set<string> get_hardware_strings() {
    set<string> result;
    for (int i = 0; i < QuantumHardware::HardwareCount;  i++) {
        result.insert(to_string(static_cast<QuantumHardware>(i)));
    }
    return result;
}

string to_string(const vector<HardwareSpecification> &hardware_specs) {
    vector<string> names;

    for (auto hw : hardware_specs) {
        names.push_back(hw.get_hardware_name());
    }

    return join(names, ",");
}


HardwareSpecification::HardwareSpecification(const QuantumHardware &quantum_hardware, const bool &thermal_relaxation, const bool &optimize) {
    this->quantum_hardware = quantum_hardware;
    if (quantum_hardware == QuantumHardware::PerfectHardware) {
        this->num_qubits = 10;
        this->basis_gates = get_value(BasisGates::TYPE1); // we put anything

        // compute in-degree of qubits (all qubits are connected with all qubits)
        // also compute digraph
        for (int i = 0; i < this->num_qubits; i++) {
            this->qubit_to_indegree[i] = this->num_qubits-1;
            for (int j = 0; j < this->num_qubits; j++) {
                if (i != j) {
                    this->digraph[i].insert(j);
                }
            }
        }
    } else {
        std::filesystem::path source_path(__FILE__);
        std::filesystem::path source_dir = source_path.parent_path();

        // determining hardware specification path
        std::filesystem::path spec_path;
        if (thermal_relaxation) {
            spec_path = source_dir / ("../hardware_specifications/with_thermalization/fake_" + to_string(quantum_hardware) + ".json");
        } else {
            spec_path = source_dir / ("../hardware_specifications/no_thermalization/fake_" + to_string(quantum_hardware) + ".json");
        }

        std::ifstream f(spec_path);
        if (!f.is_open()) {
            std::cerr << "(Failed to open file: "  << spec_path << endl;
        } else {
            LOG.write_debug_ln(to_string(quantum_hardware) + " file opened (" + spec_path.string() + ")");
        }
        json json_hardware_spec = json::parse(f);

        this->num_qubits = json_hardware_spec["num_qubits"];

        // basis gates
        for (string raw_gate : json_hardware_spec["basis_gates"]) {
            this->basis_gates.insert(get_enum_obj(raw_gate));
        }

        this->basis_gates_type = get_basis_gates_type(this->basis_gates);

        vector<json> instructions = json_hardware_spec["instructions"];

        vector<json> channels = json_hardware_spec["channels"];

        assert (instructions.size() == channels.size());

        for(int i=0; i < instructions.size(); i++) {
            shared_ptr<Instruction> instruction = make_shared<Instruction>(instructions[i]);
            shared_ptr<Channel> channel;
            if(instruction->get_instruction_type() == InstructionType::Measurement) {
                channel = make_shared<MeasurementChannel>(channels[i]);
            } else {
                channel = make_shared<QuantumChannel>(channels[i]);
                if (optimize) {
                    static_pointer_cast<QuantumChannel>(channel)->optimize();
                }
            }

            channel->normalize();
            assert(channel->is_normalized());
            this->instructions_to_channels[instruction] = channel;
        }

        // compute degree of qubits
        for (auto it : this->instructions_to_channels) {
            auto ins = it.first;
            if (ins->gate_name == GateName::Cnot || ins->gate_name == GateName::Cz) {
                int target = ins->target;
                if (this->qubit_to_indegree.find(target) == this->qubit_to_indegree.end()) {
                    this->qubit_to_indegree[target] = 0;
                }

                assert(ins->controls.size() == 1);
                if (this->qubit_to_outdegree.find(ins->controls[0]) == this->qubit_to_outdegree.end()) {
                    this->qubit_to_outdegree[ins->controls[0]] = 0;
                }
                this->qubit_to_indegree[target]++;
                this->qubit_to_outdegree[ins->controls[0]]++;
            }
        }

        for (auto qubit = 0; qubit < this->num_qubits; qubit++) {
            if (this->qubit_to_indegree.find(qubit) == this->qubit_to_indegree.end()) {
                this->qubit_to_indegree[qubit] = 0;
            }
        }
        assert(this->qubit_to_indegree.size() == this->num_qubits);

        for (auto it   : this->qubit_to_indegree) {
            assert(it.second >= 0);
        }

        // compute digraph and reverse digraph
        for (auto it : this->instructions_to_channels) {
            auto instruction = it.first;

            if (instruction->gate_name == GateName::Cnot || instruction->gate_name == GateName::Cz) {
                assert (instruction->controls.size() == 1);
                int source = instruction->controls[0];
                int target = instruction->target;
                if (this->digraph.find(source) == this->digraph.end()) {
                    this->digraph[source] = set<int>();
                }
                if (this->rev_digraph.find(target) == this->rev_digraph.end()) {
                    this->rev_digraph[target] = set<int>();
                }
                this->digraph[source].insert(target);
                this->rev_digraph[target].insert(source);
            }
        }
        f.close();
    }
}

HardwareSpecification::HardwareSpecification(const QuantumHardware &quantum_hardware, const int &num_qubits, const BasisGates &basis_gates_type, bool flag) {
    assert(flag);
    this->quantum_hardware = quantum_hardware;
    this->num_qubits = num_qubits;
    this->basis_gates_type = basis_gates_type;
}

HardwareSpecification::HardwareSpecification(const QuantumHardware &quantum_hardware, const bool &thermal_relaxation) {
    LOG.write_debug_ln("light-weight HardwareSpecification constructor called");
    this->quantum_hardware = quantum_hardware;
    if (quantum_hardware == QuantumHardware::PerfectHardware) {
        this->num_qubits = 10;
        this->basis_gates = get_value(BasisGates::TYPE1); // we put anything

        // compute in-degree of qubits (all qubits are connected with all qubits)
        // also compute digraph
        for (int i = 0; i < this->num_qubits; i++) {
            this->qubit_to_indegree[i] = this->num_qubits-1;
            for (int j = 0; j < this->num_qubits; j++) {
                if (i != j) {
                    this->digraph[i].insert(j);
                }
            }
        }
    } else {
        std::filesystem::path source_path(__FILE__);
        std::filesystem::path source_dir = source_path.parent_path();

        // determining hardware specification path
        std::filesystem::path spec_path;
        if (thermal_relaxation) {
            spec_path = source_dir / ("../hardware_specifications/with_thermalization/fake_" + to_string(quantum_hardware) + ".json");
        } else {
            spec_path = source_dir / ("../hardware_specifications/no_thermalization/fake_" + to_string(quantum_hardware) + ".json");
        }

        std::ifstream f(spec_path);
        if (!f.is_open()) {
            std::cerr << "(Failed to open file: "  << spec_path << endl;
        } else {
            LOG.write_debug_ln(to_string(quantum_hardware) + " file opened (" + spec_path.string() + ")");
        }
        json json_hardware_spec = json::parse(f);

        this->num_qubits = json_hardware_spec["num_qubits"];

        // basis gates
        for (string raw_gate : json_hardware_spec["basis_gates"]) {
            this->basis_gates.insert(get_enum_obj(raw_gate));
        }

        this->basis_gates_type = get_basis_gates_type(this->basis_gates);

        f.close();
    }
}

std::string to_string(const QuantumHardware &quantum_hardware) {
    switch(quantum_hardware) {
        case Algiers: return "algiers";
        case Almaden: return "almaden";
        // case Armonk: return "armonk";
        case Athens: return "athens";
        case Auckland: return "auckland";
        case Belem: return "belem";
        case Boeblingen: return "boeblingen";
        case Bogota: return "bogota";
        case Brisbane: return "brisbane";
        case Brooklyn: return "brooklyn";
        case Burlington: return "burlington";
        case Cairo: return "cairo";
        case Cambridge: return "cambridge";
        case Casablanca: return "casablanca";
        case Cusco: return "cusco";
        case Essex: return "essex";
        case Fez: return "fez";
        case Geneva: return "geneva";
        case Guadalupe: return "guadalupe";
        case Hanoi: return "hanoi";
        case Jakarta: return "jakarta";
        case Johannesburg: return "johannesburg";
        case Kawasaki: return "kawasaki";
        case Kolkata: return "kolkata";
        case Kyiv: return "kyiv";
        case Kyoto: return "kyoto";
        case Lagos: return "lagos";
        case Lima: return "lima";
        case London: return "london";
        case Makarresh: return "makarresh";
        case Melbourne: return "melbourne";
        case Manhattan: return "manhattan";
        case Manila: return "manila";
        case Montreal: return "montreal";
        case Mumbai: return "mumbai";
        case Nairobi: return "nairobi";
        case Osaka: return "osaka";
        case Oslo: return "oslo";
        case Ourense: return "ourense";
        case Paris: return "paris";
        case Perth: return "perth";
        case Poughkeepsie: return "poughkeepsie";
        case Prague: return "prague";
        case Singapore: return "singapore";
        case Quito: return "quito";
        case Rochester: return "rochester";
        case Rome: return "rome";
        case Santiago: return "santiago";
        case Sydney: return "sydney";
        case Torino: return "torino";
        case Toronto: return "toronto";
        case Valencia: return "valencia";
        case Vigo: return "vigo";
        case Washington: return "washington";
        case Yorktown: return "yorktown";
        case PerfectHardware: return "perfect_hardware";
        default: throw invalid_argument( "Cannot retrieve string representation of quantum hardware" );
    }
}

string to_string(const set<QuantumHardware> &quantum_hardware) {
    string result;

    for (auto hw : quantum_hardware) {
        if (!result.empty()) {
            result += ", ";
        }
        result += to_string(hw);
    }

    return result;
}

QuantumHardware to_quantum_hardware(const string &quantum_hardware) {
    for (int i = 0; i < QuantumHardware::HardwareCount; i++) {
        auto str_curr = to_string(static_cast<QuantumHardware>(i));
        if (str_curr == quantum_hardware) {
            return static_cast<QuantumHardware>(i);
        }
    }
    throw std::invalid_argument( "QuantumHardware string is not an quantum hardware: " + quantum_hardware);
}

string HardwareSpecification::get_hardware_name() const {
    return to_string(this->quantum_hardware);
}

vector<Instruction> HardwareSpecification::to_basis_gates_impl(const Instruction &current_ins) const {
    if (this->quantum_hardware == QuantumHardware::PerfectHardware) {
        return {current_ins};
    }
    if ((basis_gates.find(current_ins.gate_name) != basis_gates.end() )  ||(current_ins.instruction_type == InstructionType::Measurement) || (current_ins.instruction_type == InstructionType::Classical)) return {current_ins};
    auto temp = unordered_set<BasisGates>({BasisGates::TYPE1, BasisGates::TYPE2, BasisGates::TYPE3, BasisGates::TYPE5, BasisGates::TYPE4, BasisGates::TYPE6});
    if (temp.find(basis_gates_type) != temp.end()) {
        if (current_ins.gate_name == GateName::Cnot) {
            if (basis_gates_type == BasisGates::TYPE5) {

                auto rz_gate = this->to_basis_gates_impl(Instruction(GateName::Rz, current_ins.controls[0], vector<double>({-pi/2})));
                auto rx_gate = this->to_basis_gates_impl(Instruction(GateName::Rx, current_ins.target, vector<double>({pi/2})));
                auto ry_gate = this->to_basis_gates_impl(Instruction(GateName::Ry, current_ins.controls[0], vector<double>({pi})));

                for (auto it: rx_gate) {
                    rz_gate.push_back(it);
                }

                for (auto it: ry_gate) {
                    rz_gate.push_back(it);
                }

                rz_gate.push_back(Instruction(GateName::Ecr, current_ins.controls, current_ins.target));
                return rz_gate;
            } else {
                assert(basis_gates_type == BasisGates::TYPE2 || basis_gates_type == BasisGates::TYPE4);

                auto h_gate = this->to_basis_gates_impl(Instruction(GateName::H, current_ins.target));
                vector<Instruction> result;

                for (auto it: h_gate) {
                    result.push_back(it);
                }
                result.push_back(Instruction(GateName::Cz, current_ins.controls, current_ins.target));

                for (auto it: h_gate) {
                    result.push_back(it);
                }
                return result;
            }

        }
        if (basis_gates_type != BasisGates::TYPE3) {
            vector<Instruction> sx({Instruction(GateName::Sx, current_ins.target)});
            if (current_ins.gate_name == GateName::Ry){
                return {
                    Instruction(GateName::Sx, current_ins.target),
                    Instruction(GateName::Rz, current_ins.target, vector<double>({current_ins.params[0] + pi})),
                    Instruction(GateName::Sx, current_ins.target),
                    Instruction(GateName::Rz, current_ins.target, vector<double>({3*pi}))
                };
            }
            if( current_ins.gate_name == GateName::H) {
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({pi/2})),
                Instruction(GateName::Sx, current_ins.target),
                Instruction(GateName::Rz, current_ins.target, vector<double>({pi/2})),
                };
            }

            if (current_ins.gate_name == GateName::Z)
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({pi}))};
            if (current_ins.gate_name == GateName::U3) {
                vector<Instruction> rz_lambda({
                    Instruction(GateName::Rz, current_ins.target, vector<double>({current_ins.params[2]}))
                });
                vector<Instruction> rz_theta = this->to_basis_gates_impl(
                    Instruction(GateName::Rz, current_ins.target, vector<double>({current_ins.params[0] + pi}))
                );
                vector<Instruction> rz_phi = vector<Instruction>({Instruction(GateName::Rz, current_ins.target, vector<double>({current_ins.params[1] + 3*pi}))});

                for (auto ins : sx) {
                    rz_lambda.push_back(ins);
                }

                for (auto ins : rz_theta) {
                    rz_lambda.push_back(ins);
                }

                for (auto ins : sx) {
                    rz_lambda.push_back(ins);
                }

                for (auto ins : rz_phi) {
                    rz_lambda.push_back(ins);
                }


                return rz_lambda;
            }
            if( current_ins.gate_name == GateName::T)
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({pi/4}))};
            if (current_ins.gate_name == GateName::Td)
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({-pi/4}))};
            if (current_ins.gate_name == GateName::S)
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({-pi/2}))};
            if (current_ins.gate_name == GateName::Sd)
                return {Instruction(GateName::Rz, current_ins.target, vector<double>({pi/2}))};
            if (current_ins.gate_name == GateName::Rx){
                assert(current_ins.params.size() == 1);

                Instruction rz_pi2 = Instruction(GateName::Rz, current_ins.target, vector<double>({pi/2}));
                auto rz_phi = Instruction(GateName::Rz, current_ins.target, vector<double>({current_ins.params[0] + pi}));
                auto rz_pi5pi2 = Instruction(GateName::Rz, current_ins.target, vector<double>({5*pi/2}));

                vector<Instruction> result;
                result.push_back(rz_pi2);

                for (auto ins : sx) {
                    result.push_back(ins);
                }
                result.push_back(rz_phi);
                for (auto ins : sx) {
                    result.push_back(ins);
                }
                result.push_back(rz_pi5pi2);

                return result;
            }
        } else {
            if (current_ins.gate_name == GateName::H)
                return {Instruction(GateName::U2, current_ins.target, vector<double>({0, pi}))};
            if (current_ins.gate_name == GateName::S)
                return {Instruction(GateName::U1, current_ins.target, vector<double>({-pi/2}))};
            if (current_ins.gate_name == GateName::Z)
                return {Instruction(GateName::U1, current_ins.target, vector<double>({pi}))};
            if (current_ins.gate_name == GateName::Ry)
                return {Instruction(GateName::U3, current_ins.target, vector<double>({current_ins.params[0], 0, 0}))};
            if (current_ins.gate_name == GateName::Rx)
                return {Instruction(GateName::U3, current_ins.target, vector<double>({current_ins.params[0], -pi/2, pi/2}))};
            if (current_ins.gate_name == GateName::X)
                return {Instruction(GateName::U3, current_ins.target, vector<double>({pi, 0, pi}))};
        }
    } else {
        throw invalid_argument("Unknown basis gates" + to_string(basis_gates_type));
    }
        throw invalid_argument("[" + this->get_hardware_name() +"]cannot translate to basis gates: " + gate_to_string(current_ins.gate_name));
}