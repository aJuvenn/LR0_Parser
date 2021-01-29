#include "../lr_include.h"




LRPointerCouple * lrPointerCoupleNew(void * first, void * second)
{
	LRPointerCouple * output = malloc(sizeof(LRPointerCouple));

	if (output == NULL){
		return NULL;
	}
	output->first = first;
	output->second = second;
	return output;
}

