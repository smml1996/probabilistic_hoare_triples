from copy import deepcopy
from enum import Enum
from math import isclose
from typing import Any, Dict, List, Optional, Set, Union
import numpy as np
from qiskit_ibm_runtime.fake_provider import * # pyright: ignore
from qiskit_aer.noise import NoiseModel as IBMNoiseModel # pyright: ignore
import json
import qiskit
import pandas as pd

import logging

logging.basicConfig(
    # filename="noise_models.log",
    level=logging.DEBUG,
    format="[%(levelname)s] %(message)s"
)


HARDWARE_SPECS_PATH = ""
PRECISION: int = 8
isclose_abstol = 1/(10**(PRECISION-1))
rel_tol = 1/(10**(PRECISION-1))

def my_isclose(val1_: Union[float, complex, int], val2_: Union[float, complex, int]) -> bool:
    if not isinstance(val1_, complex):
        val1 = complex(val1_, 0)
    else:
        val1 = val1_

    if not isinstance(val2_, complex):
        val2 = complex(val2_, 0)
    else:
        val2 = val2_
    return isclose(val1.real, val2.real, abs_tol=isclose_abstol, rel_tol=rel_tol) and isclose(val1.imag, val2.imag, abs_tol=isclose_abstol, rel_tol=rel_tol)

def my_round(val: Union[float, complex, int]):
    if isinstance(val, float):
        return round(val, PRECISION)
    assert isinstance(val, complex)
    return complex(round(val.real, PRECISION), round(val.imag, PRECISION))


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
        logging.critical("Could not retrieve backend {}".format(hardware_spec))
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
    logging.critical(f"No type matches with the current basis gates ({filtered_basis_gates})")
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
    logging.critial("Could not retrieve operator {}".format(op_))
    raise Exception("Could not retrieve operator", op_)


def are_matrices_equal(m1, m2) -> bool:
    if m1 is None:
        return m2 is None
    if m2 is None:
        return False
    assert m1.shape == m2.shape

    # Frobenius inner product
    c = np.vdot(m1, m2) / np.vdot(m1, m1)
    return my_isclose(np.linalg.norm(m2 - c * m1), 0)

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
            logging.error(f"controls are initialized in a non-multiqubit gate ({op} {control})")
            raise Exception(f"controls are initialized in a non-multiqubit gate ({op} {control})")
        elif is_multiqubit_gate(op) and control is None:
            logging.error(f"{op} gate should have exactly 1 control ({control}) qubit")
            raise Exception(f"{op} gate should have exactly 1 control ({control}) qubit")
        if target == control:
            logging.error("target is in controls")
            raise Exception("target is in controls")
        self.control = control
        self.name = name # pyright: ignore

        if not (self.params is None):
            if self.params is not None:
                for i in range(self.params.shape[0]):
                    for j in range(self.params.shape[1]):
                        self.params[i, j] = my_round(self.params[i, j])

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
        return self.target == value.target and self.control == value.control and self.op == value.op and are_matrices_equal(self.params, value.params) # pyright: ignore

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

    def to_custom(self):
        assert self.control is None
        if self.op == Op.CUSTOM:
            new_params = self.params
        elif self.op == Op.I:
            new_params = np.array([[complex(1, 0),complex(0,0)],
                                   [complex(0,0),complex(1,0)]])
        elif self.op == Op.X:
            new_params = np.array([[complex(0, 0), complex(1, 0)],
                                   [complex(1, 0), complex(0, 0)]])
        elif self.op == Op.Z:
            new_params = np.array([[complex(1, 0), complex(0, 0)],
                                   [complex(0, 0), complex(-1, 0)]])
        elif self.op == Op.Y:
            new_params = np.array([[complex(0, 0), complex(0, -1)],
                                   [complex(0, 1), complex(0, 0)]])
        else:
            assert False
        return Instruction(target=self.target, op=Op.CUSTOM, params=new_params)


    def __str__(self) -> str:
        if self.op == Op.CUSTOM:
            assert self.control is None
            return f"Instruction(target={self.target}, op={self.op}, params={self.params})"
        if self.control is None:
            return f"Instruction(target={self.target}, op={self.op})"
        return f"Instruction(target={self.target}, control={self.control}, op={self.op})"

    def __repr__(self) -> str:
        return self.__str__()

