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


int lrParserSplitNextWord(LRParser *  parser, const char * str, unsigned * out__word_begin, unsigned * out__word_end, unsigned * out__automaton_id)
{
	unsigned nb_automatons = parser->nbTokens;
	cdfa__automaton ** automatons = parser->tokenAutomatons;

	unsigned nb_still_running_automatons = nb_automatons;
	char still_running[nb_automatons];
	memset(still_running, 1, nb_automatons);

	for (unsigned i = 0 ; i < nb_automatons ; i++){
		cdfa__move_to_starting_state(automatons[i]);
	}

	unsigned last_valid_char_id = (unsigned) -1;
	unsigned last_valid_automaton_id = (unsigned) -1;
	unsigned char_id = 0;

	while (isspace(str[char_id])){
		char_id++;
	}

	unsigned word_begin = char_id;
	char c;

	if (str[char_id] == '\0' || str[char_id] == EOF){
		*out__word_begin = word_begin;
		*out__word_end = word_begin;
		*out__automaton_id = (unsigned) -1;
		return EXIT_SUCCESS;
	}

	do {

		c = str[char_id];

		for (unsigned i = nb_automatons ; i > 0 ; i--){
			/*
			 * Automaton list treatment is reversed so that firsts automatons have
			 * an higher priority in case of conflict
			 */
			unsigned automaton_id = i-1;

			if (!still_running[automaton_id]){
				continue;
			}

			cdfa__automaton_state state = cdfa__move_to_next_state(c, automatons[automaton_id]);

			if (state == CDFA__WELL){
				still_running[automaton_id] = 0;
				nb_still_running_automatons--;

			} else if (cdfa__is_a_final_state(state, automatons[automaton_id])){
				// Higher priority for low indexes since they are the last in the loop
				last_valid_char_id = char_id;
				last_valid_automaton_id = automaton_id;
			}
		}

		char_id++;

	} while (nb_still_running_automatons != 0 && c != '\0' && c != EOF);


	if (last_valid_char_id == (unsigned) -1){
		return EXIT_FAILURE;
	}

	*out__word_begin = word_begin;
	*out__word_end = last_valid_char_id + 1;
	*out__automaton_id = last_valid_automaton_id;

	return EXIT_SUCCESS;
}



int lrParserNextToken(LRParser * parser, const char ** cursor, unsigned * out__token_id, char ** out__token_data)
{
	int ret;
	unsigned word_begin;
	unsigned word_end;
	unsigned automaton_id;

	ret = lrParserSplitNextWord(parser, *cursor, &word_begin, &word_end, &automaton_id);

	if (ret != EXIT_SUCCESS){
		return ret;
	}

	unsigned word_size = word_end - word_begin;

	if (word_size == 0){
		/* End of file */
		*cursor += word_end;
		*out__token_id = parser->grammar->nbSymbols - 1;
		*out__token_data = strdup("<eof>");
		return EXIT_SUCCESS;
	}

	char * token_data = malloc(word_size + 1);

	if (token_data == NULL){
		return EXIT_FAILURE;
	}

	memcpy(token_data, *cursor + word_begin, word_size);
	token_data[word_size] = '\0';

	*cursor += word_end;
	*out__token_id = (automaton_id + parser->grammar->nbNonTerminal);
	*out__token_data = token_data;

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

	stateStack[stateStackSize++] = 0;


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

		int action = (int) lrTransitionMatrixGetNextStateId(transitions, currentSate, currentTokenId);

		if (action >= 0){
			// shift
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

		// reduce
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
		fprintf(stderr, "could not load file\n");
		return NULL;
	}

	LRParseTree * output = lrParserParseStr(parser, buffer);

	free(buffer);

	return output;
}
