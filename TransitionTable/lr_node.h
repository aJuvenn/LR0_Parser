/*
 * lr_node.h
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_NODE_H_
#define LR_NODE_H_

#include "../lr_include.h"

typedef struct
{
	unsigned nbDotedRules;
	unsigned maxNbDotedRules;
	unsigned * ruleIndexes;
	unsigned * dotIndexes;

} LRNode;




LRNode * lrNodeNew();

void lrNodeFree(LRNode * node);

int lrNodeDotedRuleIsIn(const LRNode * const node, const unsigned ruleId, const unsigned dotId);

void lrNodeAddDotedRule(LRNode * const node, const unsigned ruleId, const unsigned dotId);
void lrNodeClose(const LRGrammar * const grammar, LRNode * const node);
int lrNodeAreEquals(const LRNode * const node1, const LRNode * const node2);

void lrNodePrint(const LRGrammar * const grammar, const LRNode * const node);
LRNode * lrNodeGetTransition(const LRGrammar * const grammar, const LRNode * const fromNode, const unsigned symbolId);

#endif /* LR_NODE_H_ */