def is_instruction_in_list(l: List[Instruction], instruction: Instruction) -> bool:
    for old_ins in l:
        if old_ins == instruction:
            return True
    return False

def are_single_target_seqs_equal(seq1: List[Instruction], seq2: List[Instruction]) -> bool:
    target = None

    if len(seq1) != len(seq2):
        return False

    for i in range(len(seq1)):
        assert isinstance(seq1[i], Instruction)
        assert isinstance(seq2[i], Instruction)

        if target is None:
            target = seq1[i].target
        assert(seq1[i].target == seq2[i].target)
        assert(seq1[i].control is None)
        assert(seq2[i].control is None)

        if not (seq1[i] == seq2[i]):
            return False
    return True

def get_target_to_seq_dict(seq: List[Instruction]) -> Dict[int, List[Instruction]]:
    target_to_seq = dict()

    for i in range(len(seq)):
        assert isinstance(seq[i], Instruction)
        target = seq[i].target
        if target not in target_to_seq.keys():
            target_to_seq[target] = []
        if not is_identity_instruction(seq[i]):
            target_to_seq[target].append(seq[i])

    return target_to_seq

def are_seqs_equal(seq1, seq2) -> bool:
    target_to_seq1 = get_target_to_seq_dict(seq1)
    target_to_seq2 = get_target_to_seq_dict(seq2)

    if target_to_seq1.keys() != target_to_seq2.keys():
        return False

    for key in target_to_seq1.keys():
        if not are_single_target_seqs_equal(target_to_seq1[key], target_to_seq2[key]):
            return False
    return True

def does_sequence_exists(seqs, new_seq) -> int:
    for (index, seq) in enumerate(seqs):
        if are_seqs_equal(seq,new_seq):
            return index
    return -1


def is_zero_instruction(instruction: Instruction) -> bool:
    if instruction.op == Op.CUSTOM:
        for i in range(instruction.params.shape[0]):
            for j in range(instruction.params.shape[1]):
                if not my_isclose(instruction.params[i, j], 0):
                    return False
        return True
    return False

def is_zero_seq(seq) -> bool:
    if len(seq) > 1:
        return False

    for instruction in seq:
        if not is_zero_instruction(instruction):
            return False
    return True

def is_identity_instruction(instruction: Instruction) -> bool:
    if instruction.op == Op.CUSTOM:
        return instruction == Instruction(instruction.target, Op.I).to_custom()
    return instruction.op == Op.I

def is_identity_seq(seq: List[Instruction]) -> bool:
    for instruction in seq:
        if not is_identity_instruction(instruction):
            return False
    return True

def merge_instructions(instruction1_: Instruction, instruction2_: Instruction) -> Instruction:

    assert isinstance(instruction1_, Instruction)
    assert isinstance(instruction2_, Instruction)

    instruction1 = instruction1_.to_custom()
    instruction2 = instruction2_.to_custom()
    assert instruction1.target == instruction2.target
    assert instruction1.control is None
    assert instruction1.control == instruction2.control
    return Instruction(instruction1.target, Op.CUSTOM, params=np.dot(instruction1.params, instruction2.params))


def merge_seq_instruction(err_seq: List[Any], instruction: Instruction):
    assert isinstance(instruction, Instruction)
    assert not is_zero_instruction(instruction)
    if len(err_seq) == 0:
        return [instruction]

    assert not isinstance(err_seq[-1], KrausOperator)

    new_seq = []

    found = False
    for err in err_seq:
        assert isinstance(err, Instruction)
        if is_zero_instruction(err):
            return err_seq
        if err.target == instruction.target:
            assert not found
            found = True
            new_instruction = merge_instructions(err.to_custom(), instruction.to_custom())
            if is_zero_instruction(new_instruction):
                return [new_instruction]
            else:
                new_seq.append(new_instruction)
        else:
            new_seq.append(err)

    if not found:
        new_seq.append(instruction)
    return new_seq

class ComplexEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, complex):
            return {"real": obj.real, "imag": obj.imag}
        return json.JSONEncoder.default(self, obj)

class QuantumChannel:
    def __init__(self, target_instruction: Optional[Instruction] = None, all_ins_sequences=None, all_probabilities=None, target_qubits=None, flatten=True) -> None:
        self.target_instruction = target_instruction
        self.errors = [] # list of sequences of instructions/kraus operators
        self.probabilities = []
        for (index, seq) in enumerate(all_ins_sequences):
            prob = my_round(all_probabilities[index])
            if not my_isclose(prob, 0.0):
                new_seq = QuantumChannel.translate_err_sequence(seq, target_qubits)
                self.errors.append(new_seq)
                self.probabilities.append(prob)
        assert len(self.errors) == len(self.probabilities)
        self.normalize_probabilities()
        self.check_probabilities()
        if flatten:
            self.flatten() # gets rid of Kraus channels
            assert len(self.errors) == len(self.probabilities)

        if len(self.probabilities) == 0:
            self.probabilities = [1.0]
            self.errors = [[Instruction(target_qubits[0], Op.I)]]
        else:
            assert len(self.errors) > 0

    def __str__(self) -> str:
        return {"type": "QuantumChannel", "target_instruction": self.target_instruction,"errors": self.errors, "probs":self.probabilities}.__str__()

    def __repr__(self):
        return self.__str__()

    def check_probabilities(self):
        total_sum = sum(self.probabilities)
        if not my_isclose(total_sum, 1):

            logging.warning("Probabilities do not sum to 1. Got {total_sum}".format(total_sum=total_sum))

        for p in self.probabilities:
            assert 0.0 < p <= 1.0

    def normalize_probabilities(self):
        total_sum = sum(self.probabilities)

        for index in range(len(self.probabilities)):
            self.probabilities[index] /= total_sum

    def flatten_sequence(self, err_seq):
        sequences = []
        for err in err_seq:
            if isinstance(err, Instruction):
                if len(sequences) == 0:
                    if err.op == Op.RESET:
                        sequences.append([Instruction(err.target, Op.CUSTOM, params=np.array([[complex(1, 0),complex(1,0)], [complex(0,0),complex(0,0)]]))])
                    else:
                        if not is_zero_instruction(err):
                            sequences.append([err])
                else:
                    if err.op == Op.RESET:
                        all_seqs_temp = []
                        for seq in sequences:
                            matrix = np.array([[complex(1, 0), complex(1, 0)], [complex(0, 0), complex(0, 0)]])
                            temp_seq = deepcopy(seq)
                            curr_ins = Instruction(err.target, Op.CUSTOM, params=matrix)
                            temp_seq = merge_seq_instruction(temp_seq, curr_ins)
                            if not is_zero_seq(temp_seq):
                                all_seqs_temp.append(temp_seq)

                        sequences = all_seqs_temp
                    else:
                        temp_sequences = []
                        for (index, seq) in enumerate(sequences):
                            new_seq = merge_seq_instruction(seq, err)
                            if not is_zero_seq(new_seq):
                                temp_sequences.append(new_seq)
                        sequences = temp_sequences
            else:
                assert isinstance(err, KrausOperator)
                if len(sequences) == 0:
                    for curr_ins in err.operators:
                        if not is_zero_instruction(curr_ins):
                            sequences.append([curr_ins])
                else:
                    all_seqs_temp = []
                    for seq in sequences:
                        for curr_ins in err.operators:
                            temp_seq = deepcopy(seq)
                            temp_seq = merge_seq_instruction(temp_seq, curr_ins)
                            if not is_zero_seq(temp_seq):
                                all_seqs_temp.append(temp_seq)
                    sequences = all_seqs_temp
        return sequences


    def flatten(self):
        total_probabilities = sum(self.probabilities)

        if not my_isclose(total_probabilities, 1.0):
            logging.error("[QuantumChannel::flatten] probabilities should be 1, got {}".format(total_probabilities))
            raise Exception("[QuantumChannel::flatten] probabilities should be 1, got {}".format(total_probabilities))

        new_probabilities = []
        new_errors = []

        for (err_seq, prob) in zip(self.errors, self.probabilities):
            # logging.debug("\n\n")
            # logging.debug(f"err_seq (len={len(err_seq)}, prob={prob}):\n {err_seq}")
            flattened_sequences = self.flatten_sequence(err_seq)
            # logging.debug(f"flattened_sequences (len={len(flattened_sequences)}): {flattened_sequences}")
            for flattened_seq in flattened_sequences:
                assert (not is_zero_seq(flattened_seq)) and len(flattened_seq) > 0
                if not my_isclose(prob, 0.0):
                    index = does_sequence_exists(new_errors, flattened_seq)
                    if index == -1:
                        new_probabilities.append(prob)
                        new_errors.append(flattened_seq)
                    else:
                        new_probabilities[index] += prob
                        new_probabilities[index] = my_round(new_probabilities[index])

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
            'target_instruction': self.target_instruction.serialize(),
            'probabilities': [float(x) for x in self.probabilities],
            'errors': serialized_errors
        }

    def get_success_probability(self) -> float:
        prob = 0
        for (index, err) in enumerate(self.errors):
            if is_identity_seq(err):
                prob += self.probabilities[index]
        return prob

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
                    instruction = Instruction(target_qubit, op)
                    answer.append(instruction)
            elif err['name'] == 'kraus':
                assert len(err['qubits']) == 1
                target_qubit = target_qubits[err['qubits'][0]]
                kraus_op = KrausOperator(err['params'], target_qubit)
                if len(kraus_op.operators) == 1:
                    answer.append(kraus_op.operators[0])
                elif len(kraus_op.operators) > 1:
                    answer.append(kraus_op)
            else:
                op = get_op(err['name'])
                assert len(err['qubits']) == 1
                target_qubit = target_qubits[err['qubits'][0]]
                instruction = Instruction(target_qubit, op)
                answer.append(instruction)
        return answer

