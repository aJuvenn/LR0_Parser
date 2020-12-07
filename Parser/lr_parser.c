/*
 * parser.c
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#include "../lr_include.h"



LRParser * lrParserNew(const char * const configFilePath)
{
	LRParser *  output = malloc(sizeof(LRParser));

	if (output == NULL){
		goto ERR0;
	}

	lrFileConfig * conf = lrFileConfigParse(configFilePath);

	if (conf == NULL){
		goto ERR1;
	}

	output->nbTokens = conf->nbTokens;
	output->tokenAutomatons = malloc(output->nbTokens * sizeof(cdfa__automaton *));
	output->skippedAutomatonId = conf->skippedToken;

	if (output->tokenAutomatons == NULL){
		goto ERR2;
	}

	unsigned tokenId;

	for (tokenId = 0 ; tokenId < output->nbTokens ; tokenId++){

		output->tokenAutomatons[tokenId] = cdfa__expression_recognizing_automaton(conf->tokenRegexp[tokenId]);

		if (output->tokenAutomatons[tokenId] == NULL){
			goto ERR3;
		}
	}

	output->grammar = lrGrammarFromDescr(conf);

	if (output->grammar == NULL){
		goto ERR3;
	}

	output->transition = lrTransitionMatrixFromGrammar(output->grammar);

	if (output->transition == NULL){
		goto ERR4;
	}

	return output;

	ERR4: // TODO free grammar

	ERR3:
	for (unsigned i = 0 ; i < tokenId ; i++){
		cdfa__free_automaton(output->tokenAutomatons[i]);
	}

	free(output->tokenAutomatons);

	ERR2: // TODO free conf
	ERR1: free(output);
	ERR0: return NULL;
}


void lrParserFree(LRParser * p)
{
	for (unsigned i = 0 ; i < p->nbTokens ; i++){
		cdfa__free_automaton(p->tokenAutomatons[i]);
	}

	free(p->tokenAutomatons);
	lrGrammarFree(p->grammar);
	lrTransitionMatrixFree(p->transition);
	free(p);
}


int lrParserSplitNextWord(LRParser *  parser,
						  const char * str,
						  unsigned * out_wordBegin, unsigned * out_wordEnd,
						  unsigned * out_automatonId)
{
	unsigned nbAutomatons = parser->nbTokens;
	cdfa__automaton ** automatons = parser->tokenAutomatons;

	unsigned nbStillRunningAutomatons = nbAutomatons;
	char stillRunning[nbAutomatons];
	memset(stillRunning, 1, nbAutomatons);

	for (unsigned i = 0 ; i < nbAutomatons ; i++){
		cdfa__move_to_starting_state(automatons[i]);
	}

	unsigned charId = 0;
	unsigned lastValidCharId = (unsigned) -1;
	unsigned lastValidAutomatonId = (unsigned) -1;

	while (isspace(str[charId])){
		charId++;
	}

	unsigned wordBegin = charId;
	char c;

	if (str[charId] == '\0' || str[charId] == EOF){
		*out_wordBegin = wordBegin;
		*out_wordEnd = wordBegin;
		*out_automatonId = (unsigned) -1;
		return EXIT_SUCCESS;
	}

	do {

		c = str[charId];

		for (unsigned i = nbAutomatons ; i > 0 ; i--){
			/*
			 * Automaton list treatment is reversed so that firsts automatons have
			 * an higher priority in case of conflict
			 */
			unsigned automatonId = i-1;

			if (!stillRunning[automatonId]){
				continue;
			}

			cdfa__automaton_state state = cdfa__move_to_next_state(c, automatons[automatonId]);

			if (state == CDFA__WELL){
				stillRunning[automatonId] = 0;
				nbStillRunningAutomatons--;

			} else if (cdfa__is_a_final_state(state, automatons[automatonId])){
				// Higher priority for low indexes since they are the last in the loop
				lastValidCharId = charId;
				lastValidAutomatonId = automatonId;
			}
		}

		charId++;

	} while (nbStillRunningAutomatons != 0 && c != '\0' && c != EOF);

	if (lastValidCharId == (unsigned) -1){
		return EXIT_FAILURE;
	}

	*out_wordBegin = wordBegin;
	*out_wordEnd = lastValidCharId + 1;
	*out_automatonId = lastValidAutomatonId;

	return EXIT_SUCCESS;
}



