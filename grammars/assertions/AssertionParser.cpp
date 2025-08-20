
// Generated from grammars/assertions/Assertion.g4 by ANTLR 4.13.2


#include "AssertionListener.h"

#include "AssertionParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct AssertionParserStaticData final {
  AssertionParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  AssertionParserStaticData(const AssertionParserStaticData&) = delete;
  AssertionParserStaticData(AssertionParserStaticData&&) = delete;
  AssertionParserStaticData& operator=(const AssertionParserStaticData&) = delete;
  AssertionParserStaticData& operator=(AssertionParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag assertionParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<AssertionParserStaticData> assertionParserStaticData = nullptr;

void assertionParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (assertionParserStaticData != nullptr) {
    return;
  }
#else
  assert(assertionParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<AssertionParserStaticData>(
    std::vector<std::string>{
      "assertion", "distribution_assertion", "dis_or_expr", "dis_and_expr", 
      "dis_not_expr", "probability_term", "pfactor", "states_assertion", 
      "states_or_expr", "states_and_expr", "states_not_expr", "real_term", 
      "binary_term", "binary_factor", "quantum_term", "quantum_factor", 
      "matrix"
    },
    std::vector<std::string>{
      "", "'or'", "'and'", "'!'", "'('", "')'", "'P'", "'='", "'Tr('", "'['", 
      "']'", "','"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "Row", "BINARYSTRING", 
      "BList", "QList", "RP", "ROP", "BOP", "MOP", "REALNUM", "ID", "INT", 
      "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,23,185,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,1,0,1,0,1,0,1,1,1,1,1,2,1,2,1,2,5,2,43,8,2,10,
  	2,12,2,46,9,2,1,3,1,3,1,3,5,3,51,8,3,10,3,12,3,54,9,3,1,4,1,4,1,4,1,4,
  	1,4,1,4,1,4,1,4,1,4,1,4,3,4,66,8,4,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,
  	5,5,5,77,8,5,10,5,12,5,80,9,5,3,5,82,8,5,1,6,1,6,1,6,1,6,1,6,1,6,3,6,
  	90,8,6,1,7,1,7,1,8,1,8,1,8,5,8,97,8,8,10,8,12,8,100,9,8,1,9,1,9,1,9,5,
  	9,105,8,9,10,9,12,9,108,9,9,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,
  	10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,1,10,3,10,128,8,10,1,11,1,
  	11,1,11,1,11,1,11,1,11,3,11,136,8,11,1,11,1,11,1,11,5,11,141,8,11,10,
  	11,12,11,144,9,11,1,12,1,12,1,12,5,12,149,8,12,10,12,12,12,152,9,12,1,
  	13,1,13,1,13,1,13,3,13,158,8,13,1,14,1,14,1,14,5,14,163,8,14,10,14,12,
  	14,166,9,14,1,15,1,15,1,15,1,15,3,15,172,8,15,1,16,1,16,1,16,1,16,5,16,
  	178,8,16,10,16,12,16,181,9,16,1,16,1,16,1,16,0,1,22,17,0,2,4,6,8,10,12,
  	14,16,18,20,22,24,26,28,30,32,0,0,188,0,34,1,0,0,0,2,37,1,0,0,0,4,39,
  	1,0,0,0,6,47,1,0,0,0,8,65,1,0,0,0,10,81,1,0,0,0,12,89,1,0,0,0,14,91,1,
  	0,0,0,16,93,1,0,0,0,18,101,1,0,0,0,20,127,1,0,0,0,22,135,1,0,0,0,24,145,
  	1,0,0,0,26,157,1,0,0,0,28,159,1,0,0,0,30,171,1,0,0,0,32,173,1,0,0,0,34,
  	35,3,2,1,0,35,36,5,0,0,1,36,1,1,0,0,0,37,38,3,4,2,0,38,3,1,0,0,0,39,44,
  	3,6,3,0,40,41,5,1,0,0,41,43,3,6,3,0,42,40,1,0,0,0,43,46,1,0,0,0,44,42,
  	1,0,0,0,44,45,1,0,0,0,45,5,1,0,0,0,46,44,1,0,0,0,47,52,3,8,4,0,48,49,
  	5,2,0,0,49,51,3,8,4,0,50,48,1,0,0,0,51,54,1,0,0,0,52,50,1,0,0,0,52,53,
  	1,0,0,0,53,7,1,0,0,0,54,52,1,0,0,0,55,56,5,3,0,0,56,66,3,8,4,0,57,58,
  	5,4,0,0,58,59,3,2,1,0,59,60,5,5,0,0,60,66,1,0,0,0,61,62,3,10,5,0,62,63,
  	5,16,0,0,63,64,5,20,0,0,64,66,1,0,0,0,65,55,1,0,0,0,65,57,1,0,0,0,65,
  	61,1,0,0,0,66,9,1,0,0,0,67,68,5,6,0,0,68,69,5,4,0,0,69,70,3,14,7,0,70,
  	71,5,5,0,0,71,82,1,0,0,0,72,82,5,20,0,0,73,78,3,12,6,0,74,75,5,19,0,0,
  	75,77,3,12,6,0,76,74,1,0,0,0,77,80,1,0,0,0,78,76,1,0,0,0,78,79,1,0,0,
  	0,79,82,1,0,0,0,80,78,1,0,0,0,81,67,1,0,0,0,81,72,1,0,0,0,81,73,1,0,0,
  	0,82,11,1,0,0,0,83,84,5,6,0,0,84,85,5,4,0,0,85,86,3,14,7,0,86,87,5,5,
  	0,0,87,90,1,0,0,0,88,90,5,20,0,0,89,83,1,0,0,0,89,88,1,0,0,0,90,13,1,
  	0,0,0,91,92,3,16,8,0,92,15,1,0,0,0,93,98,3,18,9,0,94,95,5,1,0,0,95,97,
  	3,18,9,0,96,94,1,0,0,0,97,100,1,0,0,0,98,96,1,0,0,0,98,99,1,0,0,0,99,
  	17,1,0,0,0,100,98,1,0,0,0,101,106,3,20,10,0,102,103,5,2,0,0,103,105,3,
  	20,10,0,104,102,1,0,0,0,105,108,1,0,0,0,106,104,1,0,0,0,106,107,1,0,0,
  	0,107,19,1,0,0,0,108,106,1,0,0,0,109,110,5,3,0,0,110,128,3,20,10,0,111,
  	112,3,28,14,0,112,113,5,7,0,0,113,114,3,28,14,0,114,128,1,0,0,0,115,116,
  	3,24,12,0,116,117,5,7,0,0,117,118,3,24,12,0,118,128,1,0,0,0,119,120,3,
  	22,11,0,120,121,5,17,0,0,121,122,3,22,11,0,122,128,1,0,0,0,123,124,5,
  	4,0,0,124,125,3,14,7,0,125,126,5,5,0,0,126,128,1,0,0,0,127,109,1,0,0,
  	0,127,111,1,0,0,0,127,115,1,0,0,0,127,119,1,0,0,0,127,123,1,0,0,0,128,
  	21,1,0,0,0,129,130,6,11,-1,0,130,131,5,8,0,0,131,132,3,28,14,0,132,133,
  	5,5,0,0,133,136,1,0,0,0,134,136,5,20,0,0,135,129,1,0,0,0,135,134,1,0,
  	0,0,136,142,1,0,0,0,137,138,10,1,0,0,138,139,5,17,0,0,139,141,3,22,11,
  	2,140,137,1,0,0,0,141,144,1,0,0,0,142,140,1,0,0,0,142,143,1,0,0,0,143,
  	23,1,0,0,0,144,142,1,0,0,0,145,150,3,26,13,0,146,147,5,18,0,0,147,149,
  	3,26,13,0,148,146,1,0,0,0,149,152,1,0,0,0,150,148,1,0,0,0,150,151,1,0,
  	0,0,151,25,1,0,0,0,152,150,1,0,0,0,153,154,5,9,0,0,154,155,5,14,0,0,155,
  	158,5,10,0,0,156,158,5,13,0,0,157,153,1,0,0,0,157,156,1,0,0,0,158,27,
  	1,0,0,0,159,164,3,30,15,0,160,161,5,19,0,0,161,163,3,30,15,0,162,160,
  	1,0,0,0,163,166,1,0,0,0,164,162,1,0,0,0,164,165,1,0,0,0,165,29,1,0,0,
  	0,166,164,1,0,0,0,167,168,5,9,0,0,168,169,5,15,0,0,169,172,5,10,0,0,170,
  	172,3,32,16,0,171,167,1,0,0,0,171,170,1,0,0,0,172,31,1,0,0,0,173,174,
  	5,9,0,0,174,179,5,12,0,0,175,176,5,11,0,0,176,178,5,12,0,0,177,175,1,
  	0,0,0,178,181,1,0,0,0,179,177,1,0,0,0,179,180,1,0,0,0,180,182,1,0,0,0,
  	181,179,1,0,0,0,182,183,5,10,0,0,183,33,1,0,0,0,16,44,52,65,78,81,89,
  	98,106,127,135,142,150,157,164,171,179
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  assertionParserStaticData = std::move(staticData);
}

}

AssertionParser::AssertionParser(TokenStream *input) : AssertionParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

AssertionParser::AssertionParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  AssertionParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *assertionParserStaticData->atn, assertionParserStaticData->decisionToDFA, assertionParserStaticData->sharedContextCache, options);
}

