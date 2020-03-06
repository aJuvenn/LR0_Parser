/*
 * lr_parse_tree.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include "lr_include.h"


LRParseTree * lrParseTreeNew(const unsigned maxNbSons)
{
	LRParseTree * output = malloc(sizeof(LRParseTree));

	if (output == NULL || maxNbSons == 0){
		return output;
	}

	output->node.sons = malloc(maxNbSons * sizeof(LRParseTree *));

	return output;
}


void lrParseTreeFree(LRParseTree * t)
{
	if (t->isLeaf){
		free(t->leaf.tokenData);

	} else {

		for (unsigned i = 0 ; i < t->node.nbSons ; i++){
			lrParseTreeFree(t->node.sons[i]);
		}

		free(t->node.sons);
	}

	free(t);
}


void lrParseTreePrintRec(const LRParseTree * const tree, const unsigned indent, const LRGrammar * const g)
{
	for (unsigned i = 0 ; i < indent ; i++){
		printf("\t");
	}

	if (tree->isLeaf){
		printf("%s\n", tree->leaf.tokenData);
		return;
	}

	unsigned ruleId = tree->node.ruleId;

	printf("%s -> ", g->symbolNames[g->leftRules[ruleId]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[ruleId] ; j++){
		printf("%s ", g->symbolNames[g->rightRules[ruleId][j]]);
	}

	printf("\n");


	for (unsigned i = 0 ; i < tree->node.nbSons ; i++){
		for (unsigned j = 0 ; j < indent+1 ; j++){
			printf("\t");
		}

		printf("[%u]\n", i);

		lrParseTreePrintRec(tree->node.sons[i], indent + 1, g);
	}
}


void lrParseTreePrint(const LRParseTree * const tree , const LRGrammar * const g)
{
	lrParseTreePrintRec(tree, 0, g);
}

