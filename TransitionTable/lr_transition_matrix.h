/*
 * lr_transition_matrix.h
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_TRANSITION_MATRIX_H_
#define LR_TRANSITION_MATRIX_H_

#include "../lr_include.h"



typedef struct
{
	unsigned nbSymbols;
	unsigned nbStates;
	unsigned maxNbStates;
	LRNode ** stateNodes;

	unsigned * transitions;

} LRTransitionMatrix;


LRTransitionMatrix * lrTransitionMatrixFromGrammar(const LRGrammar * const grammar);

void lrTransitionMatrixFree(LRTransitionMatrix * mat);

unsigned lrTransitionMatrixGetNextStateId(const LRTransitionMatrix * const transitionMatrix, const unsigned stateId, const unsigned symbolId);



#endif /* LR_TRANSITION_MATRIX_H_ */
