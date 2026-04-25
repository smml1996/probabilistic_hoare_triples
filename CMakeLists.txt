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


# Source files
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
        CGAL::CGAL
)


# TESTING
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)


## tests
add_executable(tests tests/tests.cpp
        src/hull.cpp
        src/Belief.cpp
        src/experiments.cpp
        src/solvers.cpp
        src/utils.cpp
        src/pomdp.cpp
)
target_link_libraries(tests GTest::gtest_main)
target_link_libraries(tests
        ortools::ortools
        c++      # libc++
        CGAL::CGAL)
gtest_discover_tests(tests)


