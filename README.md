# Multi-environments POMDPs artifact
The benchmarks are all located in the folder ```AB-HSVI_NeurIPS_2025/Models/```.

## Dominant points algorithm
### Installation
The code is written in C++17 and uses ```Boost, ortools, CGAL```.

```shell
cmake -S . -B build
cmake --build build
```

### Usage
On the build directory you can run any benchmark by executing

```shell
\main --command run --name benchmark_name --max_horizon 6
```
where `benchmark_name` is any of the files in the directory ```AB-HSVI_NeurIPS_2025/Models/```, and the parameter 
`--max_horizon` specifies that the benchmark will be tested for horizons 1 through 6. 
This command generates the csv file `results/f1_benchmark_name.csv`. 

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
On the folder ```AB-HSVI_NeurIPS_2025/``` you can reproduce the experiments. Just execute
```shell
python run_test benchmark_name
```
Results are written in an output file located in the directory ```AB-HSVI_NeurIPS_2025/my_results/```.
 
## Other
- The additional benchmarks for Rock Sample are generated using the file  ```Generate_RockSample_POMDP.py```. This generates the files:
  - [RockSample_POMDP_N3_G1_K3_R4_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G1_K3_R4_.txt)
  - [RockSample_POMDP_N3_G2_K3_R5_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K3_R5_.txt)
  - [RockSample_POMDP_N3_G2_K5_R7_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K5_R7_.txt)
  - [RockSample_POMDP_N3_G2_K7_R9_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G2_K7_R9_.txt)
  - [RockSample_POMDP_N3_G3_K3_R6_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G3_K3_R6_.txt)
  - [RockSample_POMDP_N3_G3_K5_R8_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G3_K5_R8_.txt)
  - [RockSample_POMDP_N3_G4_K5_R9_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G4_K5_R9_.txt)
  - [RockSample_POMDP_N3_G4_K7_R11_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G4_K7_R11_.txt)
  - [RockSample_POMDP_N3_G6_K7_R13_.txt](AB-HSVI_NeurIPS_2025/Models/RockSample_POMDP_N3_G6_K7_R13_.txt)


- the folder `python_code` contains the code to generate the robot navigation and the airplane identification benchmarks
- the file `paper_utils.py` can be executed (`python paper_utils.py`) to generate the csv files used for the experiments section of our paper.






