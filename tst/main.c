/*
 * main.c
 *
 *  Created on: 6 mars 2020
 *      Author: ajuvenn
 */

#include <stdio.h>
#include <stdlib.h>
#include "../lr_parser.h"



int main(int argc, char **argv)
{
	char * configPath = "new_funl_grammar.txt";
	const char * errorMessage;

	printf("Creating parser from %s...\n", configPath);

	LRParser * parser = lrParserNew(configPath, &errorMessage);

	if (parser == NULL){
		fprintf(stderr, "Couldn't load parser\n");
		exit(EXIT_FAILURE);
	}

	printf("Done. Printing parser...\n");

	lrParserPrint(parser);

	char * sourceFile = "source_file.txt";

	printf("Done. Parsing %s...\n", sourceFile);

	LRParseTree * tree = lrParserParseFile(parser, sourceFile, &errorMessage);

	if (tree == NULL){
		fprintf(stderr, "Couldn't parse file\n");
		exit(EXIT_FAILURE);
	}

	printf("Done. Printing parse tree...\n");

	lrParseTreePrint(parser, tree);

	printf("Done.\n");

	lrParseTreeFree(parser, tree);
	lrParserFree(parser);

	return 0;
}
