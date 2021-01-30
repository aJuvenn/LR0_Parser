/*
 * lr_grammar.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include "../lr_include.h"


LRGrammar * lrGrammarNew(unsigned nbNonTerminals, unsigned nbTerminals, unsigned maxNbRules)
{
	LRGrammar * output = malloc(sizeof(LRGrammar));

	output->nbNonTerminal = nbNonTerminals;
	output->nbTerminals = nbTerminals;
	output->nbSymbols = nbTerminals + nbNonTerminals;

	output->symbolNames = malloc(output->nbSymbols * sizeof(char *));

	output->nbRules = 0;
	output->leftRules = malloc(maxNbRules * sizeof(unsigned));
	output->rightRuleSizes = malloc(maxNbRules * sizeof(unsigned));
	output->rightRules = malloc(maxNbRules * sizeof(unsigned *));

	return output;
}


void lrGrammarFree(LRGrammar * g)
{
	free(g->leftRules);

	for (unsigned i = 0 ; i < g->nbRules ; i++){
		free(g->rightRules[i]);
	}

	free(g->rightRules);
	free(g->rightRuleSizes);

	for (unsigned i = 0 ; i < g->nbSymbols ; i++){
		free(g->symbolNames[i]);
	}

	free(g->symbolNames);
	free(g);
}


void lrGrammarPrint(const LRGrammar * const grammar)
{
	for (unsigned i = 0 ; i < grammar->nbRules ; i++){
		char * leftSymbol = grammar->symbolNames[grammar->leftRules[i]];
		unsigned rightRuleSize = grammar->rightRuleSizes[i];
		unsigned * rightRule = grammar->rightRules[i];
		printf("[%u] %s ->", i, leftSymbol);
		for (unsigned j = 0 ; j < rightRuleSize ; j++){
			printf(" %s", grammar->symbolNames[rightRule[j]]);
		}
		printf("\n");
	}
}


void lrGrammarAddRule(LRGrammar * const g,
					  unsigned leftRule,
					  unsigned rightRuleSize, unsigned * rightRule)
{
	g->leftRules[g->nbRules] = leftRule;
	g->rightRuleSizes[g->nbRules] = rightRuleSize;
	g->rightRules[g->nbRules] = rightRule;
	g->nbRules++;
}



unsigned strIndex(const char * const str, unsigned nbStrs, const char * const * const strs)
{
	for (unsigned j = 0 ; j < nbStrs ; j++){
		if (!strcmp(str, strs[j])){
			return j;
		}
	}

	return (unsigned) -1;
}



LRGrammar * lrGrammarFromConfig(LRConfig * config)
{
	unsigned nbNonTerminals = 0;
	const char * nonTerminals[config->nbGrammarRules];

	for (unsigned i = 0 ; i < config->nbGrammarRules ; i++){

		const char * leftTerm = config->grammarRules[i].leftMember;

		if (strIndex(leftTerm, nbNonTerminals, nonTerminals) == -1){
			nonTerminals[nbNonTerminals++] = leftTerm;
		}
	}

	unsigned nbTerminals = config->nbTokens + 1;
	const char * terminals[nbTerminals];
	for (unsigned i = 0 ; i < config->nbTokens ; i++){
		terminals[i] = config->tokens[i].name;
	}
	terminals[nbTerminals - 1] = "<eof>";

	for (unsigned i = 0 ; i < config->nbGrammarRules ; i++){
		for (unsigned j = 0 ; j < config->grammarRules[i].nbRightMembers ; j++){
			char * rightTerm = config->grammarRules[i].rightMembers[j];
			if (strIndex(rightTerm, nbNonTerminals, nonTerminals) == -1
					&& strIndex(rightTerm, nbTerminals, terminals) == -1){
				fprintf(stderr, "Unknown term %s in grammar rule\n", rightTerm);
				return NULL;
			}
		}
	}

	LRGrammar * output = lrGrammarNew(nbNonTerminals, nbTerminals, config->nbGrammarRules);


	for (unsigned i = 0 ; i < config->nbGrammarRules ; i++){

		unsigned ruleSize =  config->grammarRules[i].nbRightMembers;
		unsigned leftRuleIndex = nbTerminals + strIndex(config->grammarRules[i].leftMember, nbNonTerminals, nonTerminals);
		unsigned * rightRuleIndexes = malloc(ruleSize * sizeof(unsigned));

		for (unsigned j = 0 ; j <  ruleSize ; j++){

			char * rightTerm = config->grammarRules[i].rightMembers[j];
			unsigned index = strIndex(rightTerm, nbNonTerminals, nonTerminals);

			if (index == -1){
				index = strIndex(rightTerm, nbTerminals, terminals);
				if (index == -1){
					fprintf(stderr, "Unknown term %s in grammar rule\n", rightTerm);
					return NULL;
				}
			} else {
				index += nbTerminals;
			}
			rightRuleIndexes[j] = index;
		}

		lrGrammarAddRule(output, leftRuleIndex, ruleSize, rightRuleIndexes);
	}

	for (unsigned i = 0 ; i < nbTerminals ; i++){
		output->symbolNames[i] = strdup(terminals[i]);
	}

	for (unsigned i = 0 ; i < nbNonTerminals ; i++){
		output->symbolNames[i + nbTerminals] = strdup(nonTerminals[i]);
	}

	return output;
}




