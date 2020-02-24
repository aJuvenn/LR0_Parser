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


void lrNodeEmpty(LRNode * const node)
{
	node->nbDotedRules = 0;
	node->maxNbDotedRules = 0;
	node->ruleIndexes = NULL;
	node->dotIndexes = NULL;
}

void lrNodeDeallocate(LRNode * const node)
{
	free(node->dotIndexes);
	free(node->ruleIndexes);
	node->nbDotedRules = 0;
	node->maxNbDotedRules = 0;
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
	for (unsigned t = 0 ; t < 10 ; t++){
		for (unsigned i = 0 ; i < node->nbDotedRules ; i++){

			unsigned ruleId = node->ruleIndexes[i];
			unsigned dotId = node->dotIndexes[i];

			if (dotId == grammar->rightRuleSizes[ruleId]){
				// TODO ?
				continue;
			}

			unsigned nextChar = grammar->rightRules[ruleId][dotId];

			if (!isNonTerminal(grammar, nextChar)){
				continue;
			}

			for (unsigned j = 0 ; j < grammar->nbRules ; j++){
				if (grammar->leftRules[j] == nextChar){
					lrNodeAddDotedRule(node, j, 0);
				}
			}
		}
	}
}



int lrNodeGetTransition(const LRGrammar * const grammar, const LRNode * const fromNode, const unsigned symbolId, LRNode * const toNode)
{
	//int aRuleHasMatched = 0;
	lrNodeEmpty(toNode);

	for (unsigned i = 0 ; i < fromNode->nbDotedRules ; i++){

		const unsigned ruleId = fromNode->ruleIndexes[i];
		const unsigned dotId = fromNode->dotIndexes[i];

		if (dotId == grammar->rightRuleSizes[ruleId] || symbolId != grammar->rightRules[ruleId][dotId]){
			continue;
		}

		/* fromNode contains a rule with a dot just before the symbol */

#if 0
		if (aRuleHasMatched){
			/* Several rules match, grammar is ambiguous */
			printf("Several rules\n");
			return EXIT_FAILURE;
		}
#endif

		//aRuleHasMatched = 1;

		lrNodeAddDotedRule(toNode, ruleId, dotId + 1);
	}

	lrNodeClose(grammar, toNode);

	return EXIT_SUCCESS;
}


typedef struct {

	unsigned nbSymbols;
	unsigned nbStates;
	unsigned maxNbStates;
	LRNode * stateNodes;

	unsigned * transitions;

} LRTransitionMatrix;



int lrTransitionMatrixNodeIsIn(const LRTransitionMatrix * const transitionMatrix, const LRNode * const node)
{
	for (unsigned i = 0 ; i < transitionMatrix->nbStates ; i++){

		if (lrNodeAreEquals(node, transitionMatrix->stateNodes + i)){
			return 1;
		}
	}

	return 0;
}


void lrTransitionMatrixAddNode(LRTransitionMatrix * const transitionMatrix, const LRNode * const node)
{
	if (transitionMatrix->nbStates == transitionMatrix->maxNbStates){
		transitionMatrix->maxNbStates = 2 * transitionMatrix->maxNbStates + 1;
		transitionMatrix->stateNodes = realloc(transitionMatrix->stateNodes, transitionMatrix->maxNbStates * sizeof(LRNode));
	}

	transitionMatrix->stateNodes[transitionMatrix->nbStates++] = *node;
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

	LRNode initialNode;
	lrNodeEmpty(&initialNode);
	lrNodeAddDotedRule(&initialNode, 0, 0);
	lrNodeClose(grammar, &initialNode);

	lrTransitionMatrixAddNode(transitionMatrix, &initialNode);

	for (unsigned t = 0 ; t < 10 ; t++){
		for (unsigned nodeId = 0 ; nodeId < transitionMatrix->nbStates ; nodeId++){

			printf("transitionMatrix->nbStates: %u\n", transitionMatrix->nbStates);
			LRNode * fromNode = transitionMatrix->stateNodes + nodeId;

			for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

				LRNode toNode;

				int ret = lrNodeGetTransition(grammar, fromNode, symbolId, &toNode);

				if (ret != EXIT_SUCCESS){
					printf ("FFFFAIL\n");
					return ret;
				}

				if (lrTransitionMatrixNodeIsIn(transitionMatrix, &toNode)){
					lrNodeDeallocate(&toNode);
				} else {
					lrTransitionMatrixAddNode(transitionMatrix, &toNode);
				}
			}
		}
	}


	return EXIT_SUCCESS;
}


