/*
 * lr_parser_basis.c
 *
 *  Created on: 21 janv. 2021
 *      Author: ajuvenn
 */

#include "../lr_include.h"




LRParser * lrParserNewFromFunc(LRParsingFunction function)
{
	LRParser * output = malloc(sizeof(LRParser));
	if (output == NULL) return NULL;

	LRTmpConfig * tmpConfig = malloc(sizeof(LRTmpConfig));
	if (tmpConfig == NULL) goto ERR1;

	tmpConfig->nbRules = 0;
	tmpConfig->nbSymbols = 0;
	output->tmpConfig = tmpConfig;
	output->parsingFunction = function;
	function(output, 0, 0, NULL, 0, NULL, 1);

	int ret = lrTmpConfigSortingAndVerification(tmpConfig);
	if (ret != EXIT_SUCCESS) goto ERR2;

	LRConfig * config = lrConfigNew(tmpConfig);
	if (config == NULL) goto ERR2;
	output->config = config;

	output->nbTokens = config->nbTokens;
	output->tokenAutomatons = malloc(output->nbTokens * sizeof(cdfa__automaton *));
	if (output->tokenAutomatons == NULL) goto ERR3;
	output->skippedAutomatonId = config->skippedTokenId;

	unsigned tokenId;

	for (tokenId = 0 ; tokenId < output->nbTokens ; tokenId++){
		output->tokenAutomatons[tokenId] = cdfa__expression_recognizing_automaton(config->tokens[tokenId].regex);
		if (output->tokenAutomatons[tokenId] == NULL) goto ERR4;
	}

	output->grammar = lrGrammarFromConfig(config);
	if (output->grammar == NULL) goto ERR4;

	output->transition = lrTransitionMatrixFromGrammar(output->grammar);
	if (output->transition == NULL) goto ERR5;

	free(tmpConfig);
	output->tmpConfig = NULL;

	return output;

	ERR5: lrGrammarFree(output->grammar);
	ERR4:
	for (unsigned i = 0 ; i < tokenId ; i++){
		cdfa__free_automaton(output->tokenAutomatons[i]);
	}

	free(output->tokenAutomatons);

	ERR3: lrConfigFree(config);
	ERR2: free(tmpConfig);
	ERR1: free(output);
	return NULL;
}







void lrParserAddTerminalSymbol(LRParser * const parser, const unsigned symbolId, const char * const symbolName, const char * const symbolRegex)
{
	if (parser == NULL || parser->tmpConfig == NULL){
		return;
	}

	const unsigned nbSymbols = parser->tmpConfig->nbSymbols;

	if (nbSymbols == LR_PARSER_MAX_NB_SYMBOLS){
		return;
	}

	printf("Adding [%u] %s := %s\n", symbolId, symbolName, symbolRegex);

	parser->tmpConfig->symbols[nbSymbols].symbolId = symbolId;
	parser->tmpConfig->symbols[nbSymbols].symbolName = symbolName;
	parser->tmpConfig->symbols[nbSymbols].symbolRegex = symbolRegex;
	parser->tmpConfig->nbSymbols++;
}

void lrParserAddRule(LRParser * const parser, const unsigned ruleId, const char * const nonTerminalName, const char * const derivationString)
{
	if (parser == NULL || parser->tmpConfig == NULL){
		return;
	}

	const unsigned nbRules = parser->tmpConfig->nbRules;

	if (nbRules == LR_PARSER_MAX_NB_SYMBOLS){
		return;
	}

	printf("Adding [%u] %s -> %s\n", ruleId, nonTerminalName, derivationString);

	parser->tmpConfig->rules[nbRules].ruleId = ruleId;
	parser->tmpConfig->rules[nbRules].nonTerminalName = nonTerminalName;
	parser->tmpConfig->rules[nbRules].derivationString = derivationString;
	parser->tmpConfig->nbRules++;
}






void lrParserFree(LRParser * parser)
{
	for (unsigned i = 0 ; i < parser->nbTokens ; i++){
		cdfa__free_automaton(parser->tokenAutomatons[i]);
	}

	free(parser->tokenAutomatons);
	lrGrammarFree(parser->grammar);
	lrTransitionMatrixFree(parser->transition);
	free(parser);
}

void lrParserPrint(const LRParser * const parser)
{
	lrGrammarPrint(parser->grammar);
	lrTransitionMatrixPrint(parser->transition, parser->grammar);
}

unsigned lrParserGetNbSymbols(const LRParser * const parser)
{
	return parser->grammar->nbSymbols;
}

unsigned lrParserGetNbTerminalSymbols(const LRParser * const parser)
{
	return parser->grammar->nbTerminals;
}

unsigned lrParserGetNbNonTerminalSymbols(const LRParser * const parser)
{
	return parser->grammar->nbNonTerminal;
}

const char * lrParserGetSymbolName(const LRParser * const parser, const LRSymbolId symbolId)
{
	if (symbolId >= parser->grammar->nbSymbols){
		return NULL;
	}

	return parser->grammar->symbolNames[symbolId];
}

const char * lrParserGetTerminalSymbolRegex(const LRParser * const parser, const LRSymbolId symbolId)
{
	/* TODO: implement symbol regex */
	return NULL;
}

LRSymbolId lrParserGetSymbolId(const LRParser * const parser, const char * const symbolName)
{
	const unsigned nbSymbols = parser->grammar->nbSymbols;
	for (unsigned i = 0 ; i < nbSymbols ; i++){
		if (!strcmp(symbolName, parser->grammar->symbolNames[i])){
			return (LRSymbolId) i;
		}
	}
	return (LRSymbolId) -1;
}

int lrParserSymbolIsTerminal(const LRParser * const parser, const LRSymbolId symbolId)
{
	return symbolId >= parser->grammar->nbNonTerminal;
}

unsigned lrParserGetNbRules(const LRParser * const parser)
{
	return parser->grammar->nbRules;
}

const char * lrParserGetRuleName(const LRParser * const parser, const LRRuleId ruleId)
{
	if (ruleId >= parser->grammar->nbRules){
		return NULL;
	}
	return parser->grammar->ruleNames[ruleId];
}

LRSymbolId lrParserGetRuleLeftSymbolId(const LRParser * const parser, const LRRuleId ruleId)
{
	if (ruleId >= parser->grammar->nbRules){
		return (LRSymbolId) -1;
	}
	return (LRSymbolId) parser->grammar->leftRules[ruleId];
}

unsigned lrParserGetRuleNbRightSymbols(const LRParser * const parser, const LRRuleId ruleId)
{
	if (ruleId >= parser->grammar->nbRules){
		return -1;
	}
	return parser->grammar->rightRuleSizes[ruleId];
}

const LRSymbolId * lrParserGetRuleRightSymbolIds(const LRParser * const parser, const LRRuleId ruleId)
{
	if (ruleId >= parser->grammar->nbRules){
		return NULL;
	}
	return (LRSymbolId *) parser->grammar->rightRules[ruleId];
}

LRRuleId lrParserGetRuleId(const LRParser * const parser, const char * const ruleName)
{
	for (unsigned i = 0 ; i < parser->grammar->nbRules ; i++){
		if (!strcmp(ruleName, parser->grammar->ruleNames[i])){
			return (LRRuleId) i;
		}
	}
	return (LRRuleId) -1;
}

