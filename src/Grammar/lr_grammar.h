/*
 * grammar.h
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_GRAMMAR_H_
#define LR_GRAMMAR_H_

#include "../lr_include.h"

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
	char ** ruleNames;

} LRGrammar;



LRGrammar * lrGrammarFromConfig(LRConfig * config);
void lrGrammarFree(LRGrammar * g);

void lrGrammarPrint(const LRGrammar * const g);


#endif /* LR_GRAMMAR_H_ */
