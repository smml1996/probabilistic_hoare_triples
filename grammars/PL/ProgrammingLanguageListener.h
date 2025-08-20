
// Generated from grammars/PL/ProgrammingLanguage.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ProgrammingLanguageParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by ProgrammingLanguageParser.
 */
class  ProgrammingLanguageListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterLanguage(ProgrammingLanguageParser::LanguageContext *ctx) = 0;
  virtual void exitLanguage(ProgrammingLanguageParser::LanguageContext *ctx) = 0;

  virtual void enterAlgorithm(ProgrammingLanguageParser::AlgorithmContext *ctx) = 0;
  virtual void exitAlgorithm(ProgrammingLanguageParser::AlgorithmContext *ctx) = 0;

  virtual void enterClassical_statement(ProgrammingLanguageParser::Classical_statementContext *ctx) = 0;
  virtual void exitClassical_statement(ProgrammingLanguageParser::Classical_statementContext *ctx) = 0;

  virtual void enterQlist(ProgrammingLanguageParser::QlistContext *ctx) = 0;
  virtual void exitQlist(ProgrammingLanguageParser::QlistContext *ctx) = 0;


};

