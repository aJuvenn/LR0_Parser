/*
 * parser.c
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#include <stdlib.h>
#include "parser.h"
#include "parse_config.h"


LRParser * lrParserNew(const char * const configFilePath)
{
	LRParser *  output = malloc(sizeof(LRParser));

	if (output == NULL){
		goto ERR0;
	}

	GrammarDescription * conf = parseConfigFile(configFilePath);

	if (conf == NULL){
		goto ERR1;
	}

	output->cursor = NULL;
	output->nbTokens = conf->nbTokens;
	output->tokenAutomatons = malloc(output->nbTokens * sizeof(cdfa__automaton *));

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

	return output;

	ERR4: // TODO free grammar

	ERR3:
	for (unsigned i = 0 ; i < tokenId ; i++){
		cdfa__free_automaton(output->tokenAutomatons[i]);
	}

	free(output->tokenAutomatons);

	ERR2: // TODO free conf
	ERR1: free(output);
	ERR0: return NULL;
}






LRParseTree * lrParserParseStr(LRParser * parser, const char * const str);

LRParseTree * lrParserParseFile(LRParser * parser, const char * const filePath);
