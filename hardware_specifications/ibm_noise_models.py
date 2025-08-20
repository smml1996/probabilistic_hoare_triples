from cmath import cos, sin
from copy import deepcopy
from enum import Enum
from math import isclose
from typing import Any, Dict, List, Optional, Set
import numpy as np
from qiskit import QuantumCircuit, transpile # pyright: ignore
from qiskit_ibm_runtime.fake_provider import * # pyright: ignore
from qiskit_aer.noise import NoiseModel as IBMNoiseModel # pyright: ignore
import json

class Precision:
    PRECISION: int = 8  # round number to `PRECISION` floating point digits
    isclose_abstol: Optional[int] = None
    rel_tol: Optional[int] = None
    is_lowerbound: bool = True
    @staticmethod
    def update_threshold():
        Precision.isclose_abstol = 1/(10**(Precision.PRECISION-1))  
        Precision.rel_tol = 1/(10**(Precision.PRECISION-1))  

class Op(Enum):
    # PAULI GATES
    X = "X"
    Y = "Y"
    Z = "Z"
    I = "I"

    SX = "SX"
    SXD = "SXD"
    S = "S"
    SD = "SD"
    U1 = "U1"
    U1D = "U1D"
    U2 = "U2"
    U2D = "U2D"
    U3 = "U3"
    U3D = "U3D"
    TD = "TD"
    T = "T"
    
    RZ = "RZ"
    RX = "RX"
    RY = "RY"
    
    # HADAMARD
    H = "H"

    # MULTI-QUBIT GATES
    CNOT = "CNOT"
    ECR = "ECR"
    RZX = "RZX"
    RZZ = "RZZ"
    CZ = "CZ"
    CH = "CH"
    SWAP= "SWAP"

    # MEASUREMENT
    MEAS = "MEASURE"
    P0 = "P0"
    P1 = "P1"

    # NON-UNITARY
    RESET = "RESET"

    # ClassicalOp
    WRITE0 = "WRITE0"
    WRITE1 = "WRITE1"
    TOGGLE = "TOGGLE"

    DELAY = "DELAY"
    CUSTOM = "CUSTOM"
    
    FOR_LOOP = 'FOR_LOOP'
    IF_ELSE = 'IF_ELSE'
    SWITCH_CASE = 'SWITCH_CASE'
    def __repr__(self) -> str:
        return self.__str__()


class HardwareSpec(Enum):
    # Quantum hardware names available in Qiskit
    ALGIERS = "fake_algiers"
    BRISBANE = "fake_brisbane" # uses ECR gate
    CUSCO = "fake_cusco" # uses ECR gate
    FEZ = "fake_fez"
    KAWASAKI = "fake_kawasaki" # uses ecr gate
    KYIV = "fake_kyiv" # uses ecr gate
    KYOTO = "fake_kyoto" # uses ecr gate
    MAKARRESH = "fake_makarresh"
    OSAKA = "fake_osaka" # uses ecr gate
    TORINO = "fake_torino"
    VALENCIA = "fake_valencia"
    JOHANNESBURG = "fake_johannesburg"
    PERTH = "fake_perth"
    LAGOS = "fake_lagos"
    NAIROBI = "fake_nairobi"
    HANOI = "fake_hanoi"
    CAIRO = "fake_cairo" # uses ecr gate
    MUMBAI = "fake_mumbai"
    KOLKATA = "fake_kolkata"
    PRAGUE = "fake_prague"
    ALMADEN = "fake_almaden"
    ARMONK = "fake_armonk"
    ATHENS = "fake_athens"
    AUCKLAND = "fake_auckland"
    BELEM = "fake_belem"
    BOEBLINGEN = "fake_boeblingen"
    BOGOTA = "fake_bogota"
    BROOKLYN = "fake_brooklyn"
    BURLINGTON = "fake_burlington"
    CAMBRIDGE = "fake_cambridge"
    CASABLANCA = "fake_casablanca"
    ESSEX = "fake_essex"
    GENEVA = "fake_geneva"
    GUADALUPE = "fake_guadalupe"
    LIMA = "fake_lima"
    LONDON = "fake_london"
    MANHATTAN = "fake_manhattan"
    MANILA = "fake_manila"
    MELBOURNE = "fake_melbourne"
    MONTREAL = "fake_montreal"
    OSLO = "fake_oslo"
    OURENSE = "fake_ourense"
    PARIS = "fake_paris"
    QUITO = "fake_quito"
    POUGHKEEPSIE = "fake_poughkeepsie"
    ROCHESTER = "fake_rochester"
    ROME = "fake_rome"
    SANTIAGO = "fake_santiago"
    SINGAPORE = "fake_singapore"
    SYDNEY = "fake_sydney"
    TORONTO = "fake_toronto"
    VIGO = "fake_vigo"
    WASHINGTON = "fake_washington"
    YORKTOWN = "fake_yorktown"
    JAKARTA = "fake_jakarta"
    def __repr__(self) -> str:
        return self.__str__()


