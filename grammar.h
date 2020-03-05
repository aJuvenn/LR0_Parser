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


typedef struct LRStateTree
{
	unsigned rule;

	unsigned symbol;
	unsigned symbolStreamIndex;

	unsigned nbSons;
	struct LRStateTree ** sons;

} LRStateTree;




#endif /* GRAMMAR_H_ */
