/*
 * blob.c
 *
 *  Created on: 19 févr. 2020
 *      Author: ajuvenn
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


LRGrammar * lrGrammarFromDescr(GrammarDescription * descr)
{
	unsigned nbNonTerminals = 0;
	char ** nonTerminals = malloc(MAX_NB_LINES * sizeof(char *));

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
				return NULL;
			}
		}
	}

	LRGrammar * output = lrGrammarNew(nbNonTerminals, nbTerminals, descr->nbGrammarRules);


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
					return NULL;
				}

				index += nbNonTerminals;
			}

			rightRuleIndexes[j] = index;
		}

		lrGrammarAddRule(output, leftRuleIndex, ruleSize, rightRuleIndexes);
	}


	for (unsigned i = 0 ; i < nbNonTerminals ; i++){
		output->symbolNames[i] = strdup(nonTerminals[i]);
	}

	for (unsigned i = 0 ; i < nbTerminals ; i++){
		output->symbolNames[i + nbNonTerminals] = strdup(terminals[i]);
	}

	return output;
}





typedef struct
{
	unsigned nbDotedRules;
	unsigned maxNbDotedRules;
	unsigned * ruleIndexes;
	unsigned * dotIndexes;

} LRNode;









void print_node(const LRGrammar * const grammar, const LRNode * const n);


static inline int isNonTerminal(const LRGrammar * const g, const unsigned symbol)
{
	return (symbol < g->nbNonTerminal);
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


typedef struct {

	unsigned nbSymbols;
	unsigned nbStates;
	unsigned maxNbStates;
	LRNode ** stateNodes;

	unsigned * transitions;

} LRTransitionMatrix;



int lrTransitionMatrixNodeIsIn(const LRTransitionMatrix * const transitionMatrix, const LRNode * const node)
{
	for (unsigned i = 0 ; i < transitionMatrix->nbStates ; i++){

		if (lrNodeAreEquals(node, transitionMatrix->stateNodes[i])){
			return 1;
		}
	}

	return 0;
}


void lrTransitionMatrixAddNode(LRTransitionMatrix * const transitionMatrix, LRNode * const node)
{
	if (transitionMatrix->nbStates == transitionMatrix->maxNbStates){
		transitionMatrix->maxNbStates = 2 * transitionMatrix->maxNbStates + 1;
		transitionMatrix->stateNodes = realloc(transitionMatrix->stateNodes, transitionMatrix->maxNbStates * sizeof(LRNode *));
	}

	transitionMatrix->stateNodes[transitionMatrix->nbStates] = node;
	transitionMatrix->nbStates++;
}


void lrTransitionMatrixEmpty(LRTransitionMatrix * const transitionMatrix)
{
	transitionMatrix->nbSymbols = 0;
	transitionMatrix->nbStates = 0;
	transitionMatrix->maxNbStates = 0;
	transitionMatrix->stateNodes = NULL;
	transitionMatrix->transitions = NULL;
}




int lrTransitionMatrixFillStates(const LRGrammar * const grammar, LRTransitionMatrix * const transitionMatrix)
{
	const unsigned nbSymbols = grammar->nbNonTerminal + grammar->nbTerminals;

	lrTransitionMatrixEmpty(transitionMatrix);
	transitionMatrix->nbSymbols = nbSymbols;

	LRNode * initialNode = lrNodeNew();
	lrNodeAddDotedRule(initialNode, 0, 0);
	lrNodeClose(grammar, initialNode);
	lrTransitionMatrixAddNode(transitionMatrix, initialNode);

	for (unsigned nodeId = 0 ; nodeId < transitionMatrix->nbStates ; nodeId++){

		LRNode * fromNode = transitionMatrix->stateNodes[nodeId];

		for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

			LRNode * toNode = lrNodeGetTransition(grammar, fromNode, symbolId);

			if (toNode == NULL){
				return EXIT_FAILURE;
			}

			if (lrTransitionMatrixNodeIsIn(transitionMatrix, toNode)){
				lrNodeFree(toNode);
			} else {
				lrTransitionMatrixAddNode(transitionMatrix, toNode);
			}
		}
	}

	return EXIT_SUCCESS;
}


unsigned lrTransitionMatrixGetNodeId(const LRTransitionMatrix * const transitionMatrix, const LRNode * const node)
{
	for (unsigned i = 0 ; i < transitionMatrix->nbStates; i++){
		if (lrNodeAreEquals(node, transitionMatrix->stateNodes[i])){
			return i;
		}
	}

	return (unsigned) -1;
}


int lrTransitionMatrixFillTransitions(const LRGrammar * const grammar, LRTransitionMatrix * const transitionMatrix)
{
	const unsigned nbSymbols = grammar->nbSymbols;
	const unsigned nbStates = transitionMatrix->nbStates;

	transitionMatrix->transitions = malloc(nbSymbols * nbStates * sizeof(unsigned));

	for (unsigned fromNodeId = 0 ; fromNodeId < nbStates ; fromNodeId++){

		LRNode * fromNode = transitionMatrix->stateNodes[fromNodeId];

		for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

			LRNode * toNode = lrNodeGetTransition(grammar, fromNode, symbolId);

			if (toNode == NULL){
				return EXIT_FAILURE;
			}

			unsigned toNodeId = lrTransitionMatrixGetNodeId(transitionMatrix, toNode);
			lrNodeFree(toNode);

			if (toNodeId == (unsigned) -1){
				return EXIT_FAILURE;
			}

			transitionMatrix->transitions[symbolId + nbSymbols * fromNodeId] = toNodeId;
		}
	}

	return EXIT_SUCCESS;
}

void lrTransitionMatrixFillReduce(const LRGrammar * const grammar, LRTransitionMatrix * const transitionMatrix)
{
	for (unsigned i = 0 ; i < transitionMatrix->nbStates ; i++){

		const LRNode * const node = transitionMatrix->stateNodes[i];
		int fillState = 0;
		unsigned ruleId;
		unsigned savedRuleId;
		for (unsigned j = 0 ; j < node->nbDotedRules ; j++){

			ruleId = node->ruleIndexes[j];
			const unsigned dotId = node->dotIndexes[j];

			if (grammar->rightRuleSizes[ruleId] == dotId){

				if (fillState){
					printf("Warning ! Several reducing rules\n");
				}
				fillState = 1;
				savedRuleId = ruleId;
			}
		}

		if (fillState){
			for (unsigned j = grammar->nbNonTerminal ; j < transitionMatrix->nbSymbols ; j++){
				transitionMatrix->transitions[j + transitionMatrix->nbSymbols * i] = -(savedRuleId + 1);
			}
		}
	}
}


unsigned lrTransitionMatrixGetNextStateId(const LRTransitionMatrix * const transitionMatrix, const unsigned stateId, const unsigned symbolId)
{
	return transitionMatrix->transitions[symbolId + transitionMatrix->nbSymbols * stateId];
}


int lrTransitionMatrixFromGrammar(const LRGrammar * const grammar, LRTransitionMatrix * const transitionMatrix)
{
	int ret;

	lrTransitionMatrixEmpty(transitionMatrix);
	ret = lrTransitionMatrixFillStates(grammar, transitionMatrix);

	if (ret != EXIT_SUCCESS){
		printf("fill fail\n");
		return ret;
	}

	ret = lrTransitionMatrixFillTransitions(grammar, transitionMatrix);

	if (ret != EXIT_SUCCESS){
		printf("transition fail\n");
		return ret;
	}

	lrTransitionMatrixFillReduce(grammar, transitionMatrix);

	return EXIT_SUCCESS;
}



void print_rule(LRGrammar * g, unsigned i)
{
	printf("[%u]\t%s -> ", i, g->symbolNames[g->leftRules[i]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[i] ; j++){
		printf("%s ", g->symbolNames[g->rightRules[i][j]]);
	}

	printf("\n");
}

void print_grammar(LRGrammar * g)
{
	for (unsigned i = 0 ; i < g->nbRules ; i++){
		print_rule(g, i);
	}
}


unsigned nbNonTerminal(LRGrammar * g, unsigned ruleId)
{
	unsigned output = 0;

	for (unsigned i = 0 ; i < g->rightRuleSizes[ruleId] ; i++){
		if (isNonTerminal(g, g->rightRules[ruleId][i])){
			output++;
		}
	}

	return output;
}


void print_mat(LRTransitionMatrix * mat, LRGrammar * g)
{
	unsigned nbSymols = g->nbSymbols;

	printf("\t\tTransition matrix\n\n\t\t");

	for (unsigned i = 0 ; i < nbSymols ; i++){
		printf("%s\t", g->symbolNames[i]);
	}

	printf("\n\n");

	for (unsigned i = 0 ; i < mat->nbStates ; i++){
		printf("%u\t\t", i);
		for (unsigned j = 0 ; j < nbSymols ; j++){

			int action = (int) lrTransitionMatrixGetNextStateId(mat, i, j);

			if (action >= 0){
				printf("s%d\t", action);
			} else {
				printf("r%d\t", -(action+1));
			}
		}
		printf("\n");
	}
}

void print_node(const LRGrammar * const grammar, const LRNode * const node)
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

		for (unsigned j = dotId ; j < grammar->rightRuleSizes[ruleId]  ; j++){
			printf("%s ", grammar->symbolNames[grammar->rightRules[ruleId][j]]);
		}

		printf("\n");
	}
	printf("}\n");
}




typedef struct LRStateTree
{
	unsigned rule;

	unsigned symbol;
	unsigned symbolStreamIndex;

	unsigned nbSons;
	struct LRStateTree ** sons;

} LRStateTree;


LRStateTree * lrStateTreeNew(unsigned maxNbSons)
{
	LRStateTree * output = malloc(sizeof(LRStateTree));

	output->nbSons = 0;

	if (maxNbSons == 0){
		output->sons = NULL;
	} else {
		output->sons = malloc(maxNbSons * sizeof(LRStateTree *));
	}

	return output;
}

void lrStateTreePrintRec(LRStateTree * tree, unsigned indent, LRGrammar * g)
{
	for (unsigned i = 0 ; i < indent ; i++){
		printf("\t");
	}


	unsigned i = tree->rule;

	if (i == (unsigned) -1){
		printf("%s at index %u\n", g->symbolNames[tree->symbol], tree->symbolStreamIndex);
		return;
	}

	printf("%s -> ", g->symbolNames[g->leftRules[i]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[i] ; j++){
		printf("%s ", g->symbolNames[g->rightRules[i][j]]);
	}

	printf("\n");


	for (unsigned i = 0 ; i < tree->nbSons ; i++){
		for (unsigned j = 0 ; j < indent+1 ; j++){
			printf("\t");
		}

		printf("[%u]\n", i);

		lrStateTreePrintRec(tree->sons[i], indent + 1, g);
	}
}

void lrStateTreePrint(LRStateTree * tree , LRGrammar * g)
{
	lrStateTreePrintRec(tree, 0, g);
}


unsigned lrGrammarStrToSymbolId(LRGrammar * g, char * str)
{
	for (unsigned i = 0 ; i < g->nbSymbols ; i++){
		if (!strcmp(str, g->symbolNames[i])){
			return i;
		}
	}

	return -1;
}

LRStateTree * f(LRGrammar * grammar, LRTransitionMatrix * transitions, char * str)
{
	unsigned nbSymbols;
	char ** strSymbols;

	split(str, " ", &nbSymbols, &strSymbols);

	unsigned symbols[nbSymbols];

	for (unsigned i = 0 ; i < nbSymbols ; i++){
		symbols[i] = lrGrammarStrToSymbolId(grammar, strSymbols[i]);

		if (symbols[i] == -1){
			fprintf(stderr, "Unknown symbol %s\n", strSymbols[i]);
			return NULL;
		}
	}

	LRStateTree * tmp;

	unsigned stackSize = 0;
	unsigned stack[1000];

	unsigned treeStackSize = 0;
	LRStateTree * treeStack[1000];

	stack[stackSize++] = 0;

	unsigned i = 0;

	while (i < nbSymbols){

		unsigned currentSymbol = symbols[i];
		unsigned currentSate = stack[stackSize - 1];
		printf("----------------------------\n");
		printf("Current state: %u, current symbol: %s\n", currentSate, grammar->symbolNames[currentSymbol]);

		int action = (int) lrTransitionMatrixGetNextStateId(transitions, currentSate, currentSymbol);

		if (action >= 0){
			// shift
			stack[stackSize++] = (unsigned) action;
			printf("Shift %d\n", action);

			tmp = lrStateTreeNew(0);
			tmp->rule = (unsigned) -1;
			tmp->symbol = currentSymbol;
			tmp->symbolStreamIndex = i;

			treeStack[treeStackSize++] = tmp;

			i++;
			continue;
		}

		// reduce
		action = -(action + 1);

		//printf("[%c][%u] Reduce %d : [ ", symbolIdToChar[currentSymbol], currentSate, action);
		printf("Reducing rule ");
		print_rule(grammar, action);

		unsigned ruleSize = grammar->rightRuleSizes[action];
		//unsigned nbNonTerm = nbNonTerminal(grammar, action);

		tmp = lrStateTreeNew(ruleSize);
		tmp->nbSons = ruleSize;
		for (unsigned j = 0 ; j < ruleSize ; j++){
			tmp->sons[j] = treeStack[treeStackSize-ruleSize + j];
		}
		tmp->symbol = grammar->leftRules[action];
		tmp->rule = action;

		stackSize -= ruleSize;
		treeStackSize -= ruleSize;
		currentSate = stack[stackSize - 1];

		unsigned new_state = lrTransitionMatrixGetNextStateId(transitions, currentSate, grammar->leftRules[action]);
		stack[stackSize++] = new_state;
		printf("Pusing tree...\n");
		lrStateTreePrint(tmp, grammar);

		treeStack[treeStackSize++] = tmp;

		/*
		for (unsigned j = 0 ; j < stackSize ; j++){
			printf("%u ", stack[j]);
		}

		printf("]\n");
		 */
	}
	printf("----------------------------\n");
	printf("----------------------------\n");
	printf("Tree Stack Size : %u\n", treeStackSize);

	for (unsigned i = 0 ; i < treeStackSize ; i++){
		printf("[tree %u]\n", i);
		lrStateTreePrint(treeStack[i], grammar);
		printf("\n");
	}

	printf("----------------------------\n");
	printf("----------------------------\n");
	/*
	LRStateTree * output = lrStateTreeNew(treeStackSize);
	addStrRule(g, 'E', "0");

	output->nbSons = treeStackSize;

	for (unsigned i = 0 ; i < treeStackSize ; i++){
		output->sons[i] = treeStack[i];
	}

	output->symbol = 0;
	 */

	return treeStack[0];
}