def get_ibm_noise_model(hardware_spec: HardwareSpec, thermal_relaxation: bool=True) -> IBMNoiseModel: # pyright: ignore
    backend = get_backend(hardware_spec)
    ibm_noise_model = IBMNoiseModel.from_backend(backend, thermal_relaxation=thermal_relaxation) # pyright: ignore
    return ibm_noise_model # pyright: ignore

def get_backend(hardware_spec: HardwareSpec):
    backend_ = hardware_spec
    if backend_ == HardwareSpec.ALGIERS:
        backend = FakeAlgiers()
    elif backend_ == HardwareSpec.BRISBANE:
        backend = FakeBrisbane()
    elif backend_ == HardwareSpec.CUSCO:
        backend = FakeCusco()
    elif backend_ == HardwareSpec.FEZ:
        backend = FakeFez()
    elif backend_ == HardwareSpec.KAWASAKI:
        backend = FakeKawasaki()
    elif backend_ == HardwareSpec.KYIV:
        backend = FakeKyiv()
    elif backend_ == HardwareSpec.KYOTO:
        backend = FakeKyoto()
    elif backend_ == HardwareSpec.MAKARRESH:
        backend = FakeMarrakesh()
    elif backend_ == HardwareSpec.OSAKA:
        backend = FakeOsaka()
    elif backend_ == HardwareSpec.TORINO:
        backend = FakeTorino()
    elif backend_ == HardwareSpec.VALENCIA:
        backend = FakeValenciaV2()
    elif backend_ == HardwareSpec.JOHANNESBURG:
        backend = FakeJohannesburgV2()
    elif backend_ == HardwareSpec.PERTH:
        backend = FakePerth()
    elif backend_ == HardwareSpec.LAGOS:
        backend = FakeLagosV2()
    elif backend_ == HardwareSpec.NAIROBI:
        backend = FakeNairobiV2()
    elif backend_ ==  HardwareSpec.HANOI:
        backend = FakeHanoiV2()
    elif backend_ == HardwareSpec.CAIRO:
        backend = FakeCairoV2()
    elif backend_ == HardwareSpec.MUMBAI:
        backend = FakeMumbaiV2()
    elif backend_ == HardwareSpec.KOLKATA:
        backend = FakeKolkataV2()
    elif backend_ == HardwareSpec.PRAGUE:
        backend = FakePrague()
    elif backend_ == HardwareSpec.ALMADEN:
        backend = FakeAlmadenV2()
    elif backend_ == HardwareSpec.ARMONK:
        backend = FakeArmonkV2()
    elif backend_ == HardwareSpec.ATHENS:
        backend = FakeAthensV2()
    elif backend_ == HardwareSpec.AUCKLAND:
        backend = FakeAuckland()
    elif backend_ == HardwareSpec.BELEM:
        backend = FakeBelemV2()
    elif backend_ == HardwareSpec.BOEBLINGEN:
        backend = FakeBoeblingenV2()
    elif backend_ == HardwareSpec.BOGOTA:
        backend = FakeBogotaV2()
    elif backend_ == HardwareSpec.BROOKLYN:
        backend = FakeBrooklynV2()
    elif backend_ == HardwareSpec.BURLINGTON:
        backend = FakeBurlingtonV2()
    elif backend_ == HardwareSpec.CAMBRIDGE:
        backend = FakeCambridgeV2()
    elif backend_ == HardwareSpec.CASABLANCA:
        backend = FakeCasablancaV2()
    elif backend_ == HardwareSpec.ESSEX:
        backend = FakeEssexV2()
    elif backend_ == HardwareSpec.GENEVA:
        backend = FakeGeneva()
    elif backend_ == HardwareSpec.GUADALUPE:
        backend = FakeGuadalupeV2()
    elif backend_ == HardwareSpec.LIMA:
        backend = FakeLimaV2()
    elif backend_ == HardwareSpec.LONDON:
        backend = FakeLondonV2()
    elif backend_ == HardwareSpec.MANHATTAN:
        backend = FakeManhattanV2()
    elif backend_ == HardwareSpec.MANILA:
        backend = FakeManilaV2()
    elif backend_ == HardwareSpec.MELBOURNE:
        backend = FakeMelbourneV2()
    elif backend_ == HardwareSpec.MONTREAL:
        backend = FakeMontrealV2()
    elif backend_ == HardwareSpec.OSLO:
        backend = FakeOslo()
    elif backend_ == HardwareSpec.OURENSE:
        backend = FakeOurenseV2()
    elif backend_ == HardwareSpec.JAKARTA:
        backend = FakeJakartaV2()
    elif backend_ == HardwareSpec.PARIS:
        backend = FakeParisV2()
    elif backend_ == HardwareSpec.QUITO:
        backend = FakeQuitoV2()
    elif backend_ == HardwareSpec.POUGHKEEPSIE:
        backend = FakePoughkeepsieV2()
    elif backend_ == HardwareSpec.ROCHESTER:
        backend = FakeRochesterV2()
    elif backend_ == HardwareSpec.ROME:
        backend = FakeRomeV2()
    elif backend_ == HardwareSpec.SANTIAGO:
        backend = FakeSantiagoV2()
    elif backend_ == HardwareSpec.SINGAPORE:
        backend = FakeSingaporeV2()
    elif backend_ == HardwareSpec.SYDNEY:
        backend = FakeSydneyV2()
    elif backend_ == HardwareSpec.TORONTO:
        backend = FakeTorontoV2()
    elif backend_ == HardwareSpec.VIGO:
        backend = FakeVigoV2()
    elif backend_ == HardwareSpec.WASHINGTON:
        backend = FakeWashingtonV2()
    elif backend_ == HardwareSpec.YORKTOWN:
        backend = FakeYorktownV2()
    elif backend_ == HardwareSpec.JAKARTA:
        backend = FakeJakartaV2()
    else:
        raise Exception("Could not retrieve backend", hardware_spec)
    return backend

