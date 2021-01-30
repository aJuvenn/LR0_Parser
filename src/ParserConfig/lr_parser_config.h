/*
 * parse_config.h
 *
 *  Created on: 26 févr. 2020
 *      Author: ajuvenn
 */

#ifndef PARSE_CONFIG_H_
#define PARSE_CONFIG_H_

#include "../lr_include.h"

#define LR_PARSER_MAX_NB_SYMBOLS 1024
#define LR_PARSER_MAX_NB_RULES 2048


typedef struct LRParserTmpConfigSymbols
{
	unsigned symbolId;
	const char * symbolName;
	const char * symbolRegex;

} LRTmpConfigSymbol;


typedef struct LRParserTmpConfigRules
{
	unsigned ruleId;
	const char * nonTerminalName;
	const char * derivationString;

} LRTmpConfigRule;


typedef struct LRTmpConfig
{
	unsigned nbSymbols;
	LRTmpConfigSymbol symbols[LR_PARSER_MAX_NB_SYMBOLS];

	unsigned nbRules;
	LRTmpConfigRule rules[LR_PARSER_MAX_NB_RULES];

} LRTmpConfig;



typedef struct LRConfigToken
{
	const char * name;
	const char * regex;
} LRConfigToken;


typedef struct LRConfigRule
{
	const char * leftMember;
	unsigned nbRightMembers;
	char ** rightMembers;
} LRConfigRule;


typedef struct LRConfig
{
	unsigned nbTokens;
	LRConfigToken * tokens;
	unsigned skippedTokenId;

	unsigned nbGrammarRules;
	LRConfigRule * grammarRules;

} LRConfig;


LRConfig * lrConfigNew(LRTmpConfig * tmpConfig);

void lrConfigFree(LRConfig * config);

int lrTmpConfigSortingAndVerification(LRTmpConfig * const tmpConfig);


#endif /* PARSE_CONFIG_H_ */
