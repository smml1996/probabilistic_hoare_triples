
// Generated from grammars/PL/ProgrammingLanguage.g4 by ANTLR 4.13.2


#include "ProgrammingLanguageListener.h"

#include "ProgrammingLanguageParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct ProgrammingLanguageParserStaticData final {
  ProgrammingLanguageParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ProgrammingLanguageParserStaticData(const ProgrammingLanguageParserStaticData&) = delete;
  ProgrammingLanguageParserStaticData(ProgrammingLanguageParserStaticData&&) = delete;
  ProgrammingLanguageParserStaticData& operator=(const ProgrammingLanguageParserStaticData&) = delete;
  ProgrammingLanguageParserStaticData& operator=(ProgrammingLanguageParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag programminglanguageParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ProgrammingLanguageParserStaticData> programminglanguageParserStaticData = nullptr;

void programminglanguageParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (programminglanguageParserStaticData != nullptr) {
    return;
  }
#else
  assert(programminglanguageParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ProgrammingLanguageParserStaticData>(
    std::vector<std::string>{
      "language", "algorithm", "classical_statement", "qlist"
    },
    std::vector<std::string>{
      "", "'skip'", "'('", "')'", "';'", "'if'", "'{'", "'}'", "'else'", 
      "'%'", "'='", "'0'", "'1'", "'measure'", "','"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "UNITARY", 
      "REALNUM", "ID"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,17,71,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,1,0,1,0,1,0,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,32,
  	8,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,1,41,8,1,10,1,12,1,44,9,1,1,2,1,2,1,
  	2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,3,2,61,8,2,1,3,1,3,
  	1,3,5,3,66,8,3,10,3,12,3,69,9,3,1,3,0,1,2,4,0,2,4,6,0,0,75,0,8,1,0,0,
  	0,2,31,1,0,0,0,4,60,1,0,0,0,6,62,1,0,0,0,8,9,3,2,1,0,9,10,5,0,0,1,10,
  	1,1,0,0,0,11,12,6,1,-1,0,12,32,5,1,0,0,13,32,3,4,2,0,14,15,5,15,0,0,15,
  	16,5,2,0,0,16,17,3,6,3,0,17,18,5,3,0,0,18,32,1,0,0,0,19,20,5,5,0,0,20,
  	21,5,2,0,0,21,22,5,17,0,0,22,23,5,3,0,0,23,24,5,6,0,0,24,25,3,2,1,0,25,
  	26,5,7,0,0,26,27,5,8,0,0,27,28,5,6,0,0,28,29,3,2,1,0,29,30,5,7,0,0,30,
  	32,1,0,0,0,31,11,1,0,0,0,31,13,1,0,0,0,31,14,1,0,0,0,31,19,1,0,0,0,32,
  	42,1,0,0,0,33,34,10,3,0,0,34,35,5,4,0,0,35,41,3,2,1,4,36,37,10,1,0,0,
  	37,38,5,9,0,0,38,39,5,16,0,0,39,41,5,9,0,0,40,33,1,0,0,0,40,36,1,0,0,
  	0,41,44,1,0,0,0,42,40,1,0,0,0,42,43,1,0,0,0,43,3,1,0,0,0,44,42,1,0,0,
  	0,45,46,5,17,0,0,46,47,5,10,0,0,47,61,5,11,0,0,48,49,5,17,0,0,49,50,5,
  	10,0,0,50,61,5,12,0,0,51,52,5,17,0,0,52,53,5,10,0,0,53,61,5,17,0,0,54,
  	55,5,17,0,0,55,56,5,10,0,0,56,57,5,13,0,0,57,58,5,2,0,0,58,59,5,17,0,
  	0,59,61,5,3,0,0,60,45,1,0,0,0,60,48,1,0,0,0,60,51,1,0,0,0,60,54,1,0,0,
  	0,61,5,1,0,0,0,62,67,5,17,0,0,63,64,5,14,0,0,64,66,5,17,0,0,65,63,1,0,
  	0,0,66,69,1,0,0,0,67,65,1,0,0,0,67,68,1,0,0,0,68,7,1,0,0,0,69,67,1,0,
  	0,0,5,31,40,42,60,67
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  programminglanguageParserStaticData = std::move(staticData);
}

}

ProgrammingLanguageParser::ProgrammingLanguageParser(TokenStream *input) : ProgrammingLanguageParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

ProgrammingLanguageParser::ProgrammingLanguageParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  ProgrammingLanguageParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *programminglanguageParserStaticData->atn, programminglanguageParserStaticData->decisionToDFA, programminglanguageParserStaticData->sharedContextCache, options);
}

ProgrammingLanguageParser::~ProgrammingLanguageParser() {
  delete _interpreter;
}

const atn::ATN& ProgrammingLanguageParser::getATN() const {
  return *programminglanguageParserStaticData->atn;
}

