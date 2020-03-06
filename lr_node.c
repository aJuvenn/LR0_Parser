/*
 * lr_node.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include "lr_include.h"



LRNode * lrNodeNew()
{
	LRNode * node = malloc(sizeof(LRNode));
	node->nbDotedRules = 0;
	node->maxNbDotedRules = 0;
	node->ruleIndexes = NULL;
	node->dotIndexes = NULL;
	return node;
}

void lrNodeFree(LRNode * node)
{
	free(node->dotIndexes);
	free(node->ruleIndexes);
	free(node);
}



int lrNodeDotedRuleIsIn(const LRNode * const node, const unsigned ruleId, const unsigned dotId)
{
	for (unsigned i = 0 ; i < node->nbDotedRules ; i++){
		if (node->ruleIndexes[i] == ruleId && node->dotIndexes[i] == dotId){
			return 1;
		}
	}

	return 0;
}


void lrNodeAddDotedRule(LRNode * const node, const unsigned ruleId, const unsigned dotId)
{
	if (lrNodeDotedRuleIsIn(node, ruleId, dotId)){
		return;
	}

	if (node->nbDotedRules == node->maxNbDotedRules){
		node->maxNbDotedRules = 2 * node->maxNbDotedRules + 1;
		node->ruleIndexes = realloc(node->ruleIndexes, node->maxNbDotedRules * sizeof(unsigned));
		node->dotIndexes = realloc(node->dotIndexes, node->maxNbDotedRules * sizeof(unsigned));
	}

	node->ruleIndexes[node->nbDotedRules] = ruleId;
	node->dotIndexes[node->nbDotedRules] = dotId;
	node->nbDotedRules++;
}



int lrNodeAreEquals(const LRNode * const node1, const LRNode * const node2)
{
	const unsigned nbDotedRules = node1->nbDotedRules;

	if (nbDotedRules != node2->nbDotedRules){
		return 0;
	}

	for (unsigned i = 0 ; i < nbDotedRules ; i++){
		if (!lrNodeDotedRuleIsIn(node2, node1->ruleIndexes[i], node1->dotIndexes[i])){
			return 0;
		}
	}

	return 1;
}


static inline int isNonTerminal(const LRGrammar * const g, const unsigned symbol)
{
	return (symbol < g->nbNonTerminal);
}


void lrNodeClose(const LRGrammar * const grammar, LRNode * const node)
{
	for (unsigned i = 0 ; i < node->nbDotedRules ; i++){

		const unsigned ruleId = node->ruleIndexes[i];
		const unsigned dotId = node->dotIndexes[i];

		if (dotId == grammar->rightRuleSizes[ruleId]){
			/* Dot is not before a symbol */
			continue;
		}

		unsigned nextSymbol = grammar->rightRules[ruleId][dotId];

		if (!isNonTerminal(grammar, nextSymbol)){
			/* Dot is not before a non terminal symbol */
			continue;
		}

		for (unsigned j = 0 ; j < grammar->nbRules ; j++){
			if (grammar->leftRules[j] == nextSymbol){
				/* Adding symbol substitution rule if or already in the node */
				lrNodeAddDotedRule(node, j, 0);
			}
		}
	}
}



LRNode * lrNodeGetTransition(const LRGrammar * const grammar, const LRNode * const fromNode, const unsigned symbolId)
{
	LRNode * toNode = lrNodeNew();

	for (unsigned i = 0 ; i < fromNode->nbDotedRules ; i++){

		const unsigned ruleId = fromNode->ruleIndexes[i];
		const unsigned dotId = fromNode->dotIndexes[i];

		if (dotId == grammar->rightRuleSizes[ruleId] || symbolId != grammar->rightRules[ruleId][dotId]){
			continue;
		}

		/* fromNode contains a rule with a dot just before the symbol */
		lrNodeAddDotedRule(toNode, ruleId, dotId + 1);
	}

	lrNodeClose(grammar, toNode);

	return toNode;
}



void lrNodePrint(const LRGrammar * const grammar, const LRNode * const node)
{
	printf("Node\n{\n");

	for (unsigned i = 0 ; i < node->nbDotedRules ; i++){

		const unsigned ruleId = node->ruleIndexes[i];
		const unsigned dotId = node->dotIndexes[i];

		printf("\t%s -> ", grammar->symbolNames[grammar->leftRules[ruleId]]);

		for (unsigned j = 0 ; j < dotId ; j++){
			printf("%s ", grammar->symbolNames[grammar->rightRules[ruleId][j]]);
		}

		printf(".");

		for (unsigned j = dotId ; j < grammar->rightRuleSizes[ruleId] ; j++){
			printf("%s ", grammar->symbolNames[grammar->rightRules[ruleId][j]]);
		}

		printf("\n");
	}
	printf("}\n");
}
