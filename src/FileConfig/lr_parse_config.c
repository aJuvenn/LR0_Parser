/*
 * parse_config.c
 *
 *  Created on: 25 févr. 2020
 *      Author: ajuvenn
 */


#include "../lr_include.h"



static int split(char * buffer, const char * const sep, unsigned * out_nbParts, char *** out_parts)
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




static int splitLines(char * buffer, unsigned * out_nbTokenLines, char *** out_tokenLines, unsigned * out_nbGrammarLines, char *** out_grammarLines)
{
	unsigned nbTokenLines = 0;
	char ** tokenLines = malloc(MAX_NB_LINES * sizeof(char *));

	if (tokenLines == NULL){
		return EXIT_FAILURE;
	}

	unsigned nbGrammarLines = 0;
	char ** grammarLines = malloc(MAX_NB_LINES * sizeof(char *));

	if (grammarLines == NULL){
		free(tokenLines);
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


/*
 * Looks for the first occurrence of sep and returns two substrings
 */
static int separateInTwo(const char * const buffer, const char * const sep, char ** out_left, char ** out_right)
{
	const size_t bufferSize = strlen(buffer);
	const size_t sepSize = strlen(sep);

	if (bufferSize < sepSize){
		return EXIT_FAILURE;
	}

	char * left = NULL;
	char * right = NULL;

	for (size_t i = 0 ; buffer[i + sepSize] != 0 ; i++){

		if (strncmp(sep, buffer + i, sepSize)){
			continue;
		}

		left = malloc((i + 1) * sizeof(char));

		if (left == NULL){
			return EXIT_FAILURE;
		}

		memcpy(left, buffer, i * sizeof(char));
		left[i] = 0;

		right = malloc((bufferSize - i - sepSize + 1) * sizeof(char));

		if (right == NULL){
			free(left);
			return EXIT_FAILURE;
		}

		memcpy(right, buffer + i + sepSize, (bufferSize - i - sepSize) * sizeof(char));
		right[bufferSize - i - sepSize] = 0;
		break;
	}

	*out_left = left;
	*out_right = right;

	return EXIT_SUCCESS;
}


static char * removeSpaceFromVarName(const char * const varName)
{
	unsigned beginId, endId;

	beginId = 0;

	while (isspace(varName[beginId])){
		beginId++;
	}

	endId = beginId;

	while (varName[endId] != '\0' && !isspace(varName[endId])){
		endId++;
	}

	const unsigned varSize = endId - beginId;
	char * output = malloc(varSize + 1);

	if (output == NULL){
		return NULL;
	}

	memcpy(output, varName + beginId, varSize);
	output[varSize] = '\0';

	return output;
}





static int separateLeftRight(unsigned nbLines, char ** lines, char * sep, char *** out_lefts, char *** out_rights)
{
	char ** lefts = malloc(nbLines * sizeof(char *));
	char ** rights = malloc(nbLines * sizeof(char *));

	for (unsigned i = 0 ; i < nbLines ; i++){
		char * left;
		char * right;
		int ret = separateInTwo(lines[i], sep, &left, &right);

		if (ret != EXIT_SUCCESS){
			return ret;
		}

		if (left == NULL || right == NULL){
			return EXIT_FAILURE;
		}

		lefts[i] = removeSpaceFromVarName(left);
		free(left);
		rights[i] = right;
	}

	*out_lefts = lefts;
	*out_rights = rights;

	return EXIT_SUCCESS;
}












lrFileConfig * lrFileConfigParse(const char * const path)
{
	lrFileConfig * output = malloc(sizeof(lrFileConfig));

	if (output == NULL){
		return NULL;
	}

	char * buff = lrLoadFile(path);

	if (buff == NULL){
		free(output);
		return NULL;
	}


	unsigned nbTokenLines;
	char ** tokenLines;

	unsigned nbGrammarLines;
	char ** grammarLines;
	int ret;

	splitLines(buff, &nbTokenLines, &tokenLines, &nbGrammarLines, &grammarLines);

	char ** leftTokens;
	char ** rightTokens;
	ret = separateLeftRight(nbTokenLines, tokenLines, ":=", &leftTokens, &rightTokens);

	if (ret != EXIT_SUCCESS){
		return NULL;
	}

	char ** leftRules;
	char ** rightRules;
	ret = separateLeftRight(nbGrammarLines, grammarLines, "->", &leftRules, &rightRules);


	if (ret != EXIT_SUCCESS){
		return NULL;
	}

	output->nbTokens = nbTokenLines;
	output->tokenNames = leftTokens;
	output->tokenRegexp = rightTokens;

	output->skippedToken = -1;

	for (unsigned i = 0 ; i < nbTokenLines ; i++){
		if (strcmp(leftTokens[i], "<SKIPPED>") == 0){
			output->skippedToken = i;
		}
	}

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

void lrFileConfigFree(lrFileConfig * conf)
{
	free(conf->ruleRightMemberSizes);
	free(conf->ruleRightMembers);
	free(conf->ruleLeftMembers);
	free(conf->tokenNames);
	free(conf->tokenRegexp);
	free(conf);
}



void lrFileConfigPrint(lrFileConfig * descr)
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

