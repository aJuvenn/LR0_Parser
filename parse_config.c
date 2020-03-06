/*
 * parse_config.c
 *
 *  Created on: 25 févr. 2020
 *      Author: ajuvenn
 */


#include "lr_include.h"


char * loadFile(const char * const path)
{
	FILE * f;
	unsigned fileLength;
	char * output;

	f = fopen(path, "r");
	fseek(f, 0, SEEK_END);
	fileLength = ftell(f);
	output = malloc(fileLength+1);
	rewind(f);
	fread(output, fileLength, 1, f);
	fclose(f);

	output[fileLength] = '\0';

	return output;
}



int split(char * buffer, const char * const sep, unsigned * out_nbParts, char *** out_parts)
{
	unsigned nbParts = 0;
	char ** parts = malloc(MAX_NB_LINES * sizeof(char *));

	if (parts == NULL){
		return EXIT_FAILURE;
	}

	for (char * part = strtok(buffer, sep) ; part != NULL ; part = strtok(NULL, sep)){
		parts[nbParts++] = part;
	}

	*out_nbParts = nbParts;
	*out_parts = parts;

	return EXIT_SUCCESS;
}


int splitLines(char * buffer, unsigned * out_nbTokenLines, char *** out_tokenLines, unsigned * out_nbGrammarLines, char *** out_grammarLines)
{
	unsigned nbTokenLines = 0;
	char ** tokenLines = malloc(MAX_NB_LINES * sizeof(char *));

	if (tokenLines == NULL){
		return EXIT_FAILURE;
	}

	unsigned nbGrammarLines = 0;
	char ** grammarLines = malloc(MAX_NB_LINES * sizeof(char *));

	if (grammarLines == NULL){
		return EXIT_FAILURE;
	}

	char * line = strtok(buffer, "\n");

	if (line == NULL || line[0] != '#'){
		return EXIT_FAILURE;
	}

	while (1){

		line = strtok(NULL, "\n");

		if (line == NULL || line[0] == '#'){
			break;
		}

		tokenLines[nbTokenLines++] = line;
	}

	while (1){

		line = strtok(NULL, "\n");

		if (line == NULL || line[0] == '#'){
			break;
		}

		grammarLines[nbGrammarLines++] = line;
	}

	*out_nbTokenLines = nbTokenLines;
	*out_tokenLines = tokenLines;

	*out_nbGrammarLines = nbGrammarLines;
	*out_grammarLines = grammarLines;

	return EXIT_SUCCESS;
}


void separate(char * line, char * sep, char ** out_left, char ** out_right)
{
	char * left = strtok(line, sep);
	char * right = strtok(NULL, sep);

	*out_left = left;
	*out_right = right;
}


void separateLeftRight(unsigned nbLines, char ** lines, char * sep, char *** out_lefts, char *** out_rights)
{
	char ** lefts = malloc(nbLines * sizeof(char *));
	char ** rights = malloc(nbLines * sizeof(char *));

	for (unsigned i = 0 ; i < nbLines ; i++){
		separate(lines[i], sep, lefts + i, rights + i);
	}

	*out_lefts = lefts;
	*out_rights = rights;
}










GrammarDescription * parseConfigFile(const char * const path)
{
	GrammarDescription * output = malloc(sizeof(GrammarDescription));
	char * buff = loadFile(path);

	unsigned nbTokenLines;
	char ** tokenLines;

	unsigned nbGrammarLines;
	char ** grammarLines;

	splitLines(buff, &nbTokenLines, &tokenLines, &nbGrammarLines, &grammarLines);

	char ** leftTokens;
	char ** rightTokens;
	separateLeftRight(nbTokenLines, tokenLines, "=", &leftTokens, &rightTokens);

	char ** leftRules;
	char ** rightRules;
	separateLeftRight(nbGrammarLines, grammarLines, "=", &leftRules, &rightRules);

	output->nbTokens = nbTokenLines;
	output->tokenNames = leftTokens;
	output->tokenRegexp = rightTokens;

	output->nbGrammarRules = nbGrammarLines;

	output->ruleLeftMembers = leftRules;

	output->ruleRightMembers = malloc(nbGrammarLines * sizeof(char **));
	output->ruleRightMemberSizes = malloc(nbGrammarLines * sizeof(unsigned));


	for (unsigned i = 0 ; i < nbGrammarLines ; i++){

		unsigned rightRuleSize;
		char ** rightRule;
		split(rightRules[i], " ", &rightRuleSize, &rightRule);

		output->ruleRightMemberSizes[i] = rightRuleSize;
		output->ruleRightMembers[i] = rightRule;
	}

	return output;
}


void printConfigFile(GrammarDescription * descr)
{
	printf("# Tokens #\n");

	for (unsigned i = 0 ; i < descr->nbTokens ; i++){
		printf("[%u]\n", i);
		printf("\tName : '%s'\n", descr->tokenNames[i]);
		printf("\tRegexp : '%s' \n", descr->tokenRegexp[i]);
	}

	printf("# Grammar Rules #\n");

	for (unsigned i = 0 ; i < descr->nbGrammarRules ; i++){

		printf("[%u]\n", i);
		printf("\tLeft : '%s'\n", descr->ruleLeftMembers[i]);
		printf("\tSubstitution (%u terms): ", descr->ruleRightMemberSizes[i]);

		for (unsigned j = 0 ; j < descr->ruleRightMemberSizes[i] ; j++){
			printf("'%s' ", descr->ruleRightMembers[i][j]);
		}

		printf("\n");
	}
}

