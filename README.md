# Multi-environments POMDPs artifact
We consider 3 benchmarks:
1. **ABHSVI benchmark:** We consider all the POMDPs available in the [artifact](https://zenodo.org/records/17425571) of the [AB-HSVI paper](https://openreview.net/pdf?id=kcmw3OH5bh).
2. **F1 benchmark**: The POMDPs of [Cassandra thesis](https://dl.acm.org/doi/10.5555/926710) of the Robot navigation Problem and can be downloaded at https://www.pomdp.org/examples/index.html.
3. **More_rocks bencharmark**: We generated additional POMDPs using the code available in [artifact](https://zenodo.org/records/17425571) of the [AB-HSVI paper](https://openreview.net/pdf?id=kcmw3OH5bh).

The benchmarks are all located in the folder ```AB-HSVI_NeurIPS_2025/Models/```.

## Dominant points algorithm
### Installation
The code is written in C++17 and uses ```Boost, ortools, CGAL```.

```shell
cmake -S . -B build
cmake --build build
```

### Usage
On the build directory execute

| command | output file    | other |
|------|----------------| ------|
|./main --command abhsvi| results/abhsvi.csv | (no convexification)|
| ./main --command f1| results/f1.csv | (no convexification)|
|./main --command more_rocks| results/more_rocks.csv |(no convexification)|




## AB-HSVI
### Installation
This artifact contains Python code to run the AB-AHSVI algorithm (originally downloaded from 
https://zenodo.org/records/17425571 and adapted to finite horizon). We used Python3.13.
It requires 
```shell
pip install scipy gurobi
```

Additionally, Jupyter notebooks and extra code used for analysis might require
```shell
pip install pandas seaborn
```

### Usage
On the folder ```AB-HSVI_NeurIPS_2025/``` you can execute the following files to reproduce the experiments. 
Results are written in the corresponding output file located in the directory ```AB-HSVI_NeurIPS_2025/my_results/```.

| file             | output file    |
|------------------|----------------|
| experiments_abhsvi.py | results_abhsvi.csv |
| experiments_f1.py | results_f1.csv |
| more_rocks.py    | results_more_rocks.csv|
 
- The more_rocks benchmark can be generated using the file ```Generate_RockSample_POMDP.py```. This generates the files:
  - [RockSample_POMDP_N3_G1_K3_R4_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G1_K3_R4_.txt)
  - [RockSample_POMDP_N3_G2_K3_R5_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K3_R5_.txt)
  - [RockSample_POMDP_N3_G2_K5_R7_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K5_R7_.txt)
  - [RockSample_POMDP_N3_G2_K7_R9_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K7_R9_.txt)
  - [RockSample_POMDP_N3_G3_K3_R6_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G3_K3_R6_.txt)
  - [RockSample_POMDP_N3_G3_K5_R8_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G3_K5_R8_.txt)
  - [RockSample_POMDP_N3_G4_K5_R9_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G4_K5_R9_.txt)
  - [RockSample_POMDP_N3_G4_K7_R11_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G4_K7_R11_.txt)
  - [RockSample_POMDP_N3_G6_K7_R13_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G6_K7_R13_.txt)

- The Jupyter notebook ```analysis.ipynb``` contains analysis of the results.






