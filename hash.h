#pragma once

struct Hash {
	struct HashBucket *buckets;
	int numBuckets;	
};

struct HashBucket {
	struct HashElement *head;	
};

struct HashElement {
	void *obj;
	struct HashElement *next;
};

void bucketAdd(struct HashBucket *dest, void *toAdd);
int bucketRemove(struct HashBucket *source, void *target);
struct Hash* hashInit(int size);
void hashAdd(struct Hash* hash, void *target);
int hashRemove(struct Hash* hash, void *target);
void freeHash(struct Hash* hash);