std::string ProgrammingLanguageParser::getGrammarFileName() const {
  return "ProgrammingLanguage.g4";
}

const std::vector<std::string>& ProgrammingLanguageParser::getRuleNames() const {
  return programminglanguageParserStaticData->ruleNames;
}

const dfa::Vocabulary& ProgrammingLanguageParser::getVocabulary() const {
  return programminglanguageParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ProgrammingLanguageParser::getSerializedATN() const {
  return programminglanguageParserStaticData->serializedATN;
}


//----------------- LanguageContext ------------------------------------------------------------------

ProgrammingLanguageParser::LanguageContext::LanguageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ProgrammingLanguageParser::AlgorithmContext* ProgrammingLanguageParser::LanguageContext::algorithm() {
  return getRuleContext<ProgrammingLanguageParser::AlgorithmContext>(0);
}

tree::TerminalNode* ProgrammingLanguageParser::LanguageContext::EOF() {
  return getToken(ProgrammingLanguageParser::EOF, 0);
}


size_t ProgrammingLanguageParser::LanguageContext::getRuleIndex() const {
  return ProgrammingLanguageParser::RuleLanguage;
}

void ProgrammingLanguageParser::LanguageContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLanguage(this);
}

void ProgrammingLanguageParser::LanguageContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLanguage(this);
}

ProgrammingLanguageParser::LanguageContext* ProgrammingLanguageParser::language() {
  LanguageContext *_localctx = _tracker.createInstance<LanguageContext>(_ctx, getState());
  enterRule(_localctx, 0, ProgrammingLanguageParser::RuleLanguage);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(8);
    algorithm(0);
    setState(9);
    match(ProgrammingLanguageParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AlgorithmContext ------------------------------------------------------------------

ProgrammingLanguageParser::AlgorithmContext::AlgorithmContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

ProgrammingLanguageParser::Classical_statementContext* ProgrammingLanguageParser::AlgorithmContext::classical_statement() {
  return getRuleContext<ProgrammingLanguageParser::Classical_statementContext>(0);
}

tree::TerminalNode* ProgrammingLanguageParser::AlgorithmContext::UNITARY() {
  return getToken(ProgrammingLanguageParser::UNITARY, 0);
}

ProgrammingLanguageParser::QlistContext* ProgrammingLanguageParser::AlgorithmContext::qlist() {
  return getRuleContext<ProgrammingLanguageParser::QlistContext>(0);
}

tree::TerminalNode* ProgrammingLanguageParser::AlgorithmContext::ID() {
  return getToken(ProgrammingLanguageParser::ID, 0);
}

std::vector<ProgrammingLanguageParser::AlgorithmContext *> ProgrammingLanguageParser::AlgorithmContext::algorithm() {
  return getRuleContexts<ProgrammingLanguageParser::AlgorithmContext>();
}

ProgrammingLanguageParser::AlgorithmContext* ProgrammingLanguageParser::AlgorithmContext::algorithm(size_t i) {
  return getRuleContext<ProgrammingLanguageParser::AlgorithmContext>(i);
}

tree::TerminalNode* ProgrammingLanguageParser::AlgorithmContext::REALNUM() {
  return getToken(ProgrammingLanguageParser::REALNUM, 0);
}


size_t ProgrammingLanguageParser::AlgorithmContext::getRuleIndex() const {
  return ProgrammingLanguageParser::RuleAlgorithm;
}

void ProgrammingLanguageParser::AlgorithmContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAlgorithm(this);
}

void ProgrammingLanguageParser::AlgorithmContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAlgorithm(this);
}


ProgrammingLanguageParser::AlgorithmContext* ProgrammingLanguageParser::algorithm() {
   return algorithm(0);
}

ProgrammingLanguageParser::AlgorithmContext* ProgrammingLanguageParser::algorithm(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  ProgrammingLanguageParser::AlgorithmContext *_localctx = _tracker.createInstance<AlgorithmContext>(_ctx, parentState);
  ProgrammingLanguageParser::AlgorithmContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 2;
  enterRecursionRule(_localctx, 2, ProgrammingLanguageParser::RuleAlgorithm, precedence);

    

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
    setState(31);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case ProgrammingLanguageParser::T__0: {
        setState(12);
        match(ProgrammingLanguageParser::T__0);
        break;
      }

      case ProgrammingLanguageParser::ID: {
        setState(13);
        classical_statement();
        break;
      }

      case ProgrammingLanguageParser::UNITARY: {
        setState(14);
        match(ProgrammingLanguageParser::UNITARY);
        setState(15);
        match(ProgrammingLanguageParser::T__1);
        setState(16);
        qlist();
        setState(17);
        match(ProgrammingLanguageParser::T__2);
        break;
      }

      case ProgrammingLanguageParser::T__4: {
        setState(19);
        match(ProgrammingLanguageParser::T__4);
        setState(20);
        match(ProgrammingLanguageParser::T__1);
        setState(21);
        match(ProgrammingLanguageParser::ID);
        setState(22);
        match(ProgrammingLanguageParser::T__2);
        setState(23);
        match(ProgrammingLanguageParser::T__5);
        setState(24);
        algorithm(0);
        setState(25);
        match(ProgrammingLanguageParser::T__6);
        setState(26);
        match(ProgrammingLanguageParser::T__7);
        setState(27);
        match(ProgrammingLanguageParser::T__5);
        setState(28);
        algorithm(0);
        setState(29);
        match(ProgrammingLanguageParser::T__6);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(42);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(40);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<AlgorithmContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleAlgorithm);
          setState(33);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(34);
          match(ProgrammingLanguageParser::T__3);
          setState(35);
          algorithm(4);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<AlgorithmContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleAlgorithm);
          setState(36);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(37);
          match(ProgrammingLanguageParser::T__8);
          setState(38);
          match(ProgrammingLanguageParser::REALNUM);
          setState(39);
          match(ProgrammingLanguageParser::T__8);
          break;
        }

        default:
          break;
        } 
      }
      setState(44);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- Classical_statementContext ------------------------------------------------------------------

