
// Generated from grammars/assertions/Assertion.g4 by ANTLR 4.13.2


#include "AssertionLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct AssertionLexerStaticData final {
  AssertionLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  AssertionLexerStaticData(const AssertionLexerStaticData&) = delete;
  AssertionLexerStaticData(AssertionLexerStaticData&&) = delete;
  AssertionLexerStaticData& operator=(const AssertionLexerStaticData&) = delete;
  AssertionLexerStaticData& operator=(AssertionLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag assertionlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<AssertionLexerStaticData> assertionlexerLexerStaticData = nullptr;

void assertionlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (assertionlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(assertionlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<AssertionLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "T__9", "T__10", "Row", "BINARYSTRING", "BList", "QList", "RP", "ROP", 
      "BOP", "MOP", "REALNUM", "ID", "INT", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,23,163,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,2,1,2,1,3,1,3,1,4,1,4,1,
  	5,1,5,1,6,1,6,1,7,1,7,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,11,1,11,1,11,
  	1,11,5,11,79,8,11,10,11,12,11,82,9,11,1,11,1,11,1,12,1,12,1,12,5,12,89,
  	8,12,10,12,12,12,92,9,12,3,12,94,8,12,1,13,1,13,1,13,5,13,99,8,13,10,
  	13,12,13,102,9,13,1,14,1,14,1,14,5,14,107,8,14,10,14,12,14,110,9,14,1,
  	15,1,15,1,15,1,15,1,15,3,15,117,8,15,1,16,1,16,3,16,121,8,16,1,17,1,17,
  	1,17,1,17,1,17,3,17,128,8,17,1,18,1,18,1,19,4,19,133,8,19,11,19,12,19,
  	134,1,19,1,19,4,19,139,8,19,11,19,12,19,140,3,19,143,8,19,1,20,1,20,5,
  	20,147,8,20,10,20,12,20,150,9,20,1,21,4,21,153,8,21,11,21,12,21,154,1,
  	22,4,22,158,8,22,11,22,12,22,159,1,22,1,22,0,0,23,1,1,3,2,5,3,7,4,9,5,
  	11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,35,
  	18,37,19,39,20,41,21,43,22,45,23,1,0,6,2,0,38,38,124,124,2,0,42,43,45,
  	45,1,0,48,57,3,0,65,90,95,95,97,122,4,0,48,57,65,90,95,95,97,122,3,0,
  	9,10,13,13,32,32,178,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,
  	0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,
  	1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,
  	0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,0,0,
  	0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,1,47,1,0,0,0,3,50,1,0,0,0,5,54,
  	1,0,0,0,7,56,1,0,0,0,9,58,1,0,0,0,11,60,1,0,0,0,13,62,1,0,0,0,15,64,1,
  	0,0,0,17,68,1,0,0,0,19,70,1,0,0,0,21,72,1,0,0,0,23,74,1,0,0,0,25,93,1,
  	0,0,0,27,95,1,0,0,0,29,103,1,0,0,0,31,116,1,0,0,0,33,120,1,0,0,0,35,127,
  	1,0,0,0,37,129,1,0,0,0,39,132,1,0,0,0,41,144,1,0,0,0,43,152,1,0,0,0,45,
  	157,1,0,0,0,47,48,5,111,0,0,48,49,5,114,0,0,49,2,1,0,0,0,50,51,5,97,0,
  	0,51,52,5,110,0,0,52,53,5,100,0,0,53,4,1,0,0,0,54,55,5,33,0,0,55,6,1,
  	0,0,0,56,57,5,40,0,0,57,8,1,0,0,0,58,59,5,41,0,0,59,10,1,0,0,0,60,61,
  	5,80,0,0,61,12,1,0,0,0,62,63,5,61,0,0,63,14,1,0,0,0,64,65,5,84,0,0,65,
  	66,5,114,0,0,66,67,5,40,0,0,67,16,1,0,0,0,68,69,5,91,0,0,69,18,1,0,0,
  	0,70,71,5,93,0,0,71,20,1,0,0,0,72,73,5,44,0,0,73,22,1,0,0,0,74,75,5,91,
  	0,0,75,80,3,39,19,0,76,77,5,44,0,0,77,79,3,39,19,0,78,76,1,0,0,0,79,82,
  	1,0,0,0,80,78,1,0,0,0,80,81,1,0,0,0,81,83,1,0,0,0,82,80,1,0,0,0,83,84,
  	5,93,0,0,84,24,1,0,0,0,85,94,5,48,0,0,86,90,5,49,0,0,87,89,2,48,49,0,
  	88,87,1,0,0,0,89,92,1,0,0,0,90,88,1,0,0,0,90,91,1,0,0,0,91,94,1,0,0,0,
  	92,90,1,0,0,0,93,85,1,0,0,0,93,86,1,0,0,0,94,26,1,0,0,0,95,100,3,41,20,
  	0,96,97,5,44,0,0,97,99,3,41,20,0,98,96,1,0,0,0,99,102,1,0,0,0,100,98,
  	1,0,0,0,100,101,1,0,0,0,101,28,1,0,0,0,102,100,1,0,0,0,103,108,3,41,20,
  	0,104,105,5,44,0,0,105,107,3,41,20,0,106,104,1,0,0,0,107,110,1,0,0,0,
  	108,106,1,0,0,0,108,109,1,0,0,0,109,30,1,0,0,0,110,108,1,0,0,0,111,117,
  	2,61,62,0,112,113,5,62,0,0,113,117,5,61,0,0,114,115,5,60,0,0,115,117,
  	5,61,0,0,116,111,1,0,0,0,116,112,1,0,0,0,116,114,1,0,0,0,117,32,1,0,0,
  	0,118,121,3,37,18,0,119,121,5,47,0,0,120,118,1,0,0,0,120,119,1,0,0,0,
  	121,34,1,0,0,0,122,123,5,62,0,0,123,128,5,62,0,0,124,125,5,60,0,0,125,
  	128,5,60,0,0,126,128,7,0,0,0,127,122,1,0,0,0,127,124,1,0,0,0,127,126,
  	1,0,0,0,128,36,1,0,0,0,129,130,7,1,0,0,130,38,1,0,0,0,131,133,7,2,0,0,
  	132,131,1,0,0,0,133,134,1,0,0,0,134,132,1,0,0,0,134,135,1,0,0,0,135,142,
  	1,0,0,0,136,138,5,46,0,0,137,139,7,2,0,0,138,137,1,0,0,0,139,140,1,0,
  	0,0,140,138,1,0,0,0,140,141,1,0,0,0,141,143,1,0,0,0,142,136,1,0,0,0,142,
  	143,1,0,0,0,143,40,1,0,0,0,144,148,7,3,0,0,145,147,7,4,0,0,146,145,1,
  	0,0,0,147,150,1,0,0,0,148,146,1,0,0,0,148,149,1,0,0,0,149,42,1,0,0,0,
  	150,148,1,0,0,0,151,153,7,2,0,0,152,151,1,0,0,0,153,154,1,0,0,0,154,152,
  	1,0,0,0,154,155,1,0,0,0,155,44,1,0,0,0,156,158,7,5,0,0,157,156,1,0,0,
  	0,158,159,1,0,0,0,159,157,1,0,0,0,159,160,1,0,0,0,160,161,1,0,0,0,161,
  	162,6,22,0,0,162,46,1,0,0,0,15,0,80,90,93,100,108,116,120,127,134,140,
  	142,148,154,159,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  assertionlexerLexerStaticData = std::move(staticData);
}

}

AssertionLexer::AssertionLexer(CharStream *input) : Lexer(input) {
  AssertionLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *assertionlexerLexerStaticData->atn, assertionlexerLexerStaticData->decisionToDFA, assertionlexerLexerStaticData->sharedContextCache);
}

AssertionLexer::~AssertionLexer() {
  delete _interpreter;
}

std::string AssertionLexer::getGrammarFileName() const {
  return "Assertion.g4";
}

const std::vector<std::string>& AssertionLexer::getRuleNames() const {
  return assertionlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& AssertionLexer::getChannelNames() const {
  return assertionlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& AssertionLexer::getModeNames() const {
  return assertionlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& AssertionLexer::getVocabulary() const {
  return assertionlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView AssertionLexer::getSerializedATN() const {
  return assertionlexerLexerStaticData->serializedATN;
}

const atn::ATN& AssertionLexer::getATN() const {
  return *assertionlexerLexerStaticData->atn;
}




void AssertionLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  assertionlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(assertionlexerLexerOnceFlag, assertionlexerLexerInitialize);
#endif
}
