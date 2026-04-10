# Noise-Aware Quantum Program Synthesis and Verification

## Installation
Boost, ortools, nlohmann_json, antlr4-runtime, z3, googletest

```shell
cmake -S . -B build
cmake --build build
```

## Usage
```textmate
❯ ./main --help
Synthesize quantum algorithms using POMDPs
Usage:
  main [OPTION...]

      --experiment arg       can be any of the following: 
                             basis_state_discr, bell_state_discr_ipma2, 
                             bell_state_discr_ipma3, bitflip_ipma, 
                             bitflip_ipma2, bitflip_ipma3, bitflip_cxh, 
                             ghz3, ghz4, reset, basis_state_discr.
      --custom_name arg      a directory will be created with this name in 
                             results/. (default: "")
      --method arg           can be any of the following: bellman, pbvi, 
                             convex.
      --hardware arg         Comma-separated list of hardware specs. Check 
                             hardware_specifications/ directory. E.g. 
                             almaden (default: "")
      --min_horizon arg      Minimum horizon
      --max_horizon arg      Maximum horizon
      --precision arg        precision of the POMDP (default: 8)
      --with_thermalization  Enable thermalization
      --round_in_file arg    All numbers in the generated files will be 
                             formatted to show no more than this number of 
                             decimal places. (default: 5)
  -h, --help                 Print usage
```
## Basis States discrimination (1 qubit)

## Reset Problem

## GHZ state preparation

## Parity bitflip problem
### Single Distribution case (--method bellman, --method pbvi)
The precondition consists of an initial distribution that assigns uniform probability to all Bell states.
$$
\begin{split}
 \phi_{pre} :=\mathbb{P}([q_0, q_1]=|\Phi^+⟩) &= 0.25 \land 
\mathbb{P}([q_0, q_1]=|\Phi^-⟩) = 0.25 \land \mathbb{P}([q_0, q_1]=|\Psi^+⟩) = 0.25 \land \\
\mathbb{P}([q_0, q_1]&=|\Psi^-⟩) = 0.25 \land\mathbb{P}([q_2]=|0⟩) = 1.0  \land \mathbb{P}(\overrightarrow{c}= \overrightarrow{0}) = 1.0 .
\end{split}
$$

The post-condition specifies that we are interested in reaching a Bell state with even parity.
$$
\begin{split}
\phi_{post} :=\mathbb{P}([q_0, q_1]=|\Phi^+⟩ \lor [q_0, q_1]=|\Psi^+⟩) = 1.
\end{split}
$$

### Convex distribution case (--method convex)
The precondition specifies that we can start in any distribution over the four Bell states.
$$
\begin{split}
\phi_{pre} :=\mathbb{P}([q_0, q_1]=|\Phi^+⟩ \lor [q_0, q_1]=|\Phi^-⟩ \lor [q_0, q_1]=|\Psi^+⟩ \lor [q_0, q_1]=|\Psi^-⟩) +  = 1 \\
 \land \mathbb{P}(\overrightarrow{c}= \overrightarrow{0}) = 1.0 .
\end{split}
$$

The post-condition specifies that we are interested in reaching a Bell state with even parity.
$$
\begin{split}
\phi_{post} :=\mathbb{P}([q_0, q_1]=|\Phi^+⟩ \lor [q_0, q_1]=|\Psi^+⟩) = 1.
\end{split}
$$

### Instruction sets used
- Use flag `--experiment bitflip_ipma`
$$
\{CX02, CX12, X0, P2\} 
$$
- Use flag `--experiment bitflip_ipma2` (CX ~ CX02;CX02)
$$
\{X0, P2, CX\} 
$$
- Use flag `--experiment bitflip_ipma3`
$$
\{X0, P2, CX, X2\}
$$
- Use flag `--experiment bitflip_cxh`
 $$
 \{H2, H1, CX21, CX01, P2\}
 $$

## Bell State discrimination (`--method convex`)

$$
\begin{split}
\phi_{pre} :=\mathbb{P}([q_0,q_1] = [q_2,q_3])=1 \land\\\mathbb{P}([q_0, q_1]=|\Phi^+⟩ \lor [q_0, q_1]=|\Phi^-⟩ \lor [q_0, q_1]=|\Psi^+⟩ \lor [q_0, q_1]=|\Psi^-⟩)  = 1 \\
\land \mathbb{P}(\overrightarrow{c}= \overrightarrow{0}) = 1.0 .
\end{split}
$$

$$
\begin{split}
\phi_{pre} :=\mathbb{P}(([q_2, q_3]=|\Phi^+⟩ \land \overrightarrow{c}= \overrightarrow{0})\lor\\ 
([q_0, q_1]=|\Phi^-⟩\land \overrightarrow{c}= \overrightarrow{1}) \lor \\
([q_0, q_1]=|\Phi^-⟩ \land \overrightarrow{c}= \overrightarrow{2}) \lor \\
([q_0, q_1]&=|\Psi^-⟩ \land \overrightarrow{c}= \overrightarrow{3})) = 1
\end{split}
$$

### Instruction sets used
- Use flag `--experiment basis_state_discr` to use ipma instruction set
- Use flag `--experiment basis_state_discr2` to use ipma2 instruction set
- Use flag `--experiment basis_state_discr3` to use ipma3 instruction set