int main(int argc, char **argv)
{
	char * path = "grammar.txt";

	printf("Parsing file %s...\n", path);

	GrammarDescription * desc = parseConfigFile(path);

	printf("Done.\n\nPrinting parsed file...\n");

	printConfigFile(desc);

	printf("Done.\n\nCreating grammar...\n");
	LRGrammar * gr = lrGrammarFromDescr(desc);

	printf("Done.\n\nPrinting grammar...\n");

	print_grammar(gr);

	printf("Done.\n\nCreating table...\n");

	LRTransitionMatrix mat;
	int ret = lrTransitionMatrixFromGrammar(gr, &mat);

	if (ret != EXIT_SUCCESS){
		printf("FAIL.\n");
		exit(EXIT_FAILURE);
	}

	printf("Done.\n\nPrinting table...\n");

	print_mat(&mat, gr);

	printf("Done.\n\n Printing states...\n");

	for (unsigned i = 0 ; i < mat.nbStates ; i++){
		printf("[%u] ", i);
		print_node(gr, mat.stateNodes[i]);
		printf("\n--------------------------------\n");
	}

	char * str = strdup("let var var ; var var eof");

	printf("Done.\n\nCreating tree from string %s..\n", str);

	LRStateTree * tree = f(gr, &mat, str);

	printf("Done. Printing tree...\n");

	lrStateTreePrint(tree, gr);

	printf("Done\n");

	return 0;
}