def is_multiqubit_gate(op: Op):
    assert isinstance(op, Op)
    if op in [Op.CNOT, Op.CZ, Op.SWAP, Op.CH, Op.ECR, Op.RZX, Op.RZZ]:
        return True
    return False

class BasisGates(Enum):
    TYPE2 = set([Op.CNOT, Op.MEAS, Op.RESET, Op.RZ, Op.SX, Op.X])
    TYPE4 = set([Op.CZ, Op.MEAS, Op.RESET, Op.RZ, Op.SX, Op.X])
    TYPE8 = set([Op.U1, Op.RESET, Op.U3, Op.MEAS, Op.U2, Op.CNOT])
    TYPE9 = set([Op.RESET, Op.MEAS, Op.RZ, Op.SX, Op.X])
    
    # basis sets with ECR
    TYPE10 = set([Op.RZ, Op.MEAS, Op.RESET, Op.SX, Op.ECR, Op.X])
    TYPE11 = set([Op.RZ, Op.MEAS, Op.RESET, Op.SX, Op.ECR, Op.X, Op.CNOT])
    
def get_basis_gate_type(basis_gates: List[Op]):
    filtered_basis_gates: Set[Op] = set()
    
    for basis_gate in basis_gates:
        if not (basis_gate in [Op.FOR_LOOP, Op.IF_ELSE, Op.SWITCH_CASE, Op.DELAY, Op.I]):
            filtered_basis_gates.add(basis_gate) # pyright: ignore

    for b in BasisGates:
        if b.value == filtered_basis_gates:
            return b
    raise Exception(f"No type matches with the current basis gates ({filtered_basis_gates})")
    
def is_pauli(op: Op):
    return op in [Op.X, Op.Z, Op.Y, Op.I]
    
def get_op(op_: str) -> Op:
    '''used to get an Operator (Enum defined above) with name op_
    '''
    op_ = op_.strip().upper()
    if op_ == "CX":
        op_ = "CNOT"
    if op_ == "ID":
        op_ = "I"
    for op in Op:
        if op.value == op_:
            return op
    raise Exception("Could not retrieve operator", op_)

