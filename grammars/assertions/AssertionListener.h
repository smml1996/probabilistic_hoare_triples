
// Generated from grammars/assertions/Assertion.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AssertionParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by AssertionParser.
 */
class  AssertionListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterAssertion(AssertionParser::AssertionContext *ctx) = 0;
  virtual void exitAssertion(AssertionParser::AssertionContext *ctx) = 0;

  virtual void enterDistribution_assertion(AssertionParser::Distribution_assertionContext *ctx) = 0;
  virtual void exitDistribution_assertion(AssertionParser::Distribution_assertionContext *ctx) = 0;

  virtual void enterDis_or_expr(AssertionParser::Dis_or_exprContext *ctx) = 0;
  virtual void exitDis_or_expr(AssertionParser::Dis_or_exprContext *ctx) = 0;

  virtual void enterDis_and_expr(AssertionParser::Dis_and_exprContext *ctx) = 0;
  virtual void exitDis_and_expr(AssertionParser::Dis_and_exprContext *ctx) = 0;

  virtual void enterDis_not_expr(AssertionParser::Dis_not_exprContext *ctx) = 0;
  virtual void exitDis_not_expr(AssertionParser::Dis_not_exprContext *ctx) = 0;

  virtual void enterProbability_term(AssertionParser::Probability_termContext *ctx) = 0;
  virtual void exitProbability_term(AssertionParser::Probability_termContext *ctx) = 0;

  virtual void enterPfactor(AssertionParser::PfactorContext *ctx) = 0;
  virtual void exitPfactor(AssertionParser::PfactorContext *ctx) = 0;

  virtual void enterStates_assertion(AssertionParser::States_assertionContext *ctx) = 0;
  virtual void exitStates_assertion(AssertionParser::States_assertionContext *ctx) = 0;

  virtual void enterOr_expr(AssertionParser::Or_exprContext *ctx) = 0;
  virtual void exitOr_expr(AssertionParser::Or_exprContext *ctx) = 0;

  virtual void enterAnd_expr(AssertionParser::And_exprContext *ctx) = 0;
  virtual void exitAnd_expr(AssertionParser::And_exprContext *ctx) = 0;

  virtual void enterNot_expr(AssertionParser::Not_exprContext *ctx) = 0;
  virtual void exitNot_expr(AssertionParser::Not_exprContext *ctx) = 0;

  virtual void enterQCompare(AssertionParser::QCompareContext *ctx) = 0;
  virtual void exitQCompare(AssertionParser::QCompareContext *ctx) = 0;

  virtual void enterBCompare(AssertionParser::BCompareContext *ctx) = 0;
  virtual void exitBCompare(AssertionParser::BCompareContext *ctx) = 0;

  virtual void enterRCompare(AssertionParser::RCompareContext *ctx) = 0;
  virtual void exitRCompare(AssertionParser::RCompareContext *ctx) = 0;

  virtual void enterParens(AssertionParser::ParensContext *ctx) = 0;
  virtual void exitParens(AssertionParser::ParensContext *ctx) = 0;

  virtual void enterReal_term(AssertionParser::Real_termContext *ctx) = 0;
  virtual void exitReal_term(AssertionParser::Real_termContext *ctx) = 0;

  virtual void enterBinary_term(AssertionParser::Binary_termContext *ctx) = 0;
  virtual void exitBinary_term(AssertionParser::Binary_termContext *ctx) = 0;

  virtual void enterBinary_factor(AssertionParser::Binary_factorContext *ctx) = 0;
  virtual void exitBinary_factor(AssertionParser::Binary_factorContext *ctx) = 0;

  virtual void enterQuantum_term(AssertionParser::Quantum_termContext *ctx) = 0;
  virtual void exitQuantum_term(AssertionParser::Quantum_termContext *ctx) = 0;

  virtual void enterQuantum_factor(AssertionParser::Quantum_factorContext *ctx) = 0;
  virtual void exitQuantum_factor(AssertionParser::Quantum_factorContext *ctx) = 0;

  virtual void enterMatrix(AssertionParser::MatrixContext *ctx) = 0;
  virtual void exitMatrix(AssertionParser::MatrixContext *ctx) = 0;


};