AssertionParser::~AssertionParser() {
  delete _interpreter;
}

const atn::ATN& AssertionParser::getATN() const {
  return *assertionParserStaticData->atn;
}

std::string AssertionParser::getGrammarFileName() const {
  return "Assertion.g4";
}

const std::vector<std::string>& AssertionParser::getRuleNames() const {
  return assertionParserStaticData->ruleNames;
}

const dfa::Vocabulary& AssertionParser::getVocabulary() const {
  return assertionParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView AssertionParser::getSerializedATN() const {
  return assertionParserStaticData->serializedATN;
}


//----------------- AssertionContext ------------------------------------------------------------------

AssertionParser::AssertionContext::AssertionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::Distribution_assertionContext* AssertionParser::AssertionContext::distribution_assertion() {
  return getRuleContext<AssertionParser::Distribution_assertionContext>(0);
}

tree::TerminalNode* AssertionParser::AssertionContext::EOF() {
  return getToken(AssertionParser::EOF, 0);
}


size_t AssertionParser::AssertionContext::getRuleIndex() const {
  return AssertionParser::RuleAssertion;
}

void AssertionParser::AssertionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAssertion(this);
}

void AssertionParser::AssertionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAssertion(this);
}

AssertionParser::AssertionContext* AssertionParser::assertion() {
  AssertionContext *_localctx = _tracker.createInstance<AssertionContext>(_ctx, getState());
  enterRule(_localctx, 0, AssertionParser::RuleAssertion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(34);
    distribution_assertion();
    setState(35);
    match(AssertionParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Distribution_assertionContext ------------------------------------------------------------------

AssertionParser::Distribution_assertionContext::Distribution_assertionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::Dis_or_exprContext* AssertionParser::Distribution_assertionContext::dis_or_expr() {
  return getRuleContext<AssertionParser::Dis_or_exprContext>(0);
}


size_t AssertionParser::Distribution_assertionContext::getRuleIndex() const {
  return AssertionParser::RuleDistribution_assertion;
}

void AssertionParser::Distribution_assertionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDistribution_assertion(this);
}

void AssertionParser::Distribution_assertionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDistribution_assertion(this);
}

AssertionParser::Distribution_assertionContext* AssertionParser::distribution_assertion() {
  Distribution_assertionContext *_localctx = _tracker.createInstance<Distribution_assertionContext>(_ctx, getState());
  enterRule(_localctx, 2, AssertionParser::RuleDistribution_assertion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(37);
    dis_or_expr();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Dis_or_exprContext ------------------------------------------------------------------

AssertionParser::Dis_or_exprContext::Dis_or_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AssertionParser::Dis_and_exprContext *> AssertionParser::Dis_or_exprContext::dis_and_expr() {
  return getRuleContexts<AssertionParser::Dis_and_exprContext>();
}

AssertionParser::Dis_and_exprContext* AssertionParser::Dis_or_exprContext::dis_and_expr(size_t i) {
  return getRuleContext<AssertionParser::Dis_and_exprContext>(i);
}


size_t AssertionParser::Dis_or_exprContext::getRuleIndex() const {
  return AssertionParser::RuleDis_or_expr;
}

void AssertionParser::Dis_or_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDis_or_expr(this);
}

void AssertionParser::Dis_or_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDis_or_expr(this);
}

AssertionParser::Dis_or_exprContext* AssertionParser::dis_or_expr() {
  Dis_or_exprContext *_localctx = _tracker.createInstance<Dis_or_exprContext>(_ctx, getState());
  enterRule(_localctx, 4, AssertionParser::RuleDis_or_expr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(39);
    dis_and_expr();
    setState(44);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::T__0) {
      setState(40);
      match(AssertionParser::T__0);
      setState(41);
      dis_and_expr();
      setState(46);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Dis_and_exprContext ------------------------------------------------------------------

AssertionParser::Dis_and_exprContext::Dis_and_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AssertionParser::Dis_not_exprContext *> AssertionParser::Dis_and_exprContext::dis_not_expr() {
  return getRuleContexts<AssertionParser::Dis_not_exprContext>();
}

AssertionParser::Dis_not_exprContext* AssertionParser::Dis_and_exprContext::dis_not_expr(size_t i) {
  return getRuleContext<AssertionParser::Dis_not_exprContext>(i);
}


size_t AssertionParser::Dis_and_exprContext::getRuleIndex() const {
  return AssertionParser::RuleDis_and_expr;
}

void AssertionParser::Dis_and_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDis_and_expr(this);
}

void AssertionParser::Dis_and_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDis_and_expr(this);
}

AssertionParser::Dis_and_exprContext* AssertionParser::dis_and_expr() {
  Dis_and_exprContext *_localctx = _tracker.createInstance<Dis_and_exprContext>(_ctx, getState());
  enterRule(_localctx, 6, AssertionParser::RuleDis_and_expr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(47);
    dis_not_expr();
    setState(52);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::T__1) {
      setState(48);
      match(AssertionParser::T__1);
      setState(49);
      dis_not_expr();
      setState(54);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Dis_not_exprContext ------------------------------------------------------------------

AssertionParser::Dis_not_exprContext::Dis_not_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::Dis_not_exprContext* AssertionParser::Dis_not_exprContext::dis_not_expr() {
  return getRuleContext<AssertionParser::Dis_not_exprContext>(0);
}

AssertionParser::Distribution_assertionContext* AssertionParser::Dis_not_exprContext::distribution_assertion() {
  return getRuleContext<AssertionParser::Distribution_assertionContext>(0);
}

AssertionParser::Probability_termContext* AssertionParser::Dis_not_exprContext::probability_term() {
  return getRuleContext<AssertionParser::Probability_termContext>(0);
}

tree::TerminalNode* AssertionParser::Dis_not_exprContext::RP() {
  return getToken(AssertionParser::RP, 0);
}

tree::TerminalNode* AssertionParser::Dis_not_exprContext::REALNUM() {
  return getToken(AssertionParser::REALNUM, 0);
}


size_t AssertionParser::Dis_not_exprContext::getRuleIndex() const {
  return AssertionParser::RuleDis_not_expr;
}

void AssertionParser::Dis_not_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDis_not_expr(this);
}

void AssertionParser::Dis_not_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDis_not_expr(this);
}

AssertionParser::Dis_not_exprContext* AssertionParser::dis_not_expr() {
  Dis_not_exprContext *_localctx = _tracker.createInstance<Dis_not_exprContext>(_ctx, getState());
  enterRule(_localctx, 8, AssertionParser::RuleDis_not_expr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(65);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AssertionParser::T__2: {
        enterOuterAlt(_localctx, 1);
        setState(55);
        match(AssertionParser::T__2);
        setState(56);
        dis_not_expr();
        break;
      }

      case AssertionParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(57);
        match(AssertionParser::T__3);
        setState(58);
        distribution_assertion();
        setState(59);
        match(AssertionParser::T__4);
        break;
      }

      case AssertionParser::T__5:
      case AssertionParser::REALNUM: {
        enterOuterAlt(_localctx, 3);
        setState(61);
        probability_term();
        setState(62);
        match(AssertionParser::RP);
        setState(63);
        match(AssertionParser::REALNUM);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Probability_termContext ------------------------------------------------------------------

AssertionParser::Probability_termContext::Probability_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::States_assertionContext* AssertionParser::Probability_termContext::states_assertion() {
  return getRuleContext<AssertionParser::States_assertionContext>(0);
}

tree::TerminalNode* AssertionParser::Probability_termContext::REALNUM() {
  return getToken(AssertionParser::REALNUM, 0);
}

std::vector<AssertionParser::PfactorContext *> AssertionParser::Probability_termContext::pfactor() {
  return getRuleContexts<AssertionParser::PfactorContext>();
}

AssertionParser::PfactorContext* AssertionParser::Probability_termContext::pfactor(size_t i) {
  return getRuleContext<AssertionParser::PfactorContext>(i);
}

std::vector<tree::TerminalNode *> AssertionParser::Probability_termContext::MOP() {
  return getTokens(AssertionParser::MOP);
}

tree::TerminalNode* AssertionParser::Probability_termContext::MOP(size_t i) {
  return getToken(AssertionParser::MOP, i);
}


size_t AssertionParser::Probability_termContext::getRuleIndex() const {
  return AssertionParser::RuleProbability_term;
}

void AssertionParser::Probability_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProbability_term(this);
}

void AssertionParser::Probability_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProbability_term(this);
}

AssertionParser::Probability_termContext* AssertionParser::probability_term() {
  Probability_termContext *_localctx = _tracker.createInstance<Probability_termContext>(_ctx, getState());
  enterRule(_localctx, 10, AssertionParser::RuleProbability_term);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(81);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(67);
      match(AssertionParser::T__5);
      setState(68);
      match(AssertionParser::T__3);
      setState(69);
      states_assertion();
      setState(70);
      match(AssertionParser::T__4);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(72);
      match(AssertionParser::REALNUM);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(73);
      pfactor();
      setState(78);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == AssertionParser::MOP) {
        setState(74);
        match(AssertionParser::MOP);
        setState(75);
        pfactor();
        setState(80);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PfactorContext ------------------------------------------------------------------

AssertionParser::PfactorContext::PfactorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::States_assertionContext* AssertionParser::PfactorContext::states_assertion() {
  return getRuleContext<AssertionParser::States_assertionContext>(0);
}

tree::TerminalNode* AssertionParser::PfactorContext::REALNUM() {
  return getToken(AssertionParser::REALNUM, 0);
}


size_t AssertionParser::PfactorContext::getRuleIndex() const {
  return AssertionParser::RulePfactor;
}

void AssertionParser::PfactorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPfactor(this);
}

void AssertionParser::PfactorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPfactor(this);
}

