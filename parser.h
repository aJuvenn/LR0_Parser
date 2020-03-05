/*
 * parser.h
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "grammar.h"
#include "cdfa/cdfa.h"


typedef struct {

	unsigned nbTokens;
	cdfa__automaton ** tokenAutomatons;

	LRGrammar * grammar;
	LRTransitionMatrix * transition;

	char * cursor;

} LRParser;


typedef struct LRParseTree
{
	int isLeaf;

	union {

		struct {
			unsigned nonTerminalId;
			unsigned ruleId;
			unsigned nbSons;
			struct LRParseTree * sons;
		} node;

		struct {
			unsigned tokenId;
			char * tokenData;
		} leaf;
	};

} LRParseTree;



LRParser * lrParserNew(const char * const configFilePath);

LRParseTree * lrParserParseStr(LRParser * parser, const char * const str);

LRParseTree * lrParserParseFile(LRParser * parser, const char * const filePath);



#endif /* PARSER_H_ */
