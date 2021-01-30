
#ifndef LR_PARSER_H_
#define LR_PARSER_H_

struct LRParser;
typedef struct LRParser LRParser;


#define LR_SYMBOL(_symbolId, _symbolName, _symbolRegex, _instructions)\
		case _symbolId:\
		if (__lrInit){\
			__lrParserAddTerminalSymbol(__lrParser, _symbolId, _symbolName, _symbolRegex);\
		} else {\
			{_instructions}\
			break;\
		}\



#define LR_RULE(_ruleId, _leftRuleSymbol, _rightRuleDerivation, _instructions)\
		case _ruleId:\
		if (__lrInit){\
			__lrParserAddRule(__lrParser, _ruleId, _leftRuleSymbol, _rightRuleDerivation);\
		} else {\
			{_instructions}\
			break;\
		}\



#define LR_PARSING_FUNCTION(_functionName, _terminalSymbolDefinitions, _ruleDefinitions)\
		\
		void * _functionName(LRParser * const __lrParser, const unsigned __lrSymbolId,\
							 const int __lrSymbolIsTerminal, const char * const lrSymbol, \
							 const unsigned __lrRuleId, void ** const lrValues, const int __lrInit)\
							 {\
			if (__lrInit){\
				goto __LR_INIT_SYMBOL_LABEL;\
			}\
			if (__lrSymbolIsTerminal){\
				switch (__lrSymbolId){\
				__LR_INIT_SYMBOL_LABEL:\
				{_terminalSymbolDefinitions}\
				if (__lrInit){\
					goto __LR_INIT_RULE_LABEL;\
				}\
				}\
			} else {\
				switch (__lrRuleId){\
				__LR_INIT_RULE_LABEL:\
				{_ruleDefinitions}\
				}\
			}\
			return (void *) 0;\
							 }


typedef void * (LRParsingFunction)(LRParser * const, const unsigned, const int, const char * const,
		const unsigned, void ** const, const int);

LRParser * lrParserNewFromFunc(LRParsingFunction function);
void lrParserPrint(const LRParser * const parser);
void * lrParserParseFile(LRParser * const parser, const char * const filePath, const char ** outputErrorMessage);
void * lrParserParseString(LRParser * const parser, const char * const str, const char ** outputErrorMessage);
void lrParserFree(LRParser * parser);


void __lrParserAddTerminalSymbol(LRParser * const parser, const unsigned symbolId, const char * const symbolName, const char * const symbolRegex);
void __lrParserAddRule(LRParser * const parser, const unsigned ruleId, const char * const nonTerminalName, const char * const derivationString);


#endif /* LR_PARSER_H_ */
