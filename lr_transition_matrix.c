/*
 * lr_transition_matrix.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include "lr_include.h"



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


LRTransitionMatrix * lrTransitionMatrixFromGrammar(const LRGrammar * const grammar)
{
	int ret;
	LRTransitionMatrix * transitionMatrix = malloc(sizeof(LRTransitionMatrix));

	lrTransitionMatrixEmpty(transitionMatrix);
	ret = lrTransitionMatrixFillStates(grammar, transitionMatrix);

	if (ret != EXIT_SUCCESS){
		printf("fill fail\n");
		return NULL;
	}

	ret = lrTransitionMatrixFillTransitions(grammar, transitionMatrix);

	if (ret != EXIT_SUCCESS){
		printf("transition fail\n");
		return NULL;
	}

	lrTransitionMatrixFillReduce(grammar, transitionMatrix);

	return transitionMatrix;
}



void lrTransitionMatrixFree(LRTransitionMatrix * mat)
{
	for (unsigned i = 0 ; i < mat->nbStates ; i++){
		lrNodeFree(mat->stateNodes[i]);
	}

	free(mat->stateNodes);
	free(mat->transitions);
	free(mat);
}



void lrTransitionMatrixPrint(LRTransitionMatrix * mat, LRGrammar * g)
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

