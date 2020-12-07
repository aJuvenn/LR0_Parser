/*
 * parser.h
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_PARSER_H_
#define LR_PARSER_H_

#include "../lr_include.h"



typedef struct {

	unsigned nbTokens;
	unsigned skippedAutomatonId;
	cdfa__automaton ** tokenAutomatons;

	LRGrammar * grammar;
	LRTransitionMatrix * transition;

} LRParser;


#define LR_PARSER_MAX_DEPTH 1024


LRParser * lrParserNew(const char * const configFilePath);

void lrParserFree(LRParser * p);

LRParseTree * lrParserParseStr(LRParser * parser, const char * const str);

LRParseTree * lrParserParseFile(LRParser * parser, const char * const filePath);





#endif /* LR_PARSER_H_ */