class KrausOperator:
    def __init__(self, operators, qubit, factorize=False) -> None:
        self.operators = []
        self.coefficients = []
        self.target = qubit
        self.simplified_ops = []
        self.factorize = factorize

        for operator in operators:
            assert operator.shape == (2,2) # for now we are dealing only with single qubit operators
            instruction = Instruction(qubit, Op.CUSTOM, params=operator)
            coeff = self.get_coefficient(instruction.params)
            if (not is_zero_instruction(instruction)):
                if not is_instruction_in_list(self.operators, instruction):
                    if factorize:
                        self.operators.append(Instruction(qubit, Op.CUSTOM, params=operator/coeff))
                    else:
                        self.operators.append(instruction)
                    self.coefficients.append(coeff)
                    self.simplified_ops.append(self.simplify_op(instruction.params))

        assert len(self.operators) == len(self.coefficients)
        assert len(self.operators) == len(self.simplified_ops)

    def get_coefficient(self, operator) -> float:
        coefficient = None
        assert operator.shape == (2, 2)

        for i in range(operator.shape[0]):
            for j in range(operator.shape[1]):
                if not my_isclose(operator[i, j], 0):
                    if coefficient is None:
                        coefficient = abs(operator[i, j])
                        # print("coeff found:", coefficient)
                    else:
                        if not my_isclose(abs(operator[i, j]), coefficient):
                            # print("coeffs differ:", coefficient, abs(operator[i, j]))
                            return 1

        if coefficient is None:
            return 1
        return coefficient

    def simplify_op(self, operator):
        if are_matrices_equal(operator, Instruction(0, Op.I).to_custom().params):
            return "I"
        if are_matrices_equal(operator, Instruction(0, Op.X).to_custom().params):
            return "X"
        if are_matrices_equal(operator, Instruction(0, Op.Z).to_custom().params):
            return "Z"
        if are_matrices_equal(operator, Instruction(0, Op.Y).to_custom().params):
            return "Y"

        if are_matrices_equal(operator, np.array([[complex(0, 0),complex(1,0)],
                                   [complex(0,0),complex(0,0)]])):
            return "|0><1|"

        if are_matrices_equal(operator, np.array([[complex(0, 0),complex(0,0)],
                                   [complex(1,0),complex(0,0)]])):
            return "|1><0|"

        return operator



    def apply_coefficients(self):
        assert len(self.operators) == len(self.coefficients)
        for index in range(len(self.operators)):
            self.operators[index] = Instruction(self.operators[index].target, op=Op.CUSTOM, params=self.operators[index].params/self.coefficients[index])

    def __str__(self) -> str:
        return f"KrausOp[{len(self.operators)}]({self.operators})"

    def print_ops(self):
        for (coeff, op) in zip(self.coefficients, self.simplified_ops):
            print(coeff, op)

    def __repr__(self) -> str:
        return self.__str__()

