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

      --run arg       can be any of the following: 
                             lbell, lphase, ipma, ipma2, cxh, 
                             ghz, reset, setup.
      --custom_name arg      a directory will be created with this name in 
                             results/. (default: "")
      --hardware arg         Comma-separated list of hardware specs. Check 
                             hardware_specifications/ directory. E.g. 
                             almaden (default: "")
      --round_in_file arg    All numbers in the generated files will be 
                             formatted to show no more than this number of 
                             decimal places. (default: 5)
  -h, --help                 Print usage
```





