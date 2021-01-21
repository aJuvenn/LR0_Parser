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


void lrGrammarPrintRule(const LRGrammar * const g, const unsigned ruleId)
{
	printf("[%u]\t%s -> ", ruleId, g->symbolNames[g->leftRules[ruleId]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[ruleId] ; j++){
		printf("%s ", g->symbolNames[g->rightRules[ruleId][j]]);
	}

	printf("\n");
}

void lrGrammarPrint(const LRGrammar * const g)
{
	for (unsigned i = 0 ; i < g->nbRules ; i++){
		lrGrammarPrintRule(g, i);
	}
}




void lrGrammarAddRule(LRGrammar * const g, unsigned leftRule, unsigned rightRuleSize, unsigned * rightRule)
{
	g->leftRules[g->nbRules] = leftRule;
	g->rightRuleSizes[g->nbRules] = rightRuleSize;
	g->rightRules[g->nbRules] = rightRule;
	g->nbRules++;
}



unsigned strIndex(char * str, unsigned nbStrs, char ** strs)
{
	for (unsigned j = 0 ; j < nbStrs ; j++){
		if (!strcmp(str, strs[j])){
			return j;
		}
	}

	return (unsigned) -1;
}


LRGrammar * lrGrammarFromDescr(lrFileConfig * descr)
{
	unsigned nbNonTerminals = 0;
	char ** nonTerminals = malloc(MAX_NB_LINES * sizeof(char *));

	if (nonTerminals == NULL){
		return NULL;
	}

	for (unsigned i = 0 ; i < descr->nbGrammarRules ; i++){

		char * leftTerm = descr->ruleLeftMembers[i];

		if (strIndex(leftTerm, nbNonTerminals, nonTerminals) == -1){
			nonTerminals[nbNonTerminals++] = leftTerm;
		}
	}

	unsigned nbTerminals = descr->nbTokens;
	char ** terminals = descr->tokenNames;

	for (unsigned i = 0 ; i < descr->nbGrammarRules ; i++){
		for (unsigned j = 0 ; j < descr->ruleRightMemberSizes[i] ; j++){
			char * rightTerm = descr->ruleRightMembers[i][j];
			if (strIndex(rightTerm, nbNonTerminals, nonTerminals) == -1 && strIndex(rightTerm, nbTerminals, terminals) == -1){
				fprintf(stderr, "Unknown term %s in grammar rule\n", rightTerm);
				free(nonTerminals);
				return NULL;
			}
		}
	}

	LRGrammar * output = lrGrammarNew(nbNonTerminals + 1, nbTerminals + 1, descr->nbGrammarRules + 1);

	unsigned int * startRightRule = malloc(2 * sizeof(unsigned int));

	if (startRightRule == NULL){
		return NULL;
	}

	startRightRule[0] = 1;
	startRightRule[1] = nbNonTerminals + nbTerminals + 1;
	lrGrammarAddRule(output, 0, 2, startRightRule);


	for (unsigned i = 0 ; i < descr->nbGrammarRules ; i++){

		unsigned ruleSize =  descr->ruleRightMemberSizes[i];
		unsigned leftRuleIndex = strIndex(descr->ruleLeftMembers[i], nbNonTerminals, nonTerminals);
		unsigned * rightRuleIndexes = malloc(ruleSize * sizeof(unsigned));

		for (unsigned j = 0 ; j <  ruleSize ; j++){

			char * rightTerm = descr->ruleRightMembers[i][j];
			unsigned index = strIndex(rightTerm, nbNonTerminals, nonTerminals);

			if (index == -1){

				index = strIndex(rightTerm, nbTerminals, terminals);

				if (index == -1){
					fprintf(stderr, "Unknown term %s in grammar rule\n", rightTerm);
					free(nonTerminals);
					return NULL;
				}

				index += nbNonTerminals + 1;

			} else {
				index += 1;
			}

			rightRuleIndexes[j] = index;
		}

		lrGrammarAddRule(output, leftRuleIndex + 1, ruleSize, rightRuleIndexes);
	}


	output->symbolNames[0] = strdup("<START>");

	for (unsigned i = 0 ; i < nbNonTerminals ; i++){
		output->symbolNames[i+1] = strdup(nonTerminals[i]);
	}

	for (unsigned i = 0 ; i < nbTerminals ; i++){
		output->symbolNames[i + nbNonTerminals+1] = strdup(terminals[i]);
	}

	output->symbolNames[nbNonTerminals + nbTerminals + 1] = strdup("<eof>");


	free(nonTerminals);

	return output;
}




