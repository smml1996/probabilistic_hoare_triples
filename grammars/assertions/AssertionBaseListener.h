
// Generated from grammars/assertions/Assertion.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "AssertionListener.h"


/**
 * This class provides an empty implementation of AssertionListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  AssertionBaseListener : public AssertionListener {
public:

  virtual void enterAssertion(AssertionParser::AssertionContext * /*ctx*/) override { }
  virtual void exitAssertion(AssertionParser::AssertionContext * /*ctx*/) override { }

  virtual void enterDistribution_assertion(AssertionParser::Distribution_assertionContext * /*ctx*/) override { }
  virtual void exitDistribution_assertion(AssertionParser::Distribution_assertionContext * /*ctx*/) override { }

  virtual void enterDis_or_expr(AssertionParser::Dis_or_exprContext * /*ctx*/) override { }
  virtual void exitDis_or_expr(AssertionParser::Dis_or_exprContext * /*ctx*/) override { }

  virtual void enterDis_and_expr(AssertionParser::Dis_and_exprContext * /*ctx*/) override { }
  virtual void exitDis_and_expr(AssertionParser::Dis_and_exprContext * /*ctx*/) override { }

  virtual void enterDis_not_expr(AssertionParser::Dis_not_exprContext * /*ctx*/) override { }
  virtual void exitDis_not_expr(AssertionParser::Dis_not_exprContext * /*ctx*/) override { }

  virtual void enterProbability_term(AssertionParser::Probability_termContext * /*ctx*/) override { }
  virtual void exitProbability_term(AssertionParser::Probability_termContext * /*ctx*/) override { }

  virtual void enterPfactor(AssertionParser::PfactorContext * /*ctx*/) override { }
  virtual void exitPfactor(AssertionParser::PfactorContext * /*ctx*/) override { }

  virtual void enterStates_assertion(AssertionParser::States_assertionContext * /*ctx*/) override { }
  virtual void exitStates_assertion(AssertionParser::States_assertionContext * /*ctx*/) override { }

  virtual void enterOr_expr(AssertionParser::Or_exprContext * /*ctx*/) override { }
  virtual void exitOr_expr(AssertionParser::Or_exprContext * /*ctx*/) override { }

  virtual void enterAnd_expr(AssertionParser::And_exprContext * /*ctx*/) override { }
  virtual void exitAnd_expr(AssertionParser::And_exprContext * /*ctx*/) override { }

  virtual void enterNot_expr(AssertionParser::Not_exprContext * /*ctx*/) override { }
  virtual void exitNot_expr(AssertionParser::Not_exprContext * /*ctx*/) override { }

  virtual void enterQCompare(AssertionParser::QCompareContext * /*ctx*/) override { }
  virtual void exitQCompare(AssertionParser::QCompareContext * /*ctx*/) override { }

  virtual void enterBCompare(AssertionParser::BCompareContext * /*ctx*/) override { }
  virtual void exitBCompare(AssertionParser::BCompareContext * /*ctx*/) override { }

  virtual void enterRCompare(AssertionParser::RCompareContext * /*ctx*/) override { }
  virtual void exitRCompare(AssertionParser::RCompareContext * /*ctx*/) override { }

  virtual void enterParens(AssertionParser::ParensContext * /*ctx*/) override { }
  virtual void exitParens(AssertionParser::ParensContext * /*ctx*/) override { }

  virtual void enterReal_term(AssertionParser::Real_termContext * /*ctx*/) override { }
  virtual void exitReal_term(AssertionParser::Real_termContext * /*ctx*/) override { }

  virtual void enterBinary_term(AssertionParser::Binary_termContext * /*ctx*/) override { }
  virtual void exitBinary_term(AssertionParser::Binary_termContext * /*ctx*/) override { }

  virtual void enterBinary_factor(AssertionParser::Binary_factorContext * /*ctx*/) override { }
  virtual void exitBinary_factor(AssertionParser::Binary_factorContext * /*ctx*/) override { }

  virtual void enterQuantum_term(AssertionParser::Quantum_termContext * /*ctx*/) override { }
  virtual void exitQuantum_term(AssertionParser::Quantum_termContext * /*ctx*/) override { }

  virtual void enterQuantum_factor(AssertionParser::Quantum_factorContext * /*ctx*/) override { }
  virtual void exitQuantum_factor(AssertionParser::Quantum_factorContext * /*ctx*/) override { }

  virtual void enterMatrix(AssertionParser::MatrixContext * /*ctx*/) override { }
  virtual void exitMatrix(AssertionParser::MatrixContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

