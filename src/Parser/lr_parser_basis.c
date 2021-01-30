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







void __lrParserAddTerminalSymbol(LRParser * const parser, const unsigned symbolId, const char * const symbolName, const char * const symbolRegex)
{
	if (parser == NULL || parser->tmpConfig == NULL){
		return;
	}

	const unsigned nbSymbols = parser->tmpConfig->nbSymbols;

	if (nbSymbols == LR_PARSER_MAX_NB_SYMBOLS){
		return;
	}

	parser->tmpConfig->symbols[nbSymbols].symbolId = symbolId;
	parser->tmpConfig->symbols[nbSymbols].symbolName = symbolName;
	parser->tmpConfig->symbols[nbSymbols].symbolRegex = symbolRegex;
	parser->tmpConfig->nbSymbols++;
}

void __lrParserAddRule(LRParser * const parser, const unsigned ruleId, const char * const nonTerminalName, const char * const derivationString)
{
	if (parser == NULL || parser->tmpConfig == NULL){
		return;
	}

	const unsigned nbRules = parser->tmpConfig->nbRules;

	if (nbRules == LR_PARSER_MAX_NB_SYMBOLS){
		return;
	}

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
	lrConfigFree(parser->config);
	free(parser);
}


void lrParserPrint(const LRParser * const parser)
{
	lrGrammarPrint(parser->grammar);
	lrTransitionMatrixPrint(parser->transition, parser->grammar);
}