unsigned lrTransitionMatrixGetNodeId(const LRTransitionMatrix * const transitionMatrix, const LRNode * const node)
{
	for (unsigned i = 0 ; i < transitionMatrix->nbStates; i++){
		if (lrNodeAreEquals(node, transitionMatrix->stateNodes + i)){
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

		LRNode * fromNode = transitionMatrix->stateNodes + fromNodeId;

		for (unsigned symbolId = 0 ; symbolId < nbSymbols ; symbolId++){

			LRNode toNode;
			int ret = lrNodeGetTransition(grammar, fromNode, symbolId, &toNode);

			if (ret != EXIT_SUCCESS){
				printf("FaaaaAAaAaaAAAAaail\n");
				return ret;
			}

			unsigned toNodeId = lrTransitionMatrixGetNodeId(transitionMatrix, &toNode);
			lrNodeDeallocate(&toNode);

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

		const LRNode * const node = transitionMatrix->stateNodes + i;
		int fillState = 0;
		unsigned ruleId;

		for (unsigned j = 0 ; j < node->nbDotedRules ; j++){

			ruleId = node->ruleIndexes[j];
			const unsigned dotId = node->dotIndexes[j];

			if (grammar->rightRuleSizes[ruleId] == dotId){
				fillState = 1;
				break;
			}
		}

		if (fillState){
			for (unsigned j = grammar->nbNonTerminal ; j < transitionMatrix->nbSymbols ; j++){
				transitionMatrix->transitions[j + transitionMatrix->nbSymbols * i] = -(ruleId + 1);
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

//static const char * symbolIdToChar = "EFN+*01()";
static const char * symbolIdToChar = "EB01+*";
//static const char * symbolIdToChar = "SEB01+*$";


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




typedef struct LRStateTree
{
	unsigned rule;
	unsigned symbol;

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

void lrStateTreePrintRec(LRStateTree * tree, unsigned indent)
{
	for (unsigned i = 0 ; i+1 < indent ; i++){
		printf(" ");
	}

	if (indent != 0){
		printf("-");
	}


	printf("%c", symbolIdToChar[tree->symbol]);
	printf("\n");

	for (unsigned i = 0 ; i < tree->nbSons ; i++){
		lrStateTreePrintRec(tree->sons[i], indent + 1);
	}
}

void lrStateTreePrint(LRStateTree * tree)
{
	lrStateTreePrintRec(tree, 0);
}




LRStateTree * f(LRGrammar * grammar, LRTransitionMatrix * transitions, unsigned nbSymbols, unsigned * symbols)
{
	LRStateTree * tmp;

	unsigned stackSize = 0;
	unsigned stack[1000];

	unsigned treeStackSize = 0;
	LRStateTree * treeStack[1000];

	stack[stackSize++] = 0;

	for (unsigned i = 0 ; i < nbSymbols ; i++){

		unsigned currentSymbol = symbols[i];
		unsigned currentSate = stack[stackSize - 1];

		int action = (int) lrTransitionMatrixGetNextStateId(transitions, currentSate, currentSymbol);

		if (action >= 0){
			// shift
			stack[stackSize++] = (unsigned) action;

			tmp = lrStateTreeNew(0);
			tmp->symbol = currentSymbol;
			tmp->rule = -1;
			treeStack[treeStackSize++] = tmp;

			printf("[%c][%u] Shift %d : [ ",symbolIdToChar[currentSymbol], currentSate, action);
			for (unsigned j = 0 ; j < stackSize ; j++){
				printf("%u ", stack[j]);
			}
			printf("]\n");
			continue;
		}

		// reduce
		action = -(action + 1);

		printf("[%c][%u] Reduce %d : [ ", symbolIdToChar[currentSymbol], currentSate, action);

		unsigned ruleSize = grammar->rightRuleSizes[action];

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
		treeStack[treeStackSize++] = tmp;

		for (unsigned j = 0 ; j < stackSize ; j++){
			printf("%u ", stack[j]);
		}

		printf("]\n");


		i--;

	}

	printf("Tree Stack Size : %u\n", treeStackSize);

	LRStateTree * output = lrStateTreeNew(treeStackSize);

	output->nbSons = treeStackSize;

	for (unsigned i = 0 ; i < treeStackSize ; i++){
		output->sons[i] = treeStack[i];
	}

	output->symbol = 0;

	return output;
}

int main(int argc, char **argv)
{
	const unsigned nbNonTerminal = 2;
	const unsigned nbTerminals = 4;

	LRGrammar * g = lrGrammarNew(nbNonTerminal, nbTerminals, 100);

	/*
	addStrRule(g, 'E', "E+F");
	addStrRule(g, 'E', "F");

	addStrRule(g, 'F', "F*N");
	addStrRule(g, 'F', "N");

	addStrRule(g, 'N', "0");
	addStrRule(g, 'N', "1");
	addStrRule(g, 'N', "(E)");
	 */

	//addStrRule(g, 'S', "E$");
	addStrRule(g, 'E', "E*B");
	addStrRule(g, 'E', "E+B");
	addStrRule(g, 'E', "B");

	addStrRule(g, 'B', "0");
	addStrRule(g, 'B', "1");



	printf("Creating table...\n");

	LRTransitionMatrix mat;
	int ret = lrTransitionMatrixFromGrammar(g, &mat);

	if (ret != EXIT_SUCCESS){
		printf("FAIL.\n");
	} else {
		printf("Done.\n");
		print_grammar(g);
		print_mat(&mat);
	}

	printf("\n\n");

	unsigned int len;
	unsigned * str = strToRuleIds("0*1+0*1*1*0+0", &len);
	LRStateTree * t = f(g, &mat, len, str);

	lrStateTreePrint(t);

	return 0;
}




