
#ifndef LR_PARSER_H_
#define LR_PARSER_H_

struct LRParser;
typedef struct LRParser LRParser;

struct LRParseTree;
typedef struct LRParseTree LRParseTree;

typedef unsigned LRSymbolId;
typedef unsigned LRRuleId;


#if 0
LRParser * lrParserNew(const char * const configFilePath, const char ** outputErrorMessage);
void lrParserFree(LRParser * parser);
void lrParserPrint(const LRParser * const parser);

unsigned lrParserGetNbSymbols(const LRParser * const parser);
unsigned lrParserGetNbTerminalSymbols(const LRParser * const parser);
unsigned lrParserGetNbNonTerminalSymbols(const LRParser * const parser);
const char * lrParserGetSymbolName(const LRParser * const parser, const LRSymbolId symbolId);
const char * lrParserGetTerminalSymbolRegex(const LRParser * const parser, const LRSymbolId symbolId);
LRSymbolId lrParserGetSymbolId(const LRParser * const parser, const char * const symbolName);
int lrParserSymbolIsTerminal(const LRParser * const parser, const LRSymbolId symbolId);

unsigned lrParserGetNbRules(const LRParser * const parser);
const char * lrParserGetRuleName(const LRParser * const parser, const LRRuleId ruleId);
LRSymbolId lrParserGetRuleLeftSymbolId(const LRParser * const parser, const LRRuleId ruleId);
unsigned lrParserGetRuleNbRightSymbols(const LRParser * const parser, const LRRuleId ruleId);
const LRSymbolId * lrParserGetRuleRightSymbolIds(const LRParser * const parser, const LRRuleId ruleId);
LRRuleId lrParserGetRuleId(const LRParser * const parser, const char * const ruleName);

LRParseTree * lrParserParseStr(LRParser * const parser, const char * const str, const char ** outputErrorMessage);
LRParseTree * lrParserParseFile(LRParser * const parser, const char * const filePath, const char ** outputErrorMessage);

void lrParseTreeFree(const LRParser * const parser, LRParseTree * tree);

void lrParseTreePrint(const LRParser * const parser, const LRParseTree * const tree);


int lrParseTreeSymbolIsTerminal(const LRParser * const parser, const LRParseTree * const tree);
LRSymbolId lrParseTreeGetSymbolId(const LRParser * const parser, const LRParseTree * const tree);
const char * lrParseTreeGetSymbolName(const LRParser * const parser, const LRParseTree * const tree);
const char * lrParseTreeGetTerminalSymbolString(const LRParser * const parser, const LRParseTree * const tree);

LRRuleId lrParseTreeGetRuleId(const LRParser * const parser, const LRParseTree * const tree);
const char * lrParseTreeGetRuleName(const LRParser * const parser, const LRParseTree * const tree);
unsigned lrParseTreeGetNbSons(const LRParser * const parser, const LRParseTree * const tree);
const LRParseTree * const * lrParseTreeGetSons(const LRParser * const parser, const LRParseTree * const tree);
#endif


LRSymbolId lrParseTreeGetSymbolId(const LRParser * const parser, const LRParseTree * const tree);
const char * lrParseTreeGetTerminalSymbolString(const LRParser * const parser, const LRParseTree * const tree);
LRRuleId lrParseTreeGetRuleId(const LRParser * const parser, const LRParseTree * const tree);
int lrParseTreeSymbolIsTerminal(const LRParser * const parser, const LRParseTree * const tree);

#define LR_SYMBOL(number, name, regex, instr)\
		case number:\
		if (create){\
			lrParserAddTerminalSymbol(parser, number, name, regex);\
		} else {\
			{instr}\
			break;\
		}\



#define LR_RULE(number, nonTerminalString, derivationString, instr)\
		case number:\
		if (create){\
			lrParserAddRule(parser, number, nonTerminalString, derivationString);\
		} else {\
			{instr}\
			break;\
		}\

typedef void * (LRParsingFunction)(LRParser * const parser, unsigned symbolId, int isTerminal,
		char * symbolString, unsigned ruleId, void ** values, int create);

#define LR_PARSING_FUNCTION(func_name, instrTerminal, instrRules)\
		\
		void * func_name(LRParser * const parser, unsigned symbolId, int isTerminal,\
						 char * symbolString, unsigned ruleId, void ** values, int create)\
						 {\
			if (create){\
				goto INIT_SYMBOL;\
			}\
			if (isTerminal){\
				switch (symbolId){\
				INIT_SYMBOL:\
				{instrTerminal}\
				if (create){\
					goto INIT_RULE;\
				}\
				}\
			} else {\
				switch (ruleId){\
				INIT_RULE:\
				{instrRules}\
				}\
			}\
			return NULL;\
						 }



LRParser * lrParserNewFromFunc(LRParsingFunction function);



LRParseTree * lrParserParseStr(LRParser * const parser, const char * const str, const char ** outputErrorMessage);
LRParseTree * lrParserParseFile(LRParser * const parser, const char * const filePath, const char ** outputErrorMessage);
void lrParseTreeFree(const LRParser * const parser, LRParseTree * tree);
void * lrParserApplyFuncToParseTree(LRParser * const parser, const LRParseTree * const tree);
void lrParserFree(LRParser * parser);
void lrParserPrint(const LRParser * const parser);

void lrParseTreePrint(const LRParser * const parser, const LRParseTree * const tree);


void lrParserAddTerminalSymbol(LRParser * const parser, const unsigned symbolId, const char * const symbolName, const char * const symbolRegex);
void lrParserAddRule(LRParser * const parser, const unsigned ruleId, const char * const nonTerminalName, const char * const derivationString);


#endif /* LR_PARSER_H_ */
