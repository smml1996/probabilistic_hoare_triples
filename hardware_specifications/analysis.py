from typing import Tuple

from ibm_noise_models import *
from qiskit.quantum_info import Kraus
from qiskit_aer.noise import QuantumError


def ibm_get_minimal_cptp(error: QuantumError) -> Kraus:
    channel = error.to_quantumchannel()
    return Kraus(channel)

def get_ibm_op_str(instruction: Instruction) -> str:
    assert isinstance(instruction, Instruction)
    return instruction.op.value.lower()

def ibm_get_instruction_noise(noise_model: Union[IBMNoiseModel, NoiseModel], instruction: Instruction):
    if isinstance(noise_model, NoiseModel):
        local_quantum_errors = noise_model.ibm_noise_model._local_quantum_errors
    else:
        assert isinstance(noise_model, IBMNoiseModel)
        local_quantum_errors = noise_model._local_quantum_errors
    if instruction.control is None:
        q_tuple = (instruction.target,)
    else:
        q_tuple = (instruction.control, instruction.target)

    op_str = get_ibm_op_str(instruction)
    error = local_quantum_errors[op_str][q_tuple]
    return error

def my_get_quantum_channel(error: QuantumError, only_kraus=False) -> List[Tuple[List[Union[str, KrausOperator]], float]]:
    result = []
    for prob, circ in zip(error.probabilities, error.circuits):
        list_error = []
        for ci in circ.data:
            inst = ci.operation
            if inst.name == "kraus":
                list_error.append(KrausOperator(inst.params, 0, factorize=True))
            elif not only_kraus:
                list_error.append(inst)

        result.append((list_error, prob))
    return result

def get_all_kraus_operators(hardware: HardwareSpec):


if __name__ == "__main__":
    hardware = HardwareSpec.ALGIERS

