AssertionParser::PfactorContext* AssertionParser::pfactor() {
  PfactorContext *_localctx = _tracker.createInstance<PfactorContext>(_ctx, getState());
  enterRule(_localctx, 12, AssertionParser::RulePfactor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(89);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AssertionParser::T__5: {
        enterOuterAlt(_localctx, 1);
        setState(83);
        match(AssertionParser::T__5);
        setState(84);
        match(AssertionParser::T__3);
        setState(85);
        states_assertion();
        setState(86);
        match(AssertionParser::T__4);
        break;
      }

      case AssertionParser::REALNUM: {
        enterOuterAlt(_localctx, 2);
        setState(88);
        match(AssertionParser::REALNUM);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- States_assertionContext ------------------------------------------------------------------

AssertionParser::States_assertionContext::States_assertionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::States_or_exprContext* AssertionParser::States_assertionContext::states_or_expr() {
  return getRuleContext<AssertionParser::States_or_exprContext>(0);
}


size_t AssertionParser::States_assertionContext::getRuleIndex() const {
  return AssertionParser::RuleStates_assertion;
}

void AssertionParser::States_assertionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStates_assertion(this);
}

void AssertionParser::States_assertionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStates_assertion(this);
}

AssertionParser::States_assertionContext* AssertionParser::states_assertion() {
  States_assertionContext *_localctx = _tracker.createInstance<States_assertionContext>(_ctx, getState());
  enterRule(_localctx, 14, AssertionParser::RuleStates_assertion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(91);
    states_or_expr();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- States_or_exprContext ------------------------------------------------------------------

AssertionParser::States_or_exprContext::States_or_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t AssertionParser::States_or_exprContext::getRuleIndex() const {
  return AssertionParser::RuleStates_or_expr;
}

void AssertionParser::States_or_exprContext::copyFrom(States_or_exprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- Or_exprContext ------------------------------------------------------------------

std::vector<AssertionParser::States_and_exprContext *> AssertionParser::Or_exprContext::states_and_expr() {
  return getRuleContexts<AssertionParser::States_and_exprContext>();
}

AssertionParser::States_and_exprContext* AssertionParser::Or_exprContext::states_and_expr(size_t i) {
  return getRuleContext<AssertionParser::States_and_exprContext>(i);
}

AssertionParser::Or_exprContext::Or_exprContext(States_or_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::Or_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOr_expr(this);
}
void AssertionParser::Or_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOr_expr(this);
}
AssertionParser::States_or_exprContext* AssertionParser::states_or_expr() {
  States_or_exprContext *_localctx = _tracker.createInstance<States_or_exprContext>(_ctx, getState());
  enterRule(_localctx, 16, AssertionParser::RuleStates_or_expr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    _localctx = _tracker.createInstance<AssertionParser::Or_exprContext>(_localctx);
    enterOuterAlt(_localctx, 1);
    setState(93);
    states_and_expr();
    setState(98);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::T__0) {
      setState(94);
      match(AssertionParser::T__0);
      setState(95);
      states_and_expr();
      setState(100);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- States_and_exprContext ------------------------------------------------------------------

AssertionParser::States_and_exprContext::States_and_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t AssertionParser::States_and_exprContext::getRuleIndex() const {
  return AssertionParser::RuleStates_and_expr;
}

void AssertionParser::States_and_exprContext::copyFrom(States_and_exprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- And_exprContext ------------------------------------------------------------------

std::vector<AssertionParser::States_not_exprContext *> AssertionParser::And_exprContext::states_not_expr() {
  return getRuleContexts<AssertionParser::States_not_exprContext>();
}

AssertionParser::States_not_exprContext* AssertionParser::And_exprContext::states_not_expr(size_t i) {
  return getRuleContext<AssertionParser::States_not_exprContext>(i);
}

AssertionParser::And_exprContext::And_exprContext(States_and_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::And_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAnd_expr(this);
}
void AssertionParser::And_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAnd_expr(this);
}
AssertionParser::States_and_exprContext* AssertionParser::states_and_expr() {
  States_and_exprContext *_localctx = _tracker.createInstance<States_and_exprContext>(_ctx, getState());
  enterRule(_localctx, 18, AssertionParser::RuleStates_and_expr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    _localctx = _tracker.createInstance<AssertionParser::And_exprContext>(_localctx);
    enterOuterAlt(_localctx, 1);
    setState(101);
    states_not_expr();
    setState(106);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::T__1) {
      setState(102);
      match(AssertionParser::T__1);
      setState(103);
      states_not_expr();
      setState(108);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- States_not_exprContext ------------------------------------------------------------------

AssertionParser::States_not_exprContext::States_not_exprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t AssertionParser::States_not_exprContext::getRuleIndex() const {
  return AssertionParser::RuleStates_not_expr;
}

void AssertionParser::States_not_exprContext::copyFrom(States_not_exprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- Not_exprContext ------------------------------------------------------------------

AssertionParser::States_not_exprContext* AssertionParser::Not_exprContext::states_not_expr() {
  return getRuleContext<AssertionParser::States_not_exprContext>(0);
}

AssertionParser::Not_exprContext::Not_exprContext(States_not_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::Not_exprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNot_expr(this);
}
void AssertionParser::Not_exprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNot_expr(this);
}
//----------------- RCompareContext ------------------------------------------------------------------

std::vector<AssertionParser::Real_termContext *> AssertionParser::RCompareContext::real_term() {
  return getRuleContexts<AssertionParser::Real_termContext>();
}

AssertionParser::Real_termContext* AssertionParser::RCompareContext::real_term(size_t i) {
  return getRuleContext<AssertionParser::Real_termContext>(i);
}

tree::TerminalNode* AssertionParser::RCompareContext::ROP() {
  return getToken(AssertionParser::ROP, 0);
}

AssertionParser::RCompareContext::RCompareContext(States_not_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::RCompareContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRCompare(this);
}
void AssertionParser::RCompareContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRCompare(this);
}
//----------------- ParensContext ------------------------------------------------------------------

AssertionParser::States_assertionContext* AssertionParser::ParensContext::states_assertion() {
  return getRuleContext<AssertionParser::States_assertionContext>(0);
}

AssertionParser::ParensContext::ParensContext(States_not_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::ParensContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParens(this);
}
void AssertionParser::ParensContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParens(this);
}
//----------------- QCompareContext ------------------------------------------------------------------

std::vector<AssertionParser::Quantum_termContext *> AssertionParser::QCompareContext::quantum_term() {
  return getRuleContexts<AssertionParser::Quantum_termContext>();
}

AssertionParser::Quantum_termContext* AssertionParser::QCompareContext::quantum_term(size_t i) {
  return getRuleContext<AssertionParser::Quantum_termContext>(i);
}

AssertionParser::QCompareContext::QCompareContext(States_not_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::QCompareContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQCompare(this);
}
void AssertionParser::QCompareContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQCompare(this);
}
//----------------- BCompareContext ------------------------------------------------------------------

std::vector<AssertionParser::Binary_termContext *> AssertionParser::BCompareContext::binary_term() {
  return getRuleContexts<AssertionParser::Binary_termContext>();
}

AssertionParser::Binary_termContext* AssertionParser::BCompareContext::binary_term(size_t i) {
  return getRuleContext<AssertionParser::Binary_termContext>(i);
}

AssertionParser::BCompareContext::BCompareContext(States_not_exprContext *ctx) { copyFrom(ctx); }

void AssertionParser::BCompareContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBCompare(this);
}
void AssertionParser::BCompareContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBCompare(this);
}
AssertionParser::States_not_exprContext* AssertionParser::states_not_expr() {
  States_not_exprContext *_localctx = _tracker.createInstance<States_not_exprContext>(_ctx, getState());
  enterRule(_localctx, 20, AssertionParser::RuleStates_not_expr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(127);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<AssertionParser::Not_exprContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(109);
      match(AssertionParser::T__2);
      setState(110);
      states_not_expr();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<AssertionParser::QCompareContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(111);
      quantum_term();
      setState(112);
      match(AssertionParser::T__6);
      setState(113);
      quantum_term();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<AssertionParser::BCompareContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(115);
      binary_term();
      setState(116);
      match(AssertionParser::T__6);
      setState(117);
      binary_term();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<AssertionParser::RCompareContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(119);
      real_term(0);
      setState(120);
      match(AssertionParser::ROP);
      setState(121);
      real_term(0);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<AssertionParser::ParensContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(123);
      match(AssertionParser::T__3);
      setState(124);
      states_assertion();
      setState(125);
      match(AssertionParser::T__4);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Real_termContext ------------------------------------------------------------------

AssertionParser::Real_termContext::Real_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AssertionParser::Quantum_termContext* AssertionParser::Real_termContext::quantum_term() {
  return getRuleContext<AssertionParser::Quantum_termContext>(0);
}

tree::TerminalNode* AssertionParser::Real_termContext::REALNUM() {
  return getToken(AssertionParser::REALNUM, 0);
}

std::vector<AssertionParser::Real_termContext *> AssertionParser::Real_termContext::real_term() {
  return getRuleContexts<AssertionParser::Real_termContext>();
}

AssertionParser::Real_termContext* AssertionParser::Real_termContext::real_term(size_t i) {
  return getRuleContext<AssertionParser::Real_termContext>(i);
}

tree::TerminalNode* AssertionParser::Real_termContext::ROP() {
  return getToken(AssertionParser::ROP, 0);
}


size_t AssertionParser::Real_termContext::getRuleIndex() const {
  return AssertionParser::RuleReal_term;
}

void AssertionParser::Real_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterReal_term(this);
}

void AssertionParser::Real_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitReal_term(this);
}


AssertionParser::Real_termContext* AssertionParser::real_term() {
   return real_term(0);
}

AssertionParser::Real_termContext* AssertionParser::real_term(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  AssertionParser::Real_termContext *_localctx = _tracker.createInstance<Real_termContext>(_ctx, parentState);
  AssertionParser::Real_termContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 22;
  enterRecursionRule(_localctx, 22, AssertionParser::RuleReal_term, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(135);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AssertionParser::T__7: {
        setState(130);
        match(AssertionParser::T__7);
        setState(131);
        quantum_term();
        setState(132);
        match(AssertionParser::T__4);
        break;
      }

      case AssertionParser::REALNUM: {
        setState(134);
        match(AssertionParser::REALNUM);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(142);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<Real_termContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleReal_term);
        setState(137);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(138);
        match(AssertionParser::ROP);
        setState(139);
        real_term(2); 
      }
      setState(144);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- Binary_termContext ------------------------------------------------------------------

AssertionParser::Binary_termContext::Binary_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AssertionParser::Binary_factorContext *> AssertionParser::Binary_termContext::binary_factor() {
  return getRuleContexts<AssertionParser::Binary_factorContext>();
}

AssertionParser::Binary_factorContext* AssertionParser::Binary_termContext::binary_factor(size_t i) {
  return getRuleContext<AssertionParser::Binary_factorContext>(i);
}

std::vector<tree::TerminalNode *> AssertionParser::Binary_termContext::BOP() {
  return getTokens(AssertionParser::BOP);
}

tree::TerminalNode* AssertionParser::Binary_termContext::BOP(size_t i) {
  return getToken(AssertionParser::BOP, i);
}


size_t AssertionParser::Binary_termContext::getRuleIndex() const {
  return AssertionParser::RuleBinary_term;
}

void AssertionParser::Binary_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinary_term(this);
}

void AssertionParser::Binary_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinary_term(this);
}

AssertionParser::Binary_termContext* AssertionParser::binary_term() {
  Binary_termContext *_localctx = _tracker.createInstance<Binary_termContext>(_ctx, getState());
  enterRule(_localctx, 24, AssertionParser::RuleBinary_term);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(145);
    binary_factor();
    setState(150);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::BOP) {
      setState(146);
      match(AssertionParser::BOP);
      setState(147);
      binary_factor();
      setState(152);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Binary_factorContext ------------------------------------------------------------------

AssertionParser::Binary_factorContext::Binary_factorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AssertionParser::Binary_factorContext::BList() {
  return getToken(AssertionParser::BList, 0);
}

tree::TerminalNode* AssertionParser::Binary_factorContext::BINARYSTRING() {
  return getToken(AssertionParser::BINARYSTRING, 0);
}


size_t AssertionParser::Binary_factorContext::getRuleIndex() const {
  return AssertionParser::RuleBinary_factor;
}

void AssertionParser::Binary_factorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBinary_factor(this);
}

void AssertionParser::Binary_factorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBinary_factor(this);
}

