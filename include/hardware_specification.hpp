#ifndef HARDWARE_SPEC_H
#define HARDWARE_SPEC_H
#include <string>
#include <unordered_set>
#include <set>
#include <map>
#include "quantum_gates.hpp"
#include "instruction.hpp"
#include "channels.hpp"

using namespace std;


// all hardware specifications we can run experiments on
enum QuantumHardware { 
    Algiers,
    Almaden,
    // Armonk, // this one has only 1 qubit. Not useful for most of our experiments
    Athens,
    Auckland,
    Belem,
    Boeblingen,
    Bogota,
    Brisbane,
    Brooklyn,
    Burlington,
    Cairo,
    Cambridge,
    Casablanca,
    Cusco,
    Essex,
    Fez,
    Geneva,
    Guadalupe,
    Hanoi,
    Jakarta,
    Johannesburg,
    Kawasaki,
    Kolkata,
    Kyiv,
    Kyoto,
    Lagos,
    Lima,
    London,
    Makarresh,
    Melbourne,
    Manhattan,
    Manila,
    Montreal,
    Mumbai,
    Nairobi,
    Osaka,
    Oslo,
    Ourense,
    Paris,
    Perth,
    Poughkeepsie,
    Prague,
    Quito,
    Rochester,
    Rome,
    Santiago,
    Singapore,
    Sydney,
    Torino,
    Toronto,
    Valencia,
    Vigo,
    Washington,
    Yorktown,
    PerfectHardware,
    HardwareCount
};

string to_string(const QuantumHardware &quantum_hardware); // gets the string representation of the enum QuantumHardware
string to_string(const set<QuantumHardware> &quantum_hardware);
QuantumHardware to_quantum_hardware(const string &quantum_hardware);

class HardwareSpecification {
    QuantumHardware quantum_hardware;
    unordered_map<int, int> qubit_to_indegree;
    unordered_map<int, int> qubit_to_outdegree;
    public:
        int num_qubits;
        BasisGates basis_gates_type;
        map<int, set<int>> digraph;
        map<int, set<int>> rev_digraph;
        unordered_map<shared_ptr<Instruction>, shared_ptr<Channel>, InstructionHash, InstructionPtrEqual>
        instructions_to_channels;
        unordered_set<GateName> basis_gates;
        HardwareSpecification(const QuantumHardware &quantum_hardware, const bool &thermal_relaxation,
                              const bool &optimize);
        HardwareSpecification(const QuantumHardware &quantum_hardware, const int &num_qubits, const BasisGates &basis_gates_type);
        [[nodiscard]] string get_hardware_name() const;
        [[nodiscard]] int get_qubit_indegree(int qubit) const;
        [[nodiscard]] int get_qubit_outdegree(int qubit) const;
        [[nodiscard]] vector<Instruction> to_basis_gates_impl(const Instruction &current_ins) const;
        [[nodiscard]] vector<pair<int, double>> get_sorted_qubit_couplers(int target) const;
        [[nodiscard]] vector<pair<pair<int, int>, double>> get_sorted_qubit_couplers2() const;
        [[nodiscard]] shared_ptr<Channel> get_channel(const shared_ptr<Instruction> &) const;
        [[nodiscard]] QuantumHardware get_hardware() const;
        HardwareSpecification get_normalized(const unordered_map<int, int> &embedding) const;
        set<int> get_ctargets(const int &q) const;
};

set<string> get_hardware_strings();
string to_string(const vector<HardwareSpecification> &hardware_specs);
#endif
