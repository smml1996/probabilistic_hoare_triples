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
        src/hull.cpp
        src/Belief.cpp
        src/experiments.cpp
        src/solvers.cpp
        src/utils.cpp
        src/pomdp.cpp
        src/channels.cpp
        src/hardware_specification.cpp
        src/instruction.cpp
        src/quantum_gates.cpp
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