class GateData: # this was previously called GateData
    label: Op
    address: int
    controls: Optional[int]
    params: Optional[List[float]]

    def __init__(self, label: Op, address: int, control: Optional[int]=None, params: Optional[List[float]]=None) -> None:
        self.label = label
        self.address = address
        self.control = control
        self.params = params

    def __eq__(self, other: object) -> bool:
        return (self.label, self.address, self.control, self.params) == (
        other.label, other.address, other.control, self.params) # pyright: ignore

    def __str__(self) -> str:
        d: Dict[str, Any] = dict()
        d['gate'] = self.label
        d['address'] = self.address
        d['controls'] = self.control
        d['params'] = self.params
        return d.__str__()

    def __repr__(self) -> str:
        return self.__str__()

class Instruction:
    real_target:int
    target: int
    control: Optional[int]
    op: Op
    params: Any
    def __init__(self, target: int, op: Op, control: Optional[int] = None, params: Any = None, name: Optional[str]=None, symbols: Optional[List[str]]=None, real_target: int=-1) -> None:
        assert isinstance(op, Op)
        assert isinstance(target, int)
        assert isinstance(control, int) or (control is None)
        self.target = target
        self.real_target=real_target
        self.op = op
        self.params = params
        if (not is_multiqubit_gate(op)) and (control is not None):
            raise Exception(f"controls are initialized in a non-multiqubit gate ({op} {control})")
        elif is_multiqubit_gate(op) and control is None:
            raise Exception(f"{op} gate should have exactly 1 control ({control}) qubit")
        if target == control:
            raise Exception("target is in controls")
        self.control = control
        self.name = name # pyright: ignore
    
    def is_classical(self):
        return self.op in [Op.WRITE0, Op.WRITE1, Op.TOGGLE]

    def name(self):
        if self.control is None:
            return f"{self.op.name}-{self.target}"
        else:
            return f"{self.op.name}-{self.control}-{self.target}"
        
    def get_control(self)->str:
        return str(self.control)
        
    def get_target(self)->str:
        return str(self.target)
    
    
    def is_meas_instruction(self):
        return self.op in [Op.MEAS]

    def __eq__(self, value: object) -> bool:
        assert not isinstance(value, KrausOperator)
        return self.target == value.target and self.control == value.control and self.op == value.op # pyright: ignore
    
    def __hash__(self):
        return hash((self.op.value, self.target, self.control, self.params))
    
    def serialize(self) -> Dict[str, Any]:
        
        if self.control is None:
            control = -1
        else:
            control = self.control
            
        if self.params is None:
            params = []
        else:
            params = self.params.tolist()
        return {
            'type': 'instruction',
            'target': self.target,
            'control': control,
            'op': self.op.name,
            'params': params
        }
        
    def __str__(self) -> str:
        return f"Instruction(target={self.target}, control={self.control}, op={self.op})"
    
    def __repr__(self) -> str:
        return self.__str__()
    
class ComplexEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, complex):
            return {"real": obj.real, "imag": obj.imag}
        return json.JSONEncoder.default(self, obj)