AssertionParser::Binary_factorContext* AssertionParser::binary_factor() {
  Binary_factorContext *_localctx = _tracker.createInstance<Binary_factorContext>(_ctx, getState());
  enterRule(_localctx, 26, AssertionParser::RuleBinary_factor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(157);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AssertionParser::T__8: {
        enterOuterAlt(_localctx, 1);
        setState(153);
        match(AssertionParser::T__8);
        setState(154);
        match(AssertionParser::BList);
        setState(155);
        match(AssertionParser::T__9);
        break;
      }

      case AssertionParser::BINARYSTRING: {
        enterOuterAlt(_localctx, 2);
        setState(156);
        match(AssertionParser::BINARYSTRING);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Quantum_termContext ------------------------------------------------------------------

AssertionParser::Quantum_termContext::Quantum_termContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AssertionParser::Quantum_factorContext *> AssertionParser::Quantum_termContext::quantum_factor() {
  return getRuleContexts<AssertionParser::Quantum_factorContext>();
}

AssertionParser::Quantum_factorContext* AssertionParser::Quantum_termContext::quantum_factor(size_t i) {
  return getRuleContext<AssertionParser::Quantum_factorContext>(i);
}

std::vector<tree::TerminalNode *> AssertionParser::Quantum_termContext::MOP() {
  return getTokens(AssertionParser::MOP);
}

tree::TerminalNode* AssertionParser::Quantum_termContext::MOP(size_t i) {
  return getToken(AssertionParser::MOP, i);
}


size_t AssertionParser::Quantum_termContext::getRuleIndex() const {
  return AssertionParser::RuleQuantum_term;
}

void AssertionParser::Quantum_termContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQuantum_term(this);
}

void AssertionParser::Quantum_termContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQuantum_term(this);
}

AssertionParser::Quantum_termContext* AssertionParser::quantum_term() {
  Quantum_termContext *_localctx = _tracker.createInstance<Quantum_termContext>(_ctx, getState());
  enterRule(_localctx, 28, AssertionParser::RuleQuantum_term);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(159);
    quantum_factor();
    setState(164);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::MOP) {
      setState(160);
      match(AssertionParser::MOP);
      setState(161);
      quantum_factor();
      setState(166);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Quantum_factorContext ------------------------------------------------------------------

AssertionParser::Quantum_factorContext::Quantum_factorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AssertionParser::Quantum_factorContext::QList() {
  return getToken(AssertionParser::QList, 0);
}

AssertionParser::MatrixContext* AssertionParser::Quantum_factorContext::matrix() {
  return getRuleContext<AssertionParser::MatrixContext>(0);
}


size_t AssertionParser::Quantum_factorContext::getRuleIndex() const {
  return AssertionParser::RuleQuantum_factor;
}

void AssertionParser::Quantum_factorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQuantum_factor(this);
}

void AssertionParser::Quantum_factorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQuantum_factor(this);
}

