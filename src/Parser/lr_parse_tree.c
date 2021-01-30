/*
 * lr_parse_tree.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include "../lr_include.h"


LRParseTree * lrParseTreeNew(const unsigned maxNbSons)
{
	LRParseTree * output = malloc(sizeof(LRParseTree));

	if (output == NULL || maxNbSons == 0){
		return output;
	}
	output->node.ruleId = (unsigned) -1;
	output->node.sons = malloc(maxNbSons * sizeof(LRParseTree *));
	return output;
}


void lrParseTreeFree(const LRParser * const parser, LRParseTree * tree)
{
	if (tree->isLeaf){
		free(tree->leaf.tokenData);

	} else {
		for (unsigned i = 0 ; i < tree->node.nbSons ; i++){
			lrParseTreeFree(parser, tree->node.sons[i]);
		}
		free(tree->node.sons);
	}

	free(tree);
}


void lrParseTreePrintRec(const LRParser * const parser, const LRParseTree * const tree, const unsigned indent)
{
	const LRGrammar * const g = parser->grammar;

	for (unsigned i = 0 ; i < indent ; i++){
		printf("\t");
	}

	if (tree->isLeaf){
		printf("%s {symbolId: %u}\n", tree->leaf.tokenData, tree->symbolId);
		return;
	}

	unsigned ruleId = tree->node.ruleId;

	printf("%s {symbolId: %u} -> ", g->symbolNames[g->leftRules[ruleId]], tree->symbolId);

	for (unsigned j = 0 ; j < g->rightRuleSizes[ruleId] ; j++){
		printf("%s ", g->symbolNames[g->rightRules[ruleId][j]]);
	}

	printf("{ruleId: %u}\n", ruleId);

	for (unsigned i = 0 ; i < tree->node.nbSons ; i++){
		for (unsigned j = 0 ; j < indent+1 ; j++){
			printf("\t");
		}

		printf("[%u]\n", i);

		lrParseTreePrintRec(parser, tree->node.sons[i], indent + 1);
	}
}


void lrParseTreePrint(const LRParser * const parser, const LRParseTree * const tree)
{
	lrParseTreePrintRec(parser, tree, 0);
}