class QuantumChannel:
    def __init__(self, all_ins_sequences, all_probabilities, target_qubits, flatten=True) -> None:
        self.errors = [] # list of list of sequences of instructions/kraus operators
        self.probabilities = all_probabilities
        for seq in all_ins_sequences:
            new_seq = QuantumChannel.translate_err_sequence(seq, target_qubits)
            self.errors.append(new_seq)
        assert len(self.errors) == len(self.probabilities)

        if len(self.probabilities) == 0:
            self.probabilities = [1.0]
            self.errors = [[Instruction(target_qubits[0], Op.I)]]
        else:
            assert len(self.errors) > 0
        
        self.flatten()
        
        self.__check_probabilities()
        
    def __str__(self) -> str:
        return {"type": "QuantumChannel", "errors": self.errors, "probs":self.probabilities}.__str__()
    
    def __repr__(self):
        return self.__str__()

    def __check_probabilities(self):
        assert len(self.probabilities) > 0
        for p in self.probabilities:
            assert 0.0 < p <= 1.0

    def flatten_sequence(self, err_seq):
        sequences = []
        for err in err_seq:
            if isinstance(err, Instruction):
                if len(sequences) == 0:
                    sequences.append([err])
                else:
                    for seq in sequences:
                        seq.append(err)
            else:
                assert isinstance(err, KrausOperator)
                if len(sequences) == 0:
                    for matrix in err.operators:
                        sequences.append([Instruction(err.target, Op.CUSTOM, params=matrix)])
                else:
                    all_seqs_temp = []
                    for seq in sequences:
                        for matrix in err.operators:
                            temp_seq = deepcopy(seq)
                            temp_seq.append(Instruction(err.target, Op.CUSTOM, params=matrix))
                            all_seqs_temp.append(temp_seq)

                    sequences = all_seqs_temp
                

        assert len(sequences) > 0
        return sequences

    def flatten(self):
        total_probabilities = sum(self.probabilities)
        assert isclose(total_probabilities, 1.0, rel_tol=Precision.rel_tol)
        new_probabilities = []
        new_errors = []

        for (err_seq, prob) in zip(self.errors, self.probabilities):
            flattened_sequences = self.flatten_sequence(err_seq)

            for flattened_seq in flattened_sequences:
                new_probabilities.append(prob)
                new_errors.append(flattened_seq)

        self.errors = new_errors
        self.probabilities = new_probabilities


    def serialize(self):
        serialized_errors = []
        for err_seq in self.errors:
            temp_seq = []
            for e in err_seq:
                temp_seq.append(e.serialize())
            serialized_errors.append(temp_seq)
        return {
            'probabilities': [float(x) for x in self.probabilities],
            'errors': serialized_errors
        }

    @staticmethod
    def optimize_err_sequence(err_seq):
        # remove all identities
        new_seq1 = []
        for instruction in err_seq:
            if isinstance(instruction, KrausOperator) or instruction.op != Op.I:
                new_seq1.append(instruction)

        # replace Y gates for XZ (its the same up to a global phase)
        new_seq2 = []
        for instruction in new_seq1:
            if isinstance(instruction, KrausOperator) or instruction.op != Op.Y:
                new_seq2.append(instruction)
            else:
                assert instruction.op == Op.Y
                assert instruction.control is None
                new_seq2.append(Instruction(instruction.target, Op.X))
                new_seq2.append(Instruction(instruction.target, Op.Z))
                
        temp_seq = []
        new_seq3 = []
        for instruction in new_seq2:
            if isinstance(instruction, KrausOperator) or instruction.op == Op.RESET:
                new_seq3.extend(temp_seq)
                temp_seq = []
                new_seq3.append(instruction)
            else:
                assert is_pauli(instruction.op)
                temp_seq.append(instruction)
        new_seq3.extend(temp_seq)
        return new_seq3
    
    @staticmethod
    def translate_err_sequence(err_seq, target_qubits):
        answer = []
        for err in err_seq:
            if err['name'] == 'pauli':
                assert len(target_qubits) == 2
                assert len(err['params']) == 1
                assert len(err['params'][0]) == 2 # II, IX, IZ, XX, etc
                for (p, qubit) in zip(err['params'][0], err['qubits']):
                    op = get_op(p)
                    target_qubit = target_qubits[qubit]
                    answer.append(Instruction(target_qubit, op))
            elif err['name'] == 'kraus':
                assert len(err['qubits']) == 1
                answer.append(KrausOperator(err['params'], target_qubits[err['qubits'][0]]))
            else:
                op = get_op(err['name'])
                assert len(err['qubits']) == 1
                target_qubit = target_qubits[err['qubits'][0]]
                answer.append(Instruction(target_qubit, op))
        
        return answer

class KrausOperator:
    def __init__(self, operators, qubit) -> None:
        for operator in operators:
            assert operator.shape == (2,2) # for now we are dealing only with single qubit operators
        self.operators = operators # these are matrices
        self.target = qubit

    def serialize(self):
        serialized_operators = []
        for op in self.operators:
            curr_op = []
            for l in op:
                temp_l = []
                for element in l:
                    temp_l.append({'real': element.real, 'im': element.imag})
                curr_op.append(temp_l)
            serialized_operators.append(curr_op)
            

        return {
            'type': 'kraus',
            'target': self.target,
            'ops': serialized_operators,
        }



