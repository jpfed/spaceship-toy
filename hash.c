#include <stdlib.h>
#include "hash.h"

struct Hash* hashInit(int size) {
	struct Hash* result = malloc(sizeof(struct Hash));
	result->numBuckets = size;
	result->buckets = malloc(size*sizeof(struct HashBucket));
	int i;
	for (i=0;i<size;i++)
		result->buckets[i].head=NULL;
	return result;	
}

void bucketAdd(struct HashBucket *dest, void *toAdd) {
	if (dest==NULL) return;
	struct HashElement *cursor = dest->head;
	struct HashElement *prev = NULL;
	while(cursor!=NULL) {
		if (cursor->obj==toAdd)	return;
		prev = cursor;
		cursor = cursor->next;
	}
	struct HashElement *ele = malloc(sizeof(struct HashElement));
	ele->obj = toAdd;
	ele->next = NULL;	
	if (prev!=NULL) prev->next = ele;
	else dest->head = ele;
}

int bucketRemove(struct HashBucket *source, void *target) {
	struct HashElement *cursor = source->head;
	struct HashElement *prev = NULL;
	while (cursor!=NULL) {
		if (cursor->obj == target) {
			if (prev==NULL) source->head = cursor->next;
			else {prev->next = cursor->next;}
			free(cursor);
			return 1;
		}
		prev = cursor;
		cursor = cursor->next;
	}
	return 0;
}

void hashAdd(struct Hash* hash, void *target) {
	int index = (((int)(target))>>2)%(hash->numBuckets);
	bucketAdd(hash->buckets+index,target);
}

int hashRemove(struct Hash* hash, void *target) {
	int index = (((int)(target))>>2)%(hash->numBuckets);
	return bucketRemove(hash->buckets+index,target);
}

void freeHash(struct Hash* hash) {
	if (hash==NULL) return;
	int i;
	struct HashElement *cursor = NULL;
	struct HashElement *next = NULL;
	for (i=0;i<hash->numBuckets;i++) {
		cursor = hash->buckets[i].head;
		while (cursor!=NULL) {
			next = cursor->next;
			free(cursor);
			cursor = next;	
		}
	}
	free(hash->buckets);
	free(hash);
}
