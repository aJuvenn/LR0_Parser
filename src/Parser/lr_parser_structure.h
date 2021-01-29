/*
 * parser.h
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_PARSER_STRUCTURE_H_
#define LR_PARSER_STRUCTURE_H_

#include "../lr_include.h"

#define LR_PARSER_MAX_DEPTH 1024

struct LRParseTree
{
	int isLeaf;
	unsigned symbolId;

	union {

		struct {
			unsigned ruleId;
			unsigned nbSons;
			struct LRParseTree ** sons;
		} node;

		struct {
			char * tokenData;
		} leaf;
	};

};

struct LRParser;
typedef void * (LRParsingFunction)(struct LRParser * const parser, unsigned symbolId, int isTerminal,
		char * symbolString, unsigned ruleId, void ** values, int create);

struct LRParser
{
	LRParsingFunction * parsingFunction;
	LRTmpConfig * tmpConfig;
	LRConfig * config;

	unsigned nbTokens;
	unsigned skippedAutomatonId;
	cdfa__automaton ** tokenAutomatons;

	LRGrammar * grammar;
	LRTransitionMatrix * transition;
};


#endif /* LR_PARSER_STRUCTURE_H_ */
