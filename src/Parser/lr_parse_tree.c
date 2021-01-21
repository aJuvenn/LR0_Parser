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

		lrParseTreePrintRec(parser, tree->node.sons[i], indent + 1);
	}
}


void lrParseTreePrint(const LRParser * const parser, const LRParseTree * const tree)
{
	lrParseTreePrintRec(parser, tree, 0);
}

int lrParseTreeSymbolIsTerminal(const LRParser * const parser, const LRParseTree * const tree)
{
	return tree->isLeaf;
}

LRSymbolId lrParseTreeGetSymbolId(const LRParser * const parser, const LRParseTree * const tree)
{
	return (LRSymbolId) tree->symbolId;
}

const char * lrParseTreeGetSymbolName(const LRParser * const parser, const LRParseTree * const tree)
{
	return lrParserGetSymbolName(parser, tree->symbolId);
}

const char * lrParseTreeGetTerminalSymbolString(const LRParser * const parser, const LRParseTree * const tree)
{
	if (!tree->isLeaf){
		return NULL;
	}
	return tree->leaf.tokenData;
}

LRRuleId lrParseTreeGetRuleId(const LRParser * const parser, const LRParseTree * const tree)
{
	if (tree->isLeaf){
		return (LRRuleId) -1;
	}
	return tree->node.ruleId;
}

const char * lrParseTreeGetRuleName(const LRParser * const parser, const LRParseTree * const tree)
{
	if (tree->isLeaf){
		return NULL;
	}
	return lrParserGetRuleName(parser, tree->node.ruleId);
}

unsigned lrParseTreeGetNbSons(const LRParser * const parser, const LRParseTree * const tree)
{
	if (tree->isLeaf){
		return 0;
	}
	return tree->node.nbSons;
}

const LRParseTree * const * lrParseTreeGetSons(const LRParser * const parser, const LRParseTree * const tree)
{
	if (tree->isLeaf){
		return NULL;
	}
	return (const LRParseTree * const *) tree->node.sons;
}
