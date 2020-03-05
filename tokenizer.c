/*
 * tokenizer.c
 *
 *  Created on: 2 mars 2020
 *      Author: ajuvenn
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cdfa/cdfa.h"
#include "parse_config.h"


int split_next_word(char * str, unsigned nb_automatons, cdfa__automaton * automatons[], unsigned * out__word_begin, unsigned * out__word_end, unsigned * out__automaton_id)
{
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



enum token
{
	TOKEN_INVALID = 0,
	TOKEN_FUN,
	TOKEN_LET,
	TOKEN_DEF,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_LEFT_BRACKET,
	TOKEN_RIGHT_BRACKET,
	TOKEN_LEFT_SQUARED_BRACKET,
	TOKEN_RIGHT_SQUARED_BRACKET,
	TOKEN_INT,
	TOKEN_VARIABLE,

	TOKEN_END_OF_FILE
};





enum token next_token(char ** cursor, char ** out__token_data)
{
	static const char * reg_exprs[] =
	{
			"fun", "let", "def", "if", "else", "\\(", "\\)", "\\[", "\\]",
			"\\-?([0-9]+) | \\-?0x([a-f0-9]+)",  // ints
			"[a-zA-Z_]([a-zA-Z0-9_]*)", // variable names
	};

	static const unsigned nb_automatons = sizeof(reg_exprs) / sizeof(char *);
	static cdfa__automaton * automatons[sizeof(reg_exprs) / sizeof(char *)];
	static int is_initialized = 0;

	if (!is_initialized){
		for (unsigned i = 0 ; i < nb_automatons ; i++){
			automatons[i] = cdfa__expression_recognizing_automaton(reg_exprs[i]);
		}
		is_initialized = 1;
	}

	unsigned word_begin;
	unsigned word_end;
	unsigned automaton_id;
	int ret;

	ret = split_next_word(*cursor, nb_automatons, automatons, &word_begin, &word_end, &automaton_id);

	if (ret != EXIT_SUCCESS){
		return TOKEN_INVALID;
	}

	unsigned word_size = word_end - word_begin;

	if (word_size == 0){
		*cursor += word_end;
		*out__token_data = NULL;
		return TOKEN_END_OF_FILE;
	}

	char * token_data = malloc(word_size + 1);

	if (token_data == NULL){
		return TOKEN_INVALID;
	}

	memcpy(token_data, *cursor + word_begin, word_size);
	token_data[word_size] = '\0';

	*cursor += word_end;
	*out__token_data = token_data;

	return (enum token) (automaton_id + 1);
}




int main(int argc, char **argv)
{
	char * str = "def f fun (x y z) (y z)";
	char ** cursor = &str;

	enum token tk;

	while (1){

		char * token_data;
		tk = next_token(cursor, &token_data);

		if (tk == TOKEN_INVALID){
			printf("TOKEN_INVALID\n");
			break;
		} else if (tk == TOKEN_END_OF_FILE){
			printf("TOKEN_END_OF_FILE\n");
			break;
		}

		printf("%s (%u)\n", token_data, tk);
		free(token_data);
	}

	return 0;
}



