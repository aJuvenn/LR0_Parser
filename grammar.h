/*
 * grammar.h
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#ifndef GRAMMAR_H_
#define GRAMMAR_H_

#include "parse_config.h"

typedef struct
{
	unsigned nbNonTerminal;
	unsigned nbTerminals;

	unsigned nbSymbols;
	char ** symbolNames;

	unsigned nbRules;
	unsigned * leftRules;
	unsigned * rightRuleSizes;
	unsigned ** rightRules;

} LRGrammar;

LRGrammar * lrGrammarFromDescr(GrammarDescription * descr);

typedef struct
{
	unsigned nbDotedRules;
	unsigned maxNbDotedRules;
	unsigned * ruleIndexes;
	unsigned * dotIndexes;

} LRNode;


typedef struct {

	unsigned nbSymbols;
	unsigned nbStates;
	unsigned maxNbStates;
	LRNode ** stateNodes;

	unsigned * transitions;

} LRTransitionMatrix;


LRTransitionMatrix * lrTransitionMatrixFromGrammar(const LRGrammar * const grammar);

unsigned lrTransitionMatrixGetNextStateId(const LRTransitionMatrix * const transitionMatrix, const unsigned stateId, const unsigned symbolId);


typedef struct LRStateTree
{
	unsigned rule;

	unsigned symbolId;
	char * symbolData;

	unsigned nbSons;
	struct LRStateTree ** sons;

} LRStateTree;

LRStateTree * lrStateTreeNew(unsigned maxNbSons);

void lrStateTreePrint(LRStateTree * tree , LRGrammar * g);

void print_rule(LRGrammar * g, unsigned i);


#endif /* GRAMMAR_H_ */
