
#ifndef LR_PARSER_UTILS_H_
#define LR_PARSER_UTILS_H_


typedef struct LRPointerArray
{
	unsigned nbElements;
	unsigned allocatedNbElements;
	void ** elements;
} LRPointerArray;

LRPointerArray * lrPointerArrayNew(const unsigned allocatedNbElements);
int lrPointerArrayPushBack(LRPointerArray * const array, void * const element);
void lrPointerArrayFree(LRPointerArray * array);
void * lrPointerArrayPopFront(LRPointerArray * array);


typedef struct FLPointerCouple
{
	void * first;
	void * second;
} LRPointerCouple;


LRPointerCouple * lrPointerCoupleNew(void * first, void * second);



#endif /* LR_PARSER_UTILS_H_ */
