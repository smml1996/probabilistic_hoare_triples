cmake_minimum_required(VERSION 3.18)
project(probabilistc_hoare_triples)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

add_compile_options(-stdlib=libc++)
add_link_options(-stdlib=libc++)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# for convexifying pareto front
find_package(CGAL REQUIRED)

# for linear programming
find_package(ortools REQUIRED)

# json files
find_package(nlohmann_json REQUIRED)


# Source files
set(SOURCES
    main.cpp
        src/Belief.cpp
        src/channels.cpp
        src/experiments.cpp
        src/hardware_specification.cpp
        src/hull.cpp
        src/hybrid_state.cpp
        src/instruction.cpp
        src/pomdp.cpp
        src/quantum_gates.cpp
        src/solvers.cpp
        src/utils.cpp
        src/experiments/bv.cpp
        src/experiments/discrimination.cpp
        src/experiments/qec.cpp
        src/experiments/reset.cpp
        src/experiments/rus.cpp
        src/experiments/superdense_coding.cpp
)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE CGAL::CGAL)
target_link_directories(main PRIVATE /opt/homebrew/lib)
target_link_libraries(main PRIVATE
        ortools::ortools
        c++      # libc++
        CGAL::CGAL
        nlohmann_json::nlohmann_json
)


## TESTING
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
## For Windows/MSVC only: avoid overriding parent project's compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)


# test instructions
add_executable(test_instructions tests/test_instructions.cpp src/instruction.cpp src/quantum_gates.cpp src/utils.cpp)
target_link_libraries(test_instructions GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(test_instructions)

add_executable(test_qhardware tests/test_qhardware.cpp
        src/hardware_specification.cpp
        src/channels.cpp
        src/hybrid_state.cpp
        src/quantum_gates.cpp
        src/instruction.cpp
        src/utils.cpp)
target_link_libraries(test_qhardware GTest::gtest_main nlohmann_json::nlohmann_json)
gtest_discover_tests(test_qhardware)

#add_executable(test_hull tests/test_hull.cpp
#        src/hull.cpp
#        src/pomdp.cpp
#        src/utils.cpp)
#target_include_directories(test_hull PRIVATE CGAL::CGAL)
#target_link_directories(test_hull PRIVATE /opt/homebrew/lib)
#target_link_libraries(test_hull GTest::gtest_main ortools::ortools nlohmann_json::nlohmann_json CGAL::CGAL)
#gtest_discover_tests(test_hull)

add_executable(test_reset_experiment tests/test_reset_experiment.cpp
        src/solvers.cpp
        src/hull.cpp
        src/experiments.cpp
        src/experiments/reset.cpp
        src/pomdp.cpp
        src/Belief.cpp
        src/hardware_specification.cpp
        src/channels.cpp
        src/hybrid_state.cpp
        src/quantum_gates.cpp
        src/instruction.cpp
        src/utils.cpp)
target_include_directories(test_reset_experiment PRIVATE CGAL::CGAL)
target_link_directories(test_reset_experiment PRIVATE /opt/homebrew/lib)
target_link_libraries(test_reset_experiment GTest::gtest_main ortools::ortools nlohmann_json::nlohmann_json CGAL::CGAL)
gtest_discover_tests(test_reset_experiment)
