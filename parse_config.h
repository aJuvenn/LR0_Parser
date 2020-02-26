/*
 * parse_config.h
 *
 *  Created on: 26 févr. 2020
 *      Author: ajuvenn
 */

#ifndef PARSE_CONFIG_H_
#define PARSE_CONFIG_H_

#define MAX_NB_LINES 4096


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


GrammarDescription * parseConfigFile(const char * const path);

void printConfigFile(GrammarDescription * descr);

int split(char * buffer, const char * const sep, unsigned * out_nbParts, char *** out_parts);


#endif /* PARSE_CONFIG_H_ */
