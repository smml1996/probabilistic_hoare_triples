
// Generated from grammars/PL/ProgrammingLanguage.g4 by ANTLR 4.13.2


#include "ProgrammingLanguageLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct ProgrammingLanguageLexerStaticData final {
  ProgrammingLanguageLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  ProgrammingLanguageLexerStaticData(const ProgrammingLanguageLexerStaticData&) = delete;
  ProgrammingLanguageLexerStaticData(ProgrammingLanguageLexerStaticData&&) = delete;
  ProgrammingLanguageLexerStaticData& operator=(const ProgrammingLanguageLexerStaticData&) = delete;
  ProgrammingLanguageLexerStaticData& operator=(ProgrammingLanguageLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag programminglanguagelexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<ProgrammingLanguageLexerStaticData> programminglanguagelexerLexerStaticData = nullptr;

void programminglanguagelexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (programminglanguagelexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(programminglanguagelexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<ProgrammingLanguageLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "T__9", "T__10", "T__11", "T__12", "T__13", "UNITARY", "REALNUM", 
      "ID"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,17,103,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,2,1,2,1,3,1,3,
  	1,4,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,7,1,7,1,7,1,8,1,8,1,9,1,9,1,10,
  	1,10,1,11,1,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,13,1,13,1,14,
  	1,14,5,14,79,8,14,10,14,12,14,82,9,14,1,15,4,15,85,8,15,11,15,12,15,86,
  	1,15,1,15,4,15,91,8,15,11,15,12,15,92,3,15,95,8,15,1,16,1,16,5,16,99,
  	8,16,10,16,12,16,102,9,16,0,0,17,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,
  	9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,1,0,3,3,0,65,90,95,
  	95,97,122,1,0,48,57,4,0,48,57,65,90,95,95,97,122,107,0,1,1,0,0,0,0,3,
  	1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,
  	0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,
  	1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,1,35,1,0,
  	0,0,3,40,1,0,0,0,5,42,1,0,0,0,7,44,1,0,0,0,9,46,1,0,0,0,11,49,1,0,0,0,
  	13,51,1,0,0,0,15,53,1,0,0,0,17,58,1,0,0,0,19,60,1,0,0,0,21,62,1,0,0,0,
  	23,64,1,0,0,0,25,66,1,0,0,0,27,74,1,0,0,0,29,76,1,0,0,0,31,84,1,0,0,0,
  	33,96,1,0,0,0,35,36,5,115,0,0,36,37,5,107,0,0,37,38,5,105,0,0,38,39,5,
  	112,0,0,39,2,1,0,0,0,40,41,5,40,0,0,41,4,1,0,0,0,42,43,5,41,0,0,43,6,
  	1,0,0,0,44,45,5,59,0,0,45,8,1,0,0,0,46,47,5,105,0,0,47,48,5,102,0,0,48,
  	10,1,0,0,0,49,50,5,123,0,0,50,12,1,0,0,0,51,52,5,125,0,0,52,14,1,0,0,
  	0,53,54,5,101,0,0,54,55,5,108,0,0,55,56,5,115,0,0,56,57,5,101,0,0,57,
  	16,1,0,0,0,58,59,5,37,0,0,59,18,1,0,0,0,60,61,5,61,0,0,61,20,1,0,0,0,
  	62,63,5,48,0,0,63,22,1,0,0,0,64,65,5,49,0,0,65,24,1,0,0,0,66,67,5,109,
  	0,0,67,68,5,101,0,0,68,69,5,97,0,0,69,70,5,115,0,0,70,71,5,117,0,0,71,
  	72,5,114,0,0,72,73,5,101,0,0,73,26,1,0,0,0,74,75,5,44,0,0,75,28,1,0,0,
  	0,76,80,7,0,0,0,77,79,7,0,0,0,78,77,1,0,0,0,79,82,1,0,0,0,80,78,1,0,0,
  	0,80,81,1,0,0,0,81,30,1,0,0,0,82,80,1,0,0,0,83,85,7,1,0,0,84,83,1,0,0,
  	0,85,86,1,0,0,0,86,84,1,0,0,0,86,87,1,0,0,0,87,94,1,0,0,0,88,90,5,46,
  	0,0,89,91,7,1,0,0,90,89,1,0,0,0,91,92,1,0,0,0,92,90,1,0,0,0,92,93,1,0,
  	0,0,93,95,1,0,0,0,94,88,1,0,0,0,94,95,1,0,0,0,95,32,1,0,0,0,96,100,7,
  	0,0,0,97,99,7,2,0,0,98,97,1,0,0,0,99,102,1,0,0,0,100,98,1,0,0,0,100,101,
  	1,0,0,0,101,34,1,0,0,0,102,100,1,0,0,0,6,0,80,86,92,94,100,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  programminglanguagelexerLexerStaticData = std::move(staticData);
}

}

ProgrammingLanguageLexer::ProgrammingLanguageLexer(CharStream *input) : Lexer(input) {
  ProgrammingLanguageLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *programminglanguagelexerLexerStaticData->atn, programminglanguagelexerLexerStaticData->decisionToDFA, programminglanguagelexerLexerStaticData->sharedContextCache);
}

ProgrammingLanguageLexer::~ProgrammingLanguageLexer() {
  delete _interpreter;
}

std::string ProgrammingLanguageLexer::getGrammarFileName() const {
  return "ProgrammingLanguage.g4";
}

const std::vector<std::string>& ProgrammingLanguageLexer::getRuleNames() const {
  return programminglanguagelexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& ProgrammingLanguageLexer::getChannelNames() const {
  return programminglanguagelexerLexerStaticData->channelNames;
}

const std::vector<std::string>& ProgrammingLanguageLexer::getModeNames() const {
  return programminglanguagelexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& ProgrammingLanguageLexer::getVocabulary() const {
  return programminglanguagelexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView ProgrammingLanguageLexer::getSerializedATN() const {
  return programminglanguagelexerLexerStaticData->serializedATN;
}

const atn::ATN& ProgrammingLanguageLexer::getATN() const {
  return *programminglanguagelexerLexerStaticData->atn;
}




void ProgrammingLanguageLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  programminglanguagelexerLexerInitialize();
#else
  ::antlr4::internal::call_once(programminglanguagelexerLexerOnceFlag, programminglanguagelexerLexerInitialize);
#endif
}
