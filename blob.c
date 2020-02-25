/*
 * blob.c
 *
 *  Created on: 19 févr. 2020
 *      Author: ajuvenn
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
	unsigned nbNonTerminal;
	unsigned nbTerminals;

	unsigned nbRules;
	unsigned * leftRules;
	unsigned * rightRuleSizes;
	unsigned ** rightRules;

} LRGrammar;


extern const char * symbolIdToChar;


LRGrammar * lrGrammarNew(unsigned nbNonTerminal, unsigned nbTerminals, unsigned maxNbRules)
{
	LRGrammar * output = malloc(sizeof(LRGrammar));

	output->nbNonTerminal = nbNonTerminal;
	output->nbTerminals = nbTerminals;

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

		printf("fromNode->nbDotedRules: %u\n", fromNode->nbDotedRules);
		printf("\truleId: %u\n", ruleId);
		printf("\tdotId: %u\n", dotId);
		printf("\tgrammar->rightRuleSizes[ruleId]: %u\n", grammar->rightRuleSizes[ruleId]);
		printf("\tsymbol[grammar->rightRules[ruleId][dotId]]: %c\n", symbolIdToChar[grammar->rightRules[ruleId][dotId]]);
		printf("\tsymbol[symbolId]: %c\n", symbolIdToChar[symbolId]);



		if (dotId == grammar->rightRuleSizes[ruleId] || symbolId != grammar->rightRules[ruleId][dotId]){
			printf("continue\n");
			continue;
		}

		printf("Add\n");
		/* fromNode contains a rule with a dot just before the symbol */
		lrNodeAddDotedRule(toNode, ruleId, dotId + 1);
	}

	printf("Before closing: ");
	print_node(grammar, toNode);
	lrNodeClose(grammar, toNode);
	printf("After closing: ");
	print_node(grammar, toNode);

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

		printf("nodeId : %u\n", nodeId);
		printf("transitionMatrix->nbStates: %u\n", transitionMatrix->nbStates);
		LRNode * fromNode = transitionMatrix->stateNodes[nodeId];

		printf("from node : ");
		print_node(grammar, fromNode);

		for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

			printf("Symbol : %c\n", symbolIdToChar[symbolId]);

			LRNode * toNode = lrNodeGetTransition(grammar, fromNode, symbolId);

			if (toNode == NULL){
				printf ("FFFFAIL\n");
				return EXIT_FAILURE;
			}

			printf("to node : ");
			print_node(grammar, toNode);

			if (lrTransitionMatrixNodeIsIn(transitionMatrix, toNode)){
				printf("\tNode already in\n");
				lrNodeFree(toNode);
			} else {
				printf("\tNode added\n");
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
	const unsigned nbSymbols = grammar->nbNonTerminal + grammar->nbTerminals;
	const unsigned nbStates = transitionMatrix->nbStates;

	transitionMatrix->transitions = malloc(nbSymbols * nbStates * sizeof(unsigned));

	for (unsigned fromNodeId = 0 ; fromNodeId < nbStates ; fromNodeId++){

		LRNode * fromNode = transitionMatrix->stateNodes[fromNodeId];

		for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

			LRNode * toNode = lrNodeGetTransition(grammar, fromNode, symbolId);

			if (toNode == NULL){
				printf("FaaaaAAaAaaAAAAaail\n");
				return EXIT_FAILURE;
			}

			unsigned toNodeId = lrTransitionMatrixGetNodeId(transitionMatrix, toNode);
			lrNodeFree(toNode);

			if (toNodeId == (unsigned) -1){
				printf("Faaaaaaaaail\n");
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



unsigned charToRuleId(char c)
{
	for (unsigned i = 0 ; symbolIdToChar[i] != 0 ; i++){
		if (c == symbolIdToChar[i]){
			return i;
		}
	}

	return (unsigned) -1;
}


unsigned * strToRuleIds(char * str, unsigned * len)
{
	*len = strlen(str);
	unsigned * output = malloc(*len * sizeof(unsigned));

	for (unsigned i = 0 ; i < *len ; i++){
		output[i] = charToRuleId(str[i]);
	}

	return output;
}

void addStrRule(LRGrammar * g, char left, char * right)
{
	unsigned rightRuleSize;
	unsigned * rightRule = strToRuleIds(right, &rightRuleSize);
	lrGrammarAddRule(g,  charToRuleId(left), rightRuleSize, rightRule);
}


void print_rule(LRGrammar * g, unsigned i)
{
	printf("[%u]\t%c->", i, symbolIdToChar[g->leftRules[i]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[i] ; j++){
		printf("%c", symbolIdToChar[g->rightRules[i][j]]);
	}

	printf("\n");
}

void print_grammar(LRGrammar * g)
{
	for (unsigned i = 0 ; i < g->nbRules ; i++){
		printf("[%u]\t%c->", i, symbolIdToChar[g->leftRules[i]]);

		for (unsigned j = 0 ; j < g->rightRuleSizes[i] ; j++){
			printf("%c", symbolIdToChar[g->rightRules[i][j]]);
		}

		printf("\n");
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


void print_mat(LRTransitionMatrix * mat)
{
	unsigned nbSymols = strlen(symbolIdToChar);

	printf("\t\tTransition matrix\n\n\t\t");

	for (unsigned i = 0 ; i < nbSymols ; i++){
		printf("%c\t", symbolIdToChar[i]);
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
	printf("Node{\n");
	for (unsigned i = 0 ; i < node->nbDotedRules ; i++){

		const unsigned ruleId = node->ruleIndexes[i];
		const unsigned dotId = node->dotIndexes[i];

		printf("\t%c->", symbolIdToChar[grammar->leftRules[ruleId]]);

		for (unsigned j = 0 ; j < dotId ; j++){
			printf("%c", symbolIdToChar[grammar->rightRules[ruleId][j]]);
		}

		printf(".");

		for (unsigned j = dotId ; j < grammar->rightRuleSizes[ruleId]  ; j++){
			printf("%c", symbolIdToChar[grammar->rightRules[ruleId][j]]);
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
		printf("Symbol %c at index %u\n", symbolIdToChar[tree->symbol], tree->symbolStreamIndex);
		return;
	}

	printf("%c->", symbolIdToChar[g->leftRules[i]]);

	for (unsigned j = 0 ; j < g->rightRuleSizes[i] ; j++){
		printf("%c", symbolIdToChar[g->rightRules[i][j]]);
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




LRStateTree * f(LRGrammar * grammar, LRTransitionMatrix * transitions, unsigned nbSymbols, unsigned * symbols)
{

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
		printf("Current state: %u, current symbol: %c\n", currentSate, symbolIdToChar[currentSymbol]);

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


const char * symbolIdToChar ="SEAL01fvie()$";



int main(int argc, char **argv)
{
	const unsigned nbNonTerminal = 4;
	const unsigned nbTerminals = 9;

	LRGrammar * g = lrGrammarNew(nbNonTerminal, nbTerminals, 100);


	addStrRule(g, 'S', "E$");
	addStrRule(g, 'E', "f(A)E");
	addStrRule(g, 'A', "v");
	addStrRule(g, 'A', "Av");
	addStrRule(g, 'E', "(L)");
	addStrRule(g, 'L', "E");
	addStrRule(g, 'L', "LE");
	addStrRule(g, 'E', "v");
	addStrRule(g, 'E', "1");
	addStrRule(g, 'E', "0");
	addStrRule(g, 'E', "iEEeE");

	printf("Creating table...\n");

	LRTransitionMatrix mat;
	int ret = lrTransitionMatrixFromGrammar(g, &mat);

	if (ret != EXIT_SUCCESS){
		printf("FAIL.\n");
		exit(EXIT_FAILURE);
	}

	printf("Done.\n");
	print_grammar(g);
	print_mat(&mat);
	printf("\n\n");

	printf("States\n");

	for (unsigned i = 0 ; i < mat.nbStates ; i++){
		printf("[%u] ", i);
		print_node(g, mat.stateNodes[i]);
		printf("\n--------------------------------\n");
	}
	printf("\n\n");

	unsigned int len;
	char * strrr = "f(v)i(v01)(vv)e(v1)$";
	printf("Analyzing %s\n", strrr);
	unsigned * str = strToRuleIds(strrr, &len);
	LRStateTree * t = f(g, &mat, len, str);

	lrStateTreePrint(t, g);

	return 0;
}