int lrParserNextToken(LRParser * parser, const char ** cursor, unsigned * out_tokenId, char ** out_tokenData)
{
	int ret;
	unsigned wordBegin;
	unsigned wordEnd;
	unsigned automatonId;
	unsigned wordSize;

	while (1){

		ret = lrParserSplitNextWord(parser, *cursor, &wordBegin, &wordEnd, &automatonId);

		if (ret != EXIT_SUCCESS){
			return ret;
		}

		wordSize = wordEnd - wordBegin;

		if (wordSize == 0){
			/* End of file */
			*cursor += wordEnd;
			*out_tokenId = parser->grammar->nbSymbols - 1;
			*out_tokenData = strdup("<eof>");
			return EXIT_SUCCESS;
		}

		if (automatonId != parser->skippedAutomatonId){
			break;
		}

		*cursor += wordEnd;
	}

	char * tokenData = malloc(wordSize + 1);

	if (tokenData == NULL){
		return EXIT_FAILURE;
	}

	memcpy(tokenData, *cursor + wordBegin, wordSize);
	tokenData[wordSize] = '\0';

	*cursor += wordEnd;
	*out_tokenId = (automatonId + parser->grammar->nbNonTerminal);
	*out_tokenData = tokenData;

	return EXIT_SUCCESS;
}


LRParseTree * lrParserParseStr(LRParser * const parser, const char * const str)
{
	LRGrammar * grammar = parser->grammar;
	LRTransitionMatrix * transitions = parser->transition;

	LRParseTree * tmp;

	unsigned stateStackSize = 0;
	unsigned * stateStack = malloc(LR_PARSER_MAX_DEPTH * sizeof(unsigned));

	if (stateStack == NULL){
		return NULL;
	}

	stateStack[stateStackSize++] = 1;


	unsigned treeStackSize = 0;
	LRParseTree ** treeStack = malloc(LR_PARSER_MAX_DEPTH * sizeof(LRParseTree *));

	if (treeStack == NULL){
		free(stateStack);
		return NULL;
	}


	const char * cursor = str;

	unsigned currentTokenId;
	char * currentTokenData;
	const unsigned endOfFileToken = parser->grammar->nbSymbols - 1;

	int ret;

	ret = lrParserNextToken(parser, &cursor, &currentTokenId, &currentTokenData);

	if (ret != EXIT_SUCCESS || currentTokenId == endOfFileToken){
		return NULL;
	}

	int eofAlreadyEncountered = 0;

	while (1){

		unsigned currentSate = stateStack[stateStackSize - 1];

		if (currentSate == 0){
			return NULL;
		}

		int action = (int) lrTransitionMatrixGetNextStateId(transitions, currentSate, currentTokenId);

		if (action >= 0){
			/* shift */
			stateStack[stateStackSize++] = (unsigned) action;

			tmp = lrParseTreeNew(0);
			tmp->isLeaf = 1;
			tmp->leaf.tokenId = currentTokenId;
			tmp->leaf.tokenData = currentTokenData;

			treeStack[treeStackSize++] = tmp;

			ret = lrParserNextToken(parser, &cursor, &currentTokenId, &currentTokenData);

			if (ret != EXIT_SUCCESS){
				return NULL;
			}

			if (currentTokenId == endOfFileToken){
				if (eofAlreadyEncountered){
					break;
				} else {
					eofAlreadyEncountered = 1;
				}
			}

			continue;
		}

		/* reduce */
		action = -(action + 1);

		unsigned ruleSize = grammar->rightRuleSizes[action];

		tmp = lrParseTreeNew(ruleSize);
		tmp->isLeaf = 0;
		tmp->node.nbSons = ruleSize;
		for (unsigned j = 0 ; j < ruleSize ; j++){
			tmp->node.sons[j] = treeStack[treeStackSize-ruleSize + j];
		}

		tmp->node.nonTerminalId = grammar->leftRules[action];
		tmp->node.ruleId = action;

		stateStackSize -= ruleSize;
		treeStackSize -= ruleSize;
		currentSate = stateStack[stateStackSize - 1];

		unsigned new_state = lrTransitionMatrixGetNextStateId(transitions, currentSate, grammar->leftRules[action]);
		stateStack[stateStackSize++] = new_state;
		treeStack[treeStackSize++] = tmp;
	}

	if (treeStackSize != 2 || treeStack[1]->isLeaf != 1 || treeStack[1]->leaf.tokenId != endOfFileToken){
		for (unsigned i = 0 ; i < treeStackSize ; i++){
			lrParseTreeFree(treeStack[i]);
		}
		free(treeStack);
		free(stateStack);
		return NULL;
	}

	LRParseTree * output = treeStack[0];

	lrParseTreeFree(treeStack[1]);
	free(treeStack);
	free(stateStack);

	return output;
}



LRParseTree * lrParserParseFile(LRParser * parser, const char * const filePath)
{
	char * buffer = lrLoadFile(filePath);

	if (buffer == NULL){
		return NULL;
	}

	LRParseTree * output = lrParserParseStr(parser, buffer);

	free(buffer);

	return output;
}
