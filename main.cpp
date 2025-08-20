#include "antlr4-runtime.h"
#include "grammars/assertions/AssertionLexer.h"
#include "grammars/assertions/AssertionParser.h"

using namespace antlr4;

int main() {
    std::string inputText = "assert q0 = 1 and (q1 != 0 or q2 = 5)";

    ANTLRInputStream input(inputText);
    AssertionLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    AssertionParser parser(&tokens);

    tree::ParseTree *tree = parser.assertion();  // parse entry point

    std::cout << "Parse tree: " << tree->toStringTree(&parser) << std::endl;

    return 0;
}
