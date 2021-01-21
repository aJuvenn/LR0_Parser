/*
 * lr_parser_basis.c
 *
 *  Created on: 21 janv. 2021
 *      Author: ajuvenn
 */

#include "../lr_include.h"


LRParser * lrParserNew(const char * const configFilePath, const char ** outputErrorMessage)
{
	*outputErrorMessage = NULL; /* TODO: fill error message */
	LRParser *  output = malloc(sizeof(LRParser));

	if (output == NULL){
		goto ERR0;
	}

	lrFileConfig * conf = lrFileConfigParse(configFilePath);

	if (conf == NULL){
		goto ERR1;
	}

	output->nbTokens = conf->nbTokens;
	output->tokenAutomatons = malloc(output->nbTokens * sizeof(cdfa__automaton *));
	output->skippedAutomatonId = conf->skippedToken;

	if (output->tokenAutomatons == NULL){
		goto ERR2;
	}

	unsigned tokenId;

	for (tokenId = 0 ; tokenId < output->nbTokens ; tokenId++){

		output->tokenAutomatons[tokenId] = cdfa__expression_recognizing_automaton(conf->tokenRegexp[tokenId]);

		if (output->tokenAutomatons[tokenId] == NULL){
			goto ERR3;
		}
	}

	output->grammar = lrGrammarFromDescr(conf);

	if (output->grammar == NULL){
		goto ERR3;
	}

	output->transition = lrTransitionMatrixFromGrammar(output->grammar);

	if (output->transition == NULL){
		goto ERR4;
	}

	lrFileConfigFree(conf);

	return output;

	ERR4: lrGrammarFree(output->grammar);

	ERR3:
	for (unsigned i = 0 ; i < tokenId ; i++){
		cdfa__free_automaton(output->tokenAutomatons[i]);
	}

	free(output->tokenAutomatons);

	ERR2: lrFileConfigFree(conf);
	ERR1: free(output);
	ERR0: return NULL;
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
	/* TODO: implement parser rule name */
	return NULL;
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
	/* TODO: implement parser get rule id */
	return (LRRuleId) -1;
}

