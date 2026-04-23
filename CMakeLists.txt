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

# Tell CMake where to find OR-Tools
find_package(ortools REQUIRED)


# Source files (your code + generated code)
set(SOURCES
    main.cpp
        src/hull.cpp
        src/Belief.cpp
        src/experiments.cpp
        src/solvers.cpp
        src/utils.cpp
        src/pomdp.cpp
)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE CGAL::CGAL)
target_link_directories(main PRIVATE /opt/homebrew/lib)
target_link_libraries(main PRIVATE
        ortools::ortools
        c++      # libc++
)

# Download and build GoogleTest automatically
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
## For Windows/MSVC only: avoid overriding parent project's compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)


## test utils
#add_executable(test_utils tests/test_utils.cpp src/utils.cpp)
#target_link_libraries(test_utils GTest::gtest_main ortools::ortools
#        nlohmann_json::nlohmann_json
#        c++ )
#gtest_discover_tests(test_utils)


## test POMDPs
#add_executable(test_pomdp tests/test_pomdp.cpp
#        src/pomdp.cpp
#        src/Belief.cpp
#        src/hardware_specification.cpp
#        src/channels.cpp
#        src/states.cpp
#        src/quantum_gates.cpp
#        src/instruction.cpp
#        src/floats.cpp
#        src/rationals.cpp
#        src/utils.cpp)
#target_link_libraries(test_pomdp GTest::gtest_main nlohmann_json::nlohmann_json)
#gtest_discover_tests(test_pomdp)
#
## test beliefs
#add_executable(test_beliefs tests/test_beliefs.cpp
#        src/pomdp.cpp
#        src/Belief.cpp
#        src/hardware_specification.cpp
#        src/channels.cpp
#        src/states.cpp
#        src/quantum_gates.cpp
#        src/instruction.cpp
#        src/floats.cpp
#        src/rationals.cpp
#        src/utils.cpp)
#target_link_libraries(test_beliefs GTest::gtest_main nlohmann_json::nlohmann_json)
#gtest_discover_tests(test_beliefs)