class MeasChannel:
    def __init__(self, all_probabilities) -> None:
        assert len(all_probabilities) == 2
        self.meas_errors = dict()

        zero_meas_err = all_probabilities[0]
        assert len(zero_meas_err) == 2
        self.meas_errors[0] = dict()
        self.meas_errors[0][0] = zero_meas_err[0] # probability that measurement outcome is 0 given that the ideal outcome should have been 0
        self.meas_errors[0][1] = zero_meas_err[1] # probability that measurement outcome is 1 given that the ideal outcome should have been 0

        one_meas_err = all_probabilities[1]
        assert len(one_meas_err) == 2
        self.meas_errors[1] = dict()
        self.meas_errors[1][0] = one_meas_err[0] # probability that measurement outcome is 0 given that the ideal outcome should have been 1
        self.meas_errors[1][1] = one_meas_err[1] # probability that measurement outcome is 1 given that the ideal outcome should have been 1
    
    def get_success_probability(self):
        return self.get_ind_probability(0,0) + self.get_ind_probability(1,1)
        
    def get_ind_probability(self, ideal_outcome: int, noisy_outcome: int):
        assert ideal_outcome in [0, 1]
        assert noisy_outcome in [0, 1]
        return self.meas_errors[ideal_outcome][noisy_outcome]
    
    def serialize(self):
        return self.meas_errors
    
    def __str__(self) -> str:
        return {"type": "MeasChannel", "errors": self.meas_errors}.__str__()
    
    def __repr__(self):
        return self.__str__()
            

