/*
 * parse_config.c
 *
 *  Created on: 25 févr. 2020
 *      Author: ajuvenn
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_NB_LINES 1024


char * loadFile(const char * const path)
{
	FILE * f;
	unsigned fileLength;
	char * output;

	f = fopen(path, "r");
	fseek(f, 0, SEEK_END);
	fileLength = ftell(f);
	output = malloc(fileLength);
	rewind(f);
	fread(output, fileLength + 1, 1, f);
	fclose(f);

	output[fileLength] = '\0';

	return output;
}


int splitLines(const char * const buffer, unsigned * out_nbTokenLines, char *** out_tokenLines, unsigned * out_nbGrammarLines, char *** out_grammarLines)
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

	(void) (strtok(NULL, sep) == NULL);
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

typedef struct
{
	unsigned nbTokens;
	char ** tokenNames;
	char ** tokenRegexp;

	unsigned nbGrammarRules;
	char ** ruleLeftMembers;
	unsigned * ruleRightMemberSizes;
	char *** ruleRightMembers;

} GrammarDescription;










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
	separateLeftRight(nbTokenLines, tokenLines, " := ", &leftTokens, &rightTokens);

	char ** leftRules;
	char ** rightRules;
	separateLeftRight(nbGrammarLines, grammarLines, " -> ", &leftRules, &rightRules);


	/*
	 * TODO
	 *
	 */



	return output;
}


int main(int argc, char **argv)
{
	char * buff = loadFile("grammar.txt");
	printf("%s", buff);


	unsigned nbTokenLines;
	char ** tokenLines;

	unsigned nbGrammarLines;
	char ** grammarLines;

	splitLines(buff, &nbTokenLines, &tokenLines, &nbGrammarLines, &grammarLines);


	printf("TOKEN LINES [%u]\n", nbTokenLines);

	for (unsigned i = 0 ; i < nbTokenLines ; i++){
		printf("%s\n", tokenLines[i]);
	}

	printf("GRAMMAR LINES [%u]\n", nbGrammarLines);


	for (unsigned i = 0 ; i < nbGrammarLines ; i++){
		printf("%s\n", grammarLines[i]);
	}

	char ** leftTokens;
	char ** rightTokens;
	separateLeftRight(nbTokenLines, tokenLines, " := ", &leftTokens, &rightTokens);

	for (unsigned i = 0 ; i < nbTokenLines ; i++){
		printf("%s, ", leftTokens[i]);
	}

	printf("\n");

	for (unsigned i = 0 ; i < nbTokenLines ; i++){
		printf("%s, ", rightTokens[i]);
	}

	printf("\n");


	free(buff);
	return 0;
}
