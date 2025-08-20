
// Generated from grammars/PL/ProgrammingLanguage.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ProgrammingLanguageListener.h"


/**
 * This class provides an empty implementation of ProgrammingLanguageListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  ProgrammingLanguageBaseListener : public ProgrammingLanguageListener {
public:

  virtual void enterLanguage(ProgrammingLanguageParser::LanguageContext * /*ctx*/) override { }
  virtual void exitLanguage(ProgrammingLanguageParser::LanguageContext * /*ctx*/) override { }

  virtual void enterAlgorithm(ProgrammingLanguageParser::AlgorithmContext * /*ctx*/) override { }
  virtual void exitAlgorithm(ProgrammingLanguageParser::AlgorithmContext * /*ctx*/) override { }

  virtual void enterClassical_statement(ProgrammingLanguageParser::Classical_statementContext * /*ctx*/) override { }
  virtual void exitClassical_statement(ProgrammingLanguageParser::Classical_statementContext * /*ctx*/) override { }

  virtual void enterQlist(ProgrammingLanguageParser::QlistContext * /*ctx*/) override { }
  virtual void exitQlist(ProgrammingLanguageParser::QlistContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

