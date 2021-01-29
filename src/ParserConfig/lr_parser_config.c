/*
 * parse_config.c
 *
 *  Created on: 25 févr. 2020
 *      Author: ajuvenn
 */


#include "../lr_include.h"



static int symbolCompare(const void * ptr1, const void * ptr2)
{
	const LRTmpConfigSymbol * sym1 = ptr1;
	const LRTmpConfigSymbol * sym2 = ptr2;
	return ((int) sym1->symbolId) - ((int) sym2->symbolId);
}

static int ruleCompare(const void * ptr1, const void * ptr2)
{
	const LRTmpConfigRule * rule1 = ptr1;
	const LRTmpConfigRule * rule2 = ptr2;
	return ((int) rule1->ruleId) - ((int) rule2->ruleId);
}


int lrTmpConfigSortingAndVerification(LRTmpConfig * const tmpConfig)
{
	qsort(tmpConfig->symbols, tmpConfig->nbSymbols, sizeof(LRTmpConfigSymbol),
		  symbolCompare);

	qsort(tmpConfig->rules, tmpConfig->nbRules, sizeof(LRTmpConfigRule),
		  ruleCompare);

	for (unsigned i = 0 ; i < tmpConfig->nbSymbols ; i++){
		if (tmpConfig->symbols[i].symbolId != i){
			return EXIT_FAILURE;
		}
	}

	for (unsigned i = 0 ; i < tmpConfig->nbRules ; i++){
		if (tmpConfig->rules[i].ruleId != i){
			return EXIT_FAILURE;
		}
	}

	for (unsigned i = 0 ; i < tmpConfig->nbSymbols ; i++){
		for (unsigned j = 0 ; j < i ; j++){
			if (!strcmp(tmpConfig->symbols[i].symbolName, tmpConfig->symbols[j].symbolName)){
				return EXIT_FAILURE;
			}
			if (!strcmp(tmpConfig->symbols[i].symbolRegex, tmpConfig->symbols[j].symbolRegex)){
				return EXIT_FAILURE;
			}
		}
	}

	for (unsigned i = 0 ; i < tmpConfig->nbRules ; i++){
		for (unsigned j = 0 ; j < i ; j++){
			if (!strcmp(tmpConfig->rules[i].nonTerminalName, tmpConfig->rules[j].nonTerminalName)
					&& !strcmp(tmpConfig->rules[i].derivationString, tmpConfig->rules[j].derivationString)){
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}





static void findBeginAndEndOfFirstWord(const char * const str, unsigned * outWordBeginId, unsigned * outWordEndId)
{
	unsigned beginId, endId;

	beginId = 0;

	while (isspace(str[beginId])){
		beginId++;
	}

	endId = beginId;

	while (str[endId] != '\0' && !isspace(str[endId])){
		endId++;
	}

	*outWordBeginId = beginId;
	*outWordEndId = endId;
}

#if 0
static char * copyFirstWord(const char * const str)
{
	unsigned beginId, endId;
	findBeginAndEndOfFirstWord(str, &beginId, &endId);
	const unsigned wordSize = endId - beginId;
	if (wordSize == 0){
		return NULL;
	}
	return strndup(str + beginId, wordSize);
}
#endif


static int splitBySpaces(const char * const str, unsigned * outNbSubStrings, char *** outSubStrings)
{
	unsigned strLength = strlen(str);
	unsigned nbSubStrings = 0;
	char * subStrings[strLength];
	const char * cursor = str;

	while (1){
		unsigned beginId, endId;
		findBeginAndEndOfFirstWord(cursor, &beginId, &endId);
		const unsigned wordSize = endId - beginId;
		if (wordSize == 0){
			break;
		}
		char * const wordCopy = strndup(cursor + beginId, wordSize);
		if (wordCopy == NULL) goto ERR1;
		subStrings[nbSubStrings++] = wordCopy;
		cursor += endId;
	}

	*outSubStrings = malloc(nbSubStrings * sizeof(char *));
	if (*outSubStrings == NULL) goto ERR1;
	memcpy(*outSubStrings, subStrings, nbSubStrings * sizeof(char *));
	*outNbSubStrings = nbSubStrings;

	return EXIT_SUCCESS;

	ERR1:
	for (unsigned i = 0 ; i < nbSubStrings ; i++){
		free(subStrings[i]);
	}

	return EXIT_FAILURE;
}




LRConfig * lrConfigNew(LRTmpConfig * tmpConfig)
{
	LRConfig * output = malloc(sizeof(LRConfig));
	if (output == NULL) return NULL;

	output->tokens = malloc(tmpConfig->nbSymbols * sizeof(LRConfigToken));

	if (output->tokens == NULL){
		free(output);
		return NULL;
	}

	output->grammarRules = malloc(tmpConfig->nbRules * sizeof(LRConfigRule));
	if (output->grammarRules == NULL) {
		free(output->tokens);
		free(output);
		return NULL;
	}

	output->nbTokens = tmpConfig->nbSymbols;
	output->nbGrammarRules = tmpConfig->nbRules;
	output->skippedTokenId = -1;

	for (unsigned i = 0 ; i < tmpConfig->nbSymbols ; i++){
		unsigned tokenId = tmpConfig->symbols[i].symbolId;
		output->tokens[tokenId].name = tmpConfig->symbols[i].symbolName;
		output->tokens[tokenId].regex = tmpConfig->symbols[i].symbolRegex;
		if (!strcmp(output->tokens[tokenId].name, "<SKIPPED>")){
			output->skippedTokenId = tokenId;
		}
	}

	int8_t ruleAreFilled[tmpConfig->nbRules];
	memset(ruleAreFilled, 0, tmpConfig->nbRules * sizeof(int8_t));

	for (unsigned i = 0 ; i < tmpConfig->nbRules ; i++){
		unsigned ruleId = tmpConfig->rules[i].ruleId;
		unsigned nbRightRuleMembers; char ** rightRuleMembers;
		int ret = splitBySpaces(tmpConfig->rules[i].derivationString, &nbRightRuleMembers, &rightRuleMembers);
		if (ret != EXIT_SUCCESS) goto ERR3;
		output->grammarRules[ruleId].leftMember = tmpConfig->rules[i].nonTerminalName;
		output->grammarRules[ruleId].nbRightMembers = nbRightRuleMembers;
		output->grammarRules[ruleId].rightMembers = rightRuleMembers;
		ruleAreFilled[i] = 1;
	}

	return output;

	ERR3:
	for (unsigned i = 0 ; i < tmpConfig->nbRules ; i++){
		if (!(ruleAreFilled[i])){
			continue;
		}
		for (unsigned j = 0 ; j < output->grammarRules[i].nbRightMembers ; j++){
			free(output->grammarRules[i].rightMembers[j]);
		}
		free(output->grammarRules[i].rightMembers);
	}
	free(output->grammarRules);
	free(output->tokens);
	free(output);
	return NULL;
}




void lrConfigFree(LRConfig * tmpConfig)
{
	/* TODO */
}


#if 0

void lrFileConfigFree(LRFileConfig * conf)
{
	free(conf->ruleRightMemberSizes);

	for (unsigned i = 0 ; i < conf->nbGrammarRules ; i++){
		free(conf->ruleRightMembers[i]);
	}

	free(conf->ruleRightMembers);
	free(conf->ruleLeftMembers);
	free(conf->tokenNames);
	free(conf->tokenRegexp);
	free(conf);
}



void lrFileConfigPrint(LRFileConfig * descr)
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
#endif