AssertionParser::Quantum_factorContext* AssertionParser::quantum_factor() {
  Quantum_factorContext *_localctx = _tracker.createInstance<Quantum_factorContext>(_ctx, getState());
  enterRule(_localctx, 30, AssertionParser::RuleQuantum_factor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(171);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(167);
      match(AssertionParser::T__8);
      setState(168);
      match(AssertionParser::QList);
      setState(169);
      match(AssertionParser::T__9);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(170);
      matrix();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MatrixContext ------------------------------------------------------------------

AssertionParser::MatrixContext::MatrixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> AssertionParser::MatrixContext::Row() {
  return getTokens(AssertionParser::Row);
}

tree::TerminalNode* AssertionParser::MatrixContext::Row(size_t i) {
  return getToken(AssertionParser::Row, i);
}


size_t AssertionParser::MatrixContext::getRuleIndex() const {
  return AssertionParser::RuleMatrix;
}

void AssertionParser::MatrixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMatrix(this);
}

void AssertionParser::MatrixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<AssertionListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMatrix(this);
}

AssertionParser::MatrixContext* AssertionParser::matrix() {
  MatrixContext *_localctx = _tracker.createInstance<MatrixContext>(_ctx, getState());
  enterRule(_localctx, 32, AssertionParser::RuleMatrix);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(173);
    match(AssertionParser::T__8);
    setState(174);
    match(AssertionParser::Row);
    setState(179);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AssertionParser::T__10) {
      setState(175);
      match(AssertionParser::T__10);
      setState(176);
      match(AssertionParser::Row);
      setState(181);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(182);
    match(AssertionParser::T__9);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool AssertionParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 11: return real_termSempred(antlrcpp::downCast<Real_termContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool AssertionParser::real_termSempred(Real_termContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void AssertionParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  assertionParserInitialize();
#else
  ::antlr4::internal::call_once(assertionParserOnceFlag, assertionParserInitialize);
#endif
}