class NoiseModel:
    hardware_spec: HardwareSpec
    basis_gates: List[Op]
    instructions_to_channel: Dict[Instruction, QuantumChannel|MeasChannel]
    instructions_to_duration: Dict[Instruction, float]
    num_qubits: int
    qubit_to_indegree: Dict[int, int] # tells mutiqubit gates have as target a given qubit (key)
    qubit_to_outdegree: Dict[int, int]
    
    def load_noise_model(self, thermal_relaxation):
        ibm_noise_model = get_ibm_noise_model(self.hardware_spec, thermal_relaxation=thermal_relaxation)
        assert isinstance(ibm_noise_model, IBMNoiseModel)
        
        self.basis_gates = get_basis_gate_type([get_op(op) for op in ibm_noise_model.basis_gates])
        self.instructions_to_channel = dict()
        self.num_qubits = len(ibm_noise_model.noise_qubits)

        self.qubit_to_indegree = dict()
        self.qubit_to_outdegree = dict()
        # start translating quantum channels
        all_errors = ibm_noise_model.to_dict()
        
        assert len(all_errors.keys()) == 1

        all_errors = all_errors['errors'] 

        for error in all_errors:
            target_instructions = error['operations'] # this error applies to these instructions
            assert len(target_instructions) == 1 # we are assumming that errors target only 1       instruction at once
            op = get_op(target_instructions[0])

            assert len(error['gate_qubits']) == 1
            error_target_qubits = error['gate_qubits'][0] # this error targets the following qubits
            control = None
            if len(error_target_qubits) > 1:
                assert len(error_target_qubits) == 2 # the only gates for multiqubit gates at IBM are CX gates, therefore at most, this error targets 2 qubits
                control = error_target_qubits[0]
                target = error_target_qubits[1]
                target_qubits = [control, target]

                assert is_multiqubit_gate(op)
                if target not in self.qubit_to_indegree.keys():
                    self.qubit_to_indegree[target] = 0
                if control not in self.qubit_to_outdegree.keys():
                    self.qubit_to_outdegree[control] = 0
                self.qubit_to_indegree[target] += 1
                self.qubit_to_outdegree[control] += 1
            else:
                target = error_target_qubits[0]
                target_qubits = [target]
                
            target_instruction = Instruction(target, op, control)
            probabilities = error['probabilities']
            if error['type'] == "qerror":    
                error_instructions = error['instructions']
                self.instructions_to_channel[target_instruction] = QuantumChannel(error_instructions, probabilities, target_qubits)
            else:
                assert error['type'] == "roerror"
                self.instructions_to_channel[target_instruction] = MeasChannel(probabilities)
        
        # check that all single qubit gates exist
        report = dict()
        for qubit in range(self.num_qubits):
            for op in self.basis_gates.value:
                assert isinstance(op, Op)
                if not is_multiqubit_gate(op):
                    instruction_ = Instruction(qubit, op)
                    if instruction_ not in self.instructions_to_channel.keys():
                        if op not in report.keys():
                            report[op] = 0
                        report[op] += 1

                        # create a perfect quantum channel for this operation
                        self.instructions_to_channel[instruction_] = QuantumChannel([], [], [qubit])
        self.report = report
        self.digraph = self.get_digraph_()
        # if len(report.keys()) > 0:
        #     print(f"WARNING ({hardware_specification.value}) (qubits={self.num_qubits}) ({self.basis_gates.value}): no quantum channel found for {report}")
        
    def get_durations(self) -> Dict[Op, float]:
        answer = dict()
        backend = get_backend(self.hardware_spec)
        gates = backend.properties().gates
        
        for gate in gates:
            for param in gate.parameters:
                if param.name == "gate_length":
                    assert param.unit == "ns"
                    assert isinstance(gate.gate, str)
                    op = get_op(gate.gate)
                    duration = param.value
                    if len(gate.qubits) > 1:
                        assert len(gate.qubits) == 2
                        assert is_multiqubit_gate(op)
                        control = gate.qubits[0]
                        target = gate.qubits[1]
                    else:
                        control = None
                        target = gate.qubits[0]
                    instruction = Instruction(target, op, control=control)
                    assert instruction not in answer.keys()
                    answer[instruction] = duration
        for qubit_idx, qubit_props in enumerate(backend.properties().qubits):
            for param in qubit_props:
                if param.name == "readout_length":
                    assert param.unit == "ns"
                    instruction = Instruction(qubit_idx, Op.MEAS)
                    assert instruction not in answer.keys()
                    answer[instruction] = param.value
        return answer
        
    def __init__(self, hardware_specification: HardwareSpec=None, thermal_relaxation=True) -> None:
        self.hardware_spec = hardware_specification
        self.thermal_relaxation = thermal_relaxation
        
        if hardware_specification is not None:
            self.load_noise_model(thermal_relaxation=thermal_relaxation)
        else:
            self.instructions_to_channel = dict()
            self.num_qubits = None
            self.basis_gates = []
            self.report = None
            self.digraph = None
        self.instructions_to_duration = self.get_durations()

    def get_digraph_(self):
        answer = dict()
        for instruction in self.instructions_to_channel.keys():
            if is_multiqubit_gate(instruction.op):
                source = instruction.control
                target = instruction.target
                
                if source not in answer.keys():
                    answer[source] = set()
                answer[source].add(target)
        return answer
            
    def get_qubit_indegree(self, qubit) -> int:
        if qubit in self.qubit_to_indegree.keys():
            return self.qubit_to_indegree[qubit]
        else:
            return 0
        
    def get_qubit_outdegree(self, qubit) -> int:
        if qubit in self.qubit_to_outdegree.keys():
            return self.qubit_to_outdegree[qubit]
        else:
            return 0
        
    def get_most_noisy_control(self, target):
        answer_qubit = None
        succ_prob = None
        for (control, targets) in self.digraph.items():
            for target_ in targets:
                if target_ == target:
                    instruction  = Instruction(target, Op.CNOT, control)
                    channel = self.instructions_to_channel[instruction]
                    assert isinstance(channel, QuantumChannel)
                    if answer_qubit is None:
                        answer_qubit = control
                        succ_prob = channel.estimated_success_prob
                    elif succ_prob > channel.estimated_success_prob:
                        answer_qubit = control
                        succ_prob = channel.estimated_success_prob
        return answer_qubit
                    
    
    def get_most_noisy_target(self, control):
        answer_qubit = None
        succ_prob = None
        for target in self.digraph[control]:
            instruction = Instruction(target, Op.CNOT, control)
            channel = self.instructions_to_channel[instruction]
            assert isinstance(channel, QuantumChannel)
            succ_prob_ = channel.estimated_success_prob
            if answer_qubit is None:
                answer_qubit = target
                succ_prob = succ_prob_
            elif succ_prob_ < succ_prob:
                succ_prob = succ_prob_
                answer_qubit = target
        return answer_qubit

    def get_most_noisy_neighbour(self, qubit: int, is_target=True) -> List[int]:
        ''' we are looking for a qubit that can serve as control in a CX gate if is_target=True.
        Otherwise, we are looking for a neighbouring qubit that can serve as target in a CX gate.
        '''
        if is_target:
            return self.get_most_noisy_target(qubit)
        return self.get_most_noisy_control(qubit)
        
    
    def get_qubit_couplers(self, target: int, is_target=True) -> List[int]:
        ''' Returns a list of pairs (qubit_control, QuantumChannel) in which the instruction is a multiqubit gate whose target is the given qubit
        '''
        assert (target >= 0)
        result = []

        for (instruction, channel) in self.instructions_to_channel.items():
            assert isinstance(instruction, Instruction)
            if is_multiqubit_gate(instruction.op):
                assert isinstance(instruction.target, int)
                assert isinstance(instruction.control, int)
                if is_target:
                    if target == instruction.target:
                        result.append((instruction.control, channel))
                else:
                    if target == instruction.control:
                        result.append((instruction.target, channel))

        result = sorted(result, key=lambda x : x[1].estimated_success_prob, reverse=False)
        return result
    
    def get_most_noisy_couplers(self) -> List:
        result = []
        for (instruction, channel) in self.instructions_to_channel.items():
            assert isinstance(instruction, Instruction)
            if is_multiqubit_gate(instruction.op):
                assert isinstance(instruction.target, int)
                assert isinstance(instruction.control, int)
                result.append(((instruction.control, instruction.target), channel))

        result = sorted(result, key=lambda x : x[1].estimated_success_prob, reverse=False)
        return result
    
    def serialize(self):
        instructions = []
        channels = []
        durations = []
        for (instruction, channel) in self.instructions_to_channel.items():
            instructions.append(instruction.serialize())
            channels.append(channel.serialize())
            if instruction in self.instructions_to_duration.keys():
                durations.append(self.instructions_to_duration[instruction])
            else:
                durations.append(0)

        assert len(instructions) == len(channels)
        return {
            'name': self.hardware_spec.value,
            "thermalization":  1 if self.thermal_relaxation else 0,
            "num_qubits": self.num_qubits,
            "basis_gates_type": str(self.basis_gates.name),
            'basis_gates': [str(x.name) for x in self.basis_gates.value],
            'instructions': instructions,
            'channels': channels,
            'durations': durations
        }
    
    def get_instruction_channel(self, instruction):
        assert isinstance(instruction, Instruction)
        if self.hardware_spec is None:
            if instruction not in self.instructions_to_channel.keys():
                if instruction.is_meas_instruction():
                    channel = MeasChannel([[1.0, 0.0], [0.0, 1.0]])
                else:
                    channel = QuantumChannel([], [], [0])
                self.instructions_to_channel[instruction] = channel
        return self.instructions_to_channel[instruction]
        
    # functions that help to choose embeddings follow
    def get_most_noisy_qubit(self, op: Op, top=1, reverse=False) -> List[int]:
        
        assert (op in self.basis_gates.value) or (op == Op.MEAS)
        
        qubits_and_noises: List[int] = []
        for (instruction, channel) in self.instructions_to_channel.items():
            if instruction.op == op:
                if isinstance(channel, QuantumChannel):
                    if is_multiqubit_gate(op):
                        qubits_and_noises.append((channel.estimated_success_prob, (instruction.target, instruction.control)))
                    else:
                        qubits_and_noises.append((channel.estimated_success_prob, instruction.target))
                else:
                    assert isinstance(channel, MeasChannel)
                    qubits_and_noises.append((channel.get_success_probability()/2.0, instruction.target))
                    
        qubits_and_noises = sorted(qubits_and_noises, key=lambda x : x[0], reverse=reverse)
        return qubits_and_noises

HARDWARE_SPECS_PATH = "hardware_specs/"

def dump_hardware_spec(hardware_spec: HardwareSpec, with_thermalization: bool):
    noise_model = NoiseModel(hardware_spec, thermal_relaxation=with_thermalization)
    f = open(f"{HARDWARE_SPECS_PATH}{'with_thermalization' if with_thermalization else 'no_thermalization'}/{hardware_spec.value}.json", "w")
    json.dump(noise_model.serialize(), f, indent=4, cls=ComplexEncoder)
    f.close()
    
    

if __name__ == "__main__":
    Precision.PRECISION = 10
    Precision.update_threshold()
    for hardware_spec in HardwareSpec:
        print(f"dumping {hardware_spec.value}")
        dump_hardware_spec(hardware_spec, True)
        # dump_hardware_spec(hardware_spec, False)