ProgrammingLanguageParser::Classical_statementContext::Classical_statementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ProgrammingLanguageParser::Classical_statementContext::ID() {
  return getTokens(ProgrammingLanguageParser::ID);
}

tree::TerminalNode* ProgrammingLanguageParser::Classical_statementContext::ID(size_t i) {
  return getToken(ProgrammingLanguageParser::ID, i);
}


size_t ProgrammingLanguageParser::Classical_statementContext::getRuleIndex() const {
  return ProgrammingLanguageParser::RuleClassical_statement;
}

void ProgrammingLanguageParser::Classical_statementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassical_statement(this);
}

void ProgrammingLanguageParser::Classical_statementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassical_statement(this);
}

ProgrammingLanguageParser::Classical_statementContext* ProgrammingLanguageParser::classical_statement() {
  Classical_statementContext *_localctx = _tracker.createInstance<Classical_statementContext>(_ctx, getState());
  enterRule(_localctx, 4, ProgrammingLanguageParser::RuleClassical_statement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(60);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(45);
      match(ProgrammingLanguageParser::ID);
      setState(46);
      match(ProgrammingLanguageParser::T__9);
      setState(47);
      match(ProgrammingLanguageParser::T__10);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(48);
      match(ProgrammingLanguageParser::ID);
      setState(49);
      match(ProgrammingLanguageParser::T__9);
      setState(50);
      match(ProgrammingLanguageParser::T__11);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(51);
      match(ProgrammingLanguageParser::ID);
      setState(52);
      match(ProgrammingLanguageParser::T__9);
      setState(53);
      match(ProgrammingLanguageParser::ID);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(54);
      match(ProgrammingLanguageParser::ID);
      setState(55);
      match(ProgrammingLanguageParser::T__9);
      setState(56);
      match(ProgrammingLanguageParser::T__12);
      setState(57);
      match(ProgrammingLanguageParser::T__1);
      setState(58);
      match(ProgrammingLanguageParser::ID);
      setState(59);
      match(ProgrammingLanguageParser::T__2);
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

//----------------- QlistContext ------------------------------------------------------------------

ProgrammingLanguageParser::QlistContext::QlistContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> ProgrammingLanguageParser::QlistContext::ID() {
  return getTokens(ProgrammingLanguageParser::ID);
}

tree::TerminalNode* ProgrammingLanguageParser::QlistContext::ID(size_t i) {
  return getToken(ProgrammingLanguageParser::ID, i);
}


size_t ProgrammingLanguageParser::QlistContext::getRuleIndex() const {
  return ProgrammingLanguageParser::RuleQlist;
}

void ProgrammingLanguageParser::QlistContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterQlist(this);
}

void ProgrammingLanguageParser::QlistContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<ProgrammingLanguageListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitQlist(this);
}

ProgrammingLanguageParser::QlistContext* ProgrammingLanguageParser::qlist() {
  QlistContext *_localctx = _tracker.createInstance<QlistContext>(_ctx, getState());
  enterRule(_localctx, 6, ProgrammingLanguageParser::RuleQlist);
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
    setState(62);
    match(ProgrammingLanguageParser::ID);
    setState(67);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == ProgrammingLanguageParser::T__13) {
      setState(63);
      match(ProgrammingLanguageParser::T__13);
      setState(64);
      match(ProgrammingLanguageParser::ID);
      setState(69);
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

bool ProgrammingLanguageParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 1: return algorithmSempred(antlrcpp::downCast<AlgorithmContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool ProgrammingLanguageParser::algorithmSempred(AlgorithmContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 3);
    case 1: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void ProgrammingLanguageParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  programminglanguageParserInitialize();
#else
  ::antlr4::internal::call_once(programminglanguageParserOnceFlag, programminglanguageParserInitialize);
#endif
}
