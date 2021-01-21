/*
 * lr_utils.c
 *
 *  Created on: 11 mars 2020
 *      Author: ajuvenn
 */


#include "../lr_include.h"



char * lrLoadFile(const char * const path)
{
	FILE * f;
	unsigned fileLength;
	char * output;

	f = fopen(path, "r");

	if (f == NULL){
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	fileLength = ftell(f);
	output = malloc(fileLength+1);
	rewind(f);
	fread(output, fileLength, 1, f);
	fclose(f);

	output[fileLength] = '\0';

	return output;
}

