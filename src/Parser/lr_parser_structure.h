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

typedef struct LRParseTree
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

} LRParseTree;


typedef struct LRParser
{
	LRParsingFunction * parsingFunction;
	LRTmpConfig * tmpConfig;
	LRConfig * config;

	unsigned nbTokens;
	unsigned skippedAutomatonId;
	cdfa__automaton ** tokenAutomatons;

	LRGrammar * grammar;
	LRTransitionMatrix * transition;
} LRParser;


LRParseTree * lrParseTreeNew(const unsigned maxNbSons);
void lrParseTreeFree(const LRParser * const parser, LRParseTree * tree);


#endif /* LR_PARSER_STRUCTURE_H_ */
