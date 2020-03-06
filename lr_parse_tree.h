/*
 * lr_parse_tree.h
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#ifndef LR_PARSE_TREE_H_
#define LR_PARSE_TREE_H_

#include "lr_include.h"



typedef struct LRParseTree
{
	int isLeaf;

	union {

		struct {
			unsigned nonTerminalId;
			unsigned ruleId;
			unsigned nbSons;
			struct LRParseTree ** sons;
		} node;

		struct {
			unsigned tokenId;
			char * tokenData;
		} leaf;
	};

} LRParseTree;


LRParseTree * lrParseTreeNew(const unsigned maxNbSons);

void lrParseTreePrint(const LRParseTree * const tree , const LRGrammar * const g);



#endif /* LR_PARSE_TREE_H_ */
