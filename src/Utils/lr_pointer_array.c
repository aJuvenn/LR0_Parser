#include "../lr_include.h"


LRPointerArray * lrPointerArrayNew(const unsigned allocatedNbElements)
{
	LRPointerArray * output = malloc(sizeof(LRPointerArray));

	if (output == NULL){
		return NULL;
	}

	output->nbElements = 0;
	output->allocatedNbElements = allocatedNbElements;
	output->elements = malloc(allocatedNbElements * sizeof(void *));

	if (output->elements == NULL){
		free(output);
		return NULL;
	}

	return output;
}

int lrPointerArrayPushBack(LRPointerArray * const array, void * const element)
{
	if (array->nbElements == array->allocatedNbElements){
		array->allocatedNbElements = 2 * array->allocatedNbElements + 1;
		void ** newElements = realloc(array->elements, array->allocatedNbElements * sizeof(void *));
		if (newElements == NULL){
			return EXIT_FAILURE;
		}
		array->elements = newElements;
	}
	array->elements[array->nbElements++] = element;
	return EXIT_SUCCESS;
}

void lrPointerArrayFree(LRPointerArray * array)
{
	free(array->elements);
	free(array);
}


void * lrPointerArrayPopFront(LRPointerArray * array)
{
	if (array->nbElements == 0){
		return NULL;
	}

	void * firstElement = array->elements[0];
	unsigned newNbElements = array->nbElements - 1;
	void ** newElements = malloc(newNbElements * sizeof(void *));

	if (newElements == NULL){
		return NULL;
	}

	memcpy(newElements, array->elements + 1, newNbElements * sizeof(void *));
	free(array->elements);

	array->nbElements = newNbElements;
	array->allocatedNbElements = newNbElements;
	array->elements = newElements;

	return firstElement;
}
