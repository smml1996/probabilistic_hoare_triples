
// Generated from grammars/assertions/Assertion.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  AssertionParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, Row = 12, BINARYSTRING = 13, 
    BList = 14, QList = 15, RP = 16, ROP = 17, BOP = 18, MOP = 19, REALNUM = 20, 
    ID = 21, INT = 22, WS = 23
  };

  enum {
    RuleAssertion = 0, RuleDistribution_assertion = 1, RuleDis_or_expr = 2, 
    RuleDis_and_expr = 3, RuleDis_not_expr = 4, RuleProbability_term = 5, 
    RulePfactor = 6, RuleStates_assertion = 7, RuleStates_or_expr = 8, RuleStates_and_expr = 9, 
    RuleStates_not_expr = 10, RuleReal_term = 11, RuleBinary_term = 12, 
    RuleBinary_factor = 13, RuleQuantum_term = 14, RuleQuantum_factor = 15, 
    RuleMatrix = 16
  };

  explicit AssertionParser(antlr4::TokenStream *input);

  AssertionParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~AssertionParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class AssertionContext;
  class Distribution_assertionContext;
  class Dis_or_exprContext;
  class Dis_and_exprContext;
  class Dis_not_exprContext;
  class Probability_termContext;
  class PfactorContext;
  class States_assertionContext;
  class States_or_exprContext;
  class States_and_exprContext;
  class States_not_exprContext;
  class Real_termContext;
  class Binary_termContext;
  class Binary_factorContext;
  class Quantum_termContext;
  class Quantum_factorContext;
  class MatrixContext; 

  class  AssertionContext : public antlr4::ParserRuleContext {
  public:
    AssertionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Distribution_assertionContext *distribution_assertion();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  AssertionContext* assertion();

  class  Distribution_assertionContext : public antlr4::ParserRuleContext {
  public:
    Distribution_assertionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Dis_or_exprContext *dis_or_expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Distribution_assertionContext* distribution_assertion();

  class  Dis_or_exprContext : public antlr4::ParserRuleContext {
  public:
    Dis_or_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Dis_and_exprContext *> dis_and_expr();
    Dis_and_exprContext* dis_and_expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Dis_or_exprContext* dis_or_expr();

  class  Dis_and_exprContext : public antlr4::ParserRuleContext {
  public:
    Dis_and_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Dis_not_exprContext *> dis_not_expr();
    Dis_not_exprContext* dis_not_expr(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Dis_and_exprContext* dis_and_expr();

  class  Dis_not_exprContext : public antlr4::ParserRuleContext {
  public:
    Dis_not_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Dis_not_exprContext *dis_not_expr();
    Distribution_assertionContext *distribution_assertion();
    Probability_termContext *probability_term();
    antlr4::tree::TerminalNode *RP();
    antlr4::tree::TerminalNode *REALNUM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Dis_not_exprContext* dis_not_expr();

  class  Probability_termContext : public antlr4::ParserRuleContext {
  public:
    Probability_termContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    States_assertionContext *states_assertion();
    antlr4::tree::TerminalNode *REALNUM();
    std::vector<PfactorContext *> pfactor();
    PfactorContext* pfactor(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MOP();
    antlr4::tree::TerminalNode* MOP(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Probability_termContext* probability_term();

  class  PfactorContext : public antlr4::ParserRuleContext {
  public:
    PfactorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    States_assertionContext *states_assertion();
    antlr4::tree::TerminalNode *REALNUM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  PfactorContext* pfactor();

  class  States_assertionContext : public antlr4::ParserRuleContext {
  public:
    States_assertionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    States_or_exprContext *states_or_expr();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  States_assertionContext* states_assertion();

  class  States_or_exprContext : public antlr4::ParserRuleContext {
  public:
    States_or_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    States_or_exprContext() = default;
    void copyFrom(States_or_exprContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  Or_exprContext : public States_or_exprContext {
  public:
    Or_exprContext(States_or_exprContext *ctx);

    std::vector<States_and_exprContext *> states_and_expr();
    States_and_exprContext* states_and_expr(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  States_or_exprContext* states_or_expr();

  class  States_and_exprContext : public antlr4::ParserRuleContext {
  public:
    States_and_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    States_and_exprContext() = default;
    void copyFrom(States_and_exprContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  And_exprContext : public States_and_exprContext {
  public:
    And_exprContext(States_and_exprContext *ctx);

    std::vector<States_not_exprContext *> states_not_expr();
    States_not_exprContext* states_not_expr(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  States_and_exprContext* states_and_expr();

  class  States_not_exprContext : public antlr4::ParserRuleContext {
  public:
    States_not_exprContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    States_not_exprContext() = default;
    void copyFrom(States_not_exprContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  Not_exprContext : public States_not_exprContext {
  public:
    Not_exprContext(States_not_exprContext *ctx);

    States_not_exprContext *states_not_expr();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  class  RCompareContext : public States_not_exprContext {
  public:
    RCompareContext(States_not_exprContext *ctx);

    std::vector<Real_termContext *> real_term();
    Real_termContext* real_term(size_t i);
    antlr4::tree::TerminalNode *ROP();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  class  ParensContext : public States_not_exprContext {
  public:
    ParensContext(States_not_exprContext *ctx);

    States_assertionContext *states_assertion();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  class  QCompareContext : public States_not_exprContext {
  public:
    QCompareContext(States_not_exprContext *ctx);

    std::vector<Quantum_termContext *> quantum_term();
    Quantum_termContext* quantum_term(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  class  BCompareContext : public States_not_exprContext {
  public:
    BCompareContext(States_not_exprContext *ctx);

    std::vector<Binary_termContext *> binary_term();
    Binary_termContext* binary_term(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
  };

  States_not_exprContext* states_not_expr();

  class  Real_termContext : public antlr4::ParserRuleContext {
  public:
    Real_termContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Quantum_termContext *quantum_term();
    antlr4::tree::TerminalNode *REALNUM();
    std::vector<Real_termContext *> real_term();
    Real_termContext* real_term(size_t i);
    antlr4::tree::TerminalNode *ROP();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Real_termContext* real_term();
  Real_termContext* real_term(int precedence);
  class  Binary_termContext : public antlr4::ParserRuleContext {
  public:
    Binary_termContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Binary_factorContext *> binary_factor();
    Binary_factorContext* binary_factor(size_t i);
    std::vector<antlr4::tree::TerminalNode *> BOP();
    antlr4::tree::TerminalNode* BOP(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Binary_termContext* binary_term();

  class  Binary_factorContext : public antlr4::ParserRuleContext {
  public:
    Binary_factorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *BList();
    antlr4::tree::TerminalNode *BINARYSTRING();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Binary_factorContext* binary_factor();

  class  Quantum_termContext : public antlr4::ParserRuleContext {
  public:
    Quantum_termContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Quantum_factorContext *> quantum_factor();
    Quantum_factorContext* quantum_factor(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MOP();
    antlr4::tree::TerminalNode* MOP(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Quantum_termContext* quantum_term();

  class  Quantum_factorContext : public antlr4::ParserRuleContext {
  public:
    Quantum_factorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *QList();
    MatrixContext *matrix();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Quantum_factorContext* quantum_factor();

  class  MatrixContext : public antlr4::ParserRuleContext {
  public:
    MatrixContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Row();
    antlr4::tree::TerminalNode* Row(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  MatrixContext* matrix();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool real_termSempred(Real_termContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

