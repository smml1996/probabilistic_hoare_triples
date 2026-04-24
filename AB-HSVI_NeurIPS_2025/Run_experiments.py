import AB_HSVI
import AB_HSVI_RockSample_experts

if __name__ == "__main__":
    # Tests for table 1:
    tests = ["Birds_MOPOMDP_S3_N3_A3_R20",
             "Birds_POMEMDP_S3_N3_A3_R20",
             "Birds_MEPOMDP_S3_N3_A3_R20",
             "Birds_MOPOMDP_S3_N3_A4_R55",
             "Birds_POMEMDP_S3_N3_A4_R55",
             "Birds_MEPOMDP_S3_N3_A4_R55",
             "Birds_MOPOMDP_S3_N3_A5_R1",
             "Birds_POMEMDP_S3_N3_A5_R1",
             "Birds_MEPOMDP_S3_N3_A5_R1",
             "Birds_MOPOMDP_S3_N4_A3_R42",
             "Birds_POMEMDP_S3_N4_A3_R42",
             "Birds_MEPOMDP_S3_N4_A3_R42",
             "Birds_MOPOMDP_S3_N5_A3_R6",
             "Birds_POMEMDP_S3_N5_A3_R6",
             "Birds_MEPOMDP_S3_N5_A3_R6",
             "Birds_MOPOMDP_S4_N3_A3_R21",
             "Birds_POMEMDP_S4_N3_A3_R21",
             "Birds_MEPOMDP_S4_N3_A3_R21",
             "Birds_MOPOMDP_S5_N3_A3_R11",
             "Birds_POMEMDP_S5_N3_A3_R11",
             "Birds_MEPOMDP_S5_N3_A3_R11"
                ]
    for test in tests:
        print(f"\nStarting test {test} for AB-HSVI:\n")
        AB_HSVI.AB_HSVI(f"Models/{test}.txt",0.95,0.5,f"Results/{test}.txt")
    
    # Tests for table 4:
    # tests = ["RockSample_MEPOMDP_N3_G1_K2_R18",
    #          "RockSample_MEPOMDP_N3_G1_K3_R13",
    #          "RockSample_MEPOMDP_N3_G1_K4_R83",
    #          "RockSample_MEPOMDP_N3_G1_K5_R78",
    #          "RockSample_MEPOMDP_N3_G2_K3_R4",
    #          "RockSample_MEPOMDP_N3_G2_K4_R72",
    #          "RockSample_MEPOMDP_N4_G1_K2_R33",
    #          "RockSample_MEPOMDP_N5_G1_K2_R96",
    #          "RockSample_MEPOMDP_N6_G1_K2_R77",
    #          "RockSample_MEPOMDP_N7_G1_K2_R10"
    #             ]
    # for test in tests:
    #     print(f"\nStarting test {test} for AB-HSVI:\n")
    #     AB_HSVI.AB_HSVI(f"Models/{test}.txt",0.95,1,f"Results/{test}.txt")

    # Tests for table 5 (partial overlap with tests for table 2):
    # tests = ["RockSample_MEPOMDP_N3_G1_K2_R18",
    #          "RockSample_POMDP_N3_G1_K2_R18",
    #          "RockSample_MEPOMDP_N3_G1_K3_R13",
    #          "RockSample_POMDP_N3_G1_K3_R13",
    #          "RockSample_MEPOMDP_N3_G1_K4_R83",
    #          "RockSample_POMDP_N3_G1_K4_R83",
    #          "RockSample_MEPOMDP_N3_G2_K3_R4",
    #          "RockSample_POMDP_N3_G2_K3_R4",
    #          "RockSample_MEPOMDP_N4_G1_K2_R33",
    #          "RockSample_POMDP_N4_G1_K2_R33",
    #          "RockSample_MEPOMDP_N5_G1_K2_R96",
    #          "RockSample_POMDP_N5_G1_K2_R96",
    #          "RockSample_MEPOMDP_N6_G1_K2_R77",
    #          "RockSample_POMDP_N6_G1_K2_R77"
    #             ]
    # for test in tests:
    #     print(f"\nStarting test {test} for AB-HSVI:\n")
    #     AB_HSVI.AB_HSVI(f"Models/{test}.txt",0.95,1,f"Results/{test}.txt")

    # Tests for table 6  (partial overlap with tests for table 2):
    # tests = ["RockSample_MEPOMDP_N3_G1_K2_R18",
    #          "RockSample_MEPOMDP_N3_G1_K3_R13",
    #          "RockSample_MEPOMDP_N3_G1_K4_R83",
    #          "RockSample_MEPOMDP_N3_G2_K3_R4",
    #          "RockSample_MEPOMDP_N4_G1_K2_R33",
    #          "RockSample_MEPOMDP_N5_G1_K2_R96",
    #          "RockSample_MEPOMDP_N6_G1_K2_R77"
    #             ]
    # for test in tests:
    #     print(f"\nStarting test {test} for AB-HSVI:\n")
    #     AB_HSVI.AB_HSVI(f"Models/{test}.txt",0.95,1,f"Results/{test}.txt")
    # AB_HSVI_RockSample_experts.experts_comp(3,1,2,18)
    # AB_HSVI_RockSample_experts.experts_comp(3,1,3,13)
    # AB_HSVI_RockSample_experts.experts_comp(3,1,4,83)
    # AB_HSVI_RockSample_experts.experts_comp(3,2,3,4)
    # AB_HSVI_RockSample_experts.experts_comp(4,1,2,33)
    # AB_HSVI_RockSample_experts.experts_comp(5,1,2,96)
    # AB_HSVI_RockSample_experts.experts_comp(6,1,2,77)

    # Test for table 2/7
    # tests = ["RockSample_MEPOMDP_2corners_N2_G1",
    #          "RockSample_MEPOMDP_2corners_close_N3_G1",
    #          "RockSample_MEPOMDP_2corners_close_N4_G1",
    #          "RockSample_MEPOMDP_2corners_close_N5_G1",
    #          "RockSample_MEPOMDP_2corners_close_N6_G1",
    #          "RockSample_MEPOMDP_2corners_close_N7_G1",
    #          "RockSample_MEPOMDP_2corners_N3_G1",
    #          "RockSample_MEPOMDP_2corners_N4_G1",
    #          "RockSample_MEPOMDP_2corners_N5_G1",
    #          "RockSample_MEPOMDP_corners_N2_G1",
    #          "RockSample_MEPOMDP_corners_close_N3_G1",
    #          "RockSample_MEPOMDP_corners_close_N4_G1",
    #          "RockSample_MEPOMDP_corners_close_N5_G1",
    #          "RockSample_MEPOMDP_corners_close_N6_G1",
    #          "RockSample_MEPOMDP_corners_close_N7_G1",
    #          "RockSample_MEPOMDP_corners_N3_G1",
    #          "RockSample_MEPOMDP_corners_N4_G1",
    #          "RockSample_MEPOMDP_corners_N5_G1",
    #          "RockSample_MEPOMDP_corners_N2_G2",
    #          "RockSample_MEPOMDP_corners_close_N3_G2",
    #          "RockSample_MEPOMDP_corners_close_N4_G2",
    #          "RockSample_MEPOMDP_corners_close_N5_G2",
    #          "RockSample_MEPOMDP_corners_N3_G2",
    #          "RockSample_MEPOMDP_corners_N4_G2",
    #             ]
    # for test in tests:
    #     print(f"\nStarting test {test} for AB-HSVI:\n")
    #     AB_HSVI.AB_HSVI(f"Models/{test}.txt",0.95,1,f"Results/{test}.txt")

    # Triviality test for random generation Bird problem instances
    # results = []
    # for r in range(100):
    #     test = f"Birds_MEPOMDP_S3_N3_A3_R{r}"
    #     print(f"\nStarting test {test}:")
    #     results.append(AB_HSVI(f"Models/Triviality_test/{test}.txt",0.95,0.5,f"Results/{test}.txt"))
    # for r in range(100):
    #     print(f"MEPOMDP {r}: {results[r]}")
    # print(f"Number of trival models: {sum(results)}")