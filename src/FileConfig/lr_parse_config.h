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
	unsigned skippedToken;

	unsigned nbGrammarRules;
	char ** ruleLeftMembers;
	unsigned * ruleRightMemberSizes;
	char *** ruleRightMembers;

} lrFileConfig;


lrFileConfig * lrFileConfigParse(const char * const path);

void lrFileConfigPrint(lrFileConfig * descr);

void lrFileConfigFree(lrFileConfig * conf);

#endif /* PARSE_CONFIG_H_ */