class MeasChannel:
    def __init__(self, target_instruction: Instruction, all_probabilities) -> None:
        self.target_instruction = target_instruction
        assert len(all_probabilities) == 2
        self.meas_errors = dict()

        zero_meas_err = all_probabilities[0]
        assert len(zero_meas_err) == 2
        self.meas_errors[0] = dict()
        self.meas_errors[0][0] = my_round(zero_meas_err[0]) # probability that measurement outcome is 0 given that the ideal outcome should have been 0
        self.meas_errors[0][1] = my_round(zero_meas_err[1]) # probability that measurement outcome is 1 given that the ideal outcome should have been 0

        one_meas_err = all_probabilities[1]
        assert len(one_meas_err) == 2
        self.meas_errors[1] = dict()
        self.meas_errors[1][0] = my_round(one_meas_err[0]) # probability that measurement outcome is 0 given that the ideal outcome should have been 1
        self.meas_errors[1][1] = my_round(one_meas_err[1]) # probability that measurement outcome is 1 given that the ideal outcome should have been 1
        self.check_probabilities()

    def check_probabilities(self) -> None:
        for ideal_outcome in range(2):
            prob = 0
            for noisy_outcome in range(2):
                prob += self.get_ind_probability(ideal_outcome, noisy_outcome)
            if not my_isclose(prob, 1):
                logging.error(f"measurement error channel for instruction {self.target_instruction.name} probabilities for ideal outcome {ideal_outcome}: 1 != {prob}")
                raise Exception("Meas. channel probabilities dont add up to 1")


    def get_success_probability(self):
        return self.get_ind_probability(0,0) + self.get_ind_probability(1,1)

    def get_ind_probability(self, ideal_outcome: int, noisy_outcome: int):
        assert ideal_outcome in [0, 1]
        assert noisy_outcome in [0, 1]
        return self.meas_errors[ideal_outcome][noisy_outcome]

    def serialize(self):
        result = deepcopy(self.meas_errors)
        result["target_instruction"] = self.target_instruction.serialize()
        return result

    def __str__(self) -> str:
        return {"type": "MeasChannel", "errors": self.meas_errors}.__str__()

    def __repr__(self):
        return self.__str__()


