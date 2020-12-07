/*
 * main.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */


#include "lr_include.h"



int main(int argc, char **argv)
{
	char * configPath = "funl_grammar.txt";

	LRParser * parser = lrParserNew(configPath);

	if (parser == NULL){
		fprintf(stderr, "Couldn't load parser\n");
		exit(EXIT_FAILURE);
	}


	lrGrammarPrint(parser->grammar);

	lrTransitionMatrixPrint(parser->transition, parser->grammar);
	LRParseTree * tree = lrParserParseFile(parser, "source_file.txt");

	if (tree == NULL){
		fprintf(stderr, "Couldn't parse file\n");
		exit(EXIT_FAILURE);
	}

	printf("\n\tParsed tree:\n");

	lrParseTreePrint(tree, parser->grammar);
	lrParseTreeFree(tree);
	lrParserFree(parser);

	return 0;
}
