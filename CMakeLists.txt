cmake_minimum_required(VERSION 3.10)
project(probabilistc_hoare_triples)
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${ANTLR4_RUNTIME_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/grammars/assertions
    ${CMAKE_CURRENT_SOURCE_DIR}
    /opt/homebrew/Cellar/antlr4-cpp-runtime/4.13.2/include/antlr4-runtime/
)

# Source files (your code + generated code)
set(SOURCES
    main.cpp
    grammars/assertions/AssertionLexer.cpp
    grammars/assertions/AssertionParser.cpp
    # add more generated files if ANTLR produces them
)

add_executable(main ${SOURCES})

target_link_libraries(main PRIVATE /opt/homebrew/Cellar/antlr4-cpp-runtime/4.13.2/lib/libantlr4-runtime.a)