class NoiseModel:
    hardware_spec: HardwareSpec
    basis_gates: List[Op]
    instructions_to_channel: Dict[Instruction, QuantumChannel|MeasChannel]
    num_qubits: int
    flatten: bool
    ibm_noise_model: IBMNoiseModel

    def load_noise_model(self, thermal_relaxation):
        ibm_noise_model = get_ibm_noise_model(self.hardware_spec, thermal_relaxation=thermal_relaxation)
        assert isinstance(ibm_noise_model, IBMNoiseModel)

        self.ibm_noise_model = ibm_noise_model

        self.basis_gates = get_basis_gate_type([get_op(op) for op in ibm_noise_model.basis_gates])
        self.num_qubits = len(ibm_noise_model.noise_qubits)
        logging.debug(f"basis_gates={self.basis_gates} num_qubits={self.num_qubits}")

        # start translating quantum channels
        self.instructions_to_channel = dict()
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
            else:
                assert len(error_target_qubits) == 1
                target = error_target_qubits[0]
                target_qubits = [target]

            target_instruction = Instruction(target, op, control)
            if target_instruction.op != Op.I:
                probabilities = error['probabilities']
                if error['type'] == "qerror":
                    error_instructions = error['instructions']
                    self.instructions_to_channel[target_instruction] = QuantumChannel(target_instruction, error_instructions, probabilities, target_qubits, flatten=self.flatten)
                else:
                    assert error['type'] == "roerror"
                    self.instructions_to_channel[target_instruction] = MeasChannel(target_instruction, probabilities)

    def __init__(self, hardware_specification: HardwareSpec=None, thermal_relaxation=True, flatten=True) -> None:
        self.hardware_spec = hardware_specification
        self.thermal_relaxation = thermal_relaxation
        self.flatten = flatten
        logging.debug(f"{self.hardware_spec} thermal_relaxation: {self.thermal_relaxation}")
        if hardware_specification is not None:
            self.load_noise_model(thermal_relaxation=thermal_relaxation)
        else:
            self.instructions_to_channel = dict()
            self.num_qubits = None
            self.basis_gates = []

    def get_success_probability(self, instruction: Instruction):
        return self.instructions_to_channel[instruction].get_identity_probability()

    def serialize(self):
        instructions = []
        channels = []
        for (instruction, channel) in self.instructions_to_channel.items():
            instructions.append(instruction.serialize())
            channels.append(channel.serialize())

        assert len(instructions) == len(channels)
        return {
            'name': self.hardware_spec.value,
            "thermalization":  1 if self.thermal_relaxation else 0,
            "num_qubits": self.num_qubits,
            "basis_gates_type": str(self.basis_gates.name),
            'basis_gates': [str(x.name) for x in self.basis_gates.value],
            'instructions': instructions,
            'channels': channels
        }

result = []
result_complete = []

def dump_hardware_spec(hardware_spec: HardwareSpec, with_thermalization: bool):
    noise_model = NoiseModel(hardware_spec, thermal_relaxation=with_thermalization)
    f = open(f"{HARDWARE_SPECS_PATH}{'with_thermalization' if with_thermalization else 'no_thermalization'}/{hardware_spec.value}.json", "w")
    json.dump(noise_model.serialize(), f, cls=ComplexEncoder, indent=4)
    f.close()

    if with_thermalization:
        instr_to_succ_probs = dict()
        for (instruction, channel) in noise_model.instructions_to_channel.items():
            if instruction.op not in instr_to_succ_probs.keys():
                instr_to_succ_probs[instruction.op] = []
            instr_to_succ_probs[instruction.op].append(channel.get_success_probability())
            result_complete.append({
                "hardware_spec": hardware_spec.value,
                "op": instruction.op,
                "target": instruction.target,
                "control": -1 if (instruction.control is None) else instruction.control,
                "success": channel.get_success_probability(),
                "num_errors": len(channel.probabilities) if isinstance(channel, QuantumChannel) else 2
            })

        for (op, v_probs) in instr_to_succ_probs.items():
            result.append({
                "hardware_spec": hardware_spec.value,
                "op": op,
                "avg_success": sum(v_probs) / len(v_probs),
                "num_errors": len(v_probs)
            })

if __name__ == "__main__":
    print(qiskit.__version__)
    for hardware_spec in HardwareSpec:
        logging.info(f"dumping {hardware_spec.value}")
        dump_hardware_spec(hardware_spec, True)
        dump_hardware_spec(hardware_spec, False)

    df = pd.DataFrame(result)
    df.to_csv("avg_success.csv", index=False)

    df_complete = pd.DataFrame(result_complete)
    df_complete.to_csv("all_success.csv", index=False)

