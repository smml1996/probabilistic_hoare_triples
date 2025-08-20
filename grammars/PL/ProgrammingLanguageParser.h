
// Generated from grammars/PL/ProgrammingLanguage.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ProgrammingLanguageParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    UNITARY = 15, REALNUM = 16, ID = 17
  };

  enum {
    RuleLanguage = 0, RuleAlgorithm = 1, RuleClassical_statement = 2, RuleQlist = 3
  };

  explicit ProgrammingLanguageParser(antlr4::TokenStream *input);

  ProgrammingLanguageParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~ProgrammingLanguageParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class LanguageContext;
  class AlgorithmContext;
  class Classical_statementContext;
  class QlistContext; 

  class  LanguageContext : public antlr4::ParserRuleContext {
  public:
    LanguageContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    AlgorithmContext *algorithm();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  LanguageContext* language();

  class  AlgorithmContext : public antlr4::ParserRuleContext {
  public:
    AlgorithmContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Classical_statementContext *classical_statement();
    antlr4::tree::TerminalNode *UNITARY();
    QlistContext *qlist();
    antlr4::tree::TerminalNode *ID();
    std::vector<AlgorithmContext *> algorithm();
    AlgorithmContext* algorithm(size_t i);
    antlr4::tree::TerminalNode *REALNUM();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  AlgorithmContext* algorithm();
  AlgorithmContext* algorithm(int precedence);
  class  Classical_statementContext : public antlr4::ParserRuleContext {
  public:
    Classical_statementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Classical_statementContext* classical_statement();

  class  QlistContext : public antlr4::ParserRuleContext {
  public:
    QlistContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  QlistContext* qlist();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool algorithmSempred(AlgorithmContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

